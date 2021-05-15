/*
 * Virtual memory management
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/spinlock.h>
#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <filesystem/vfs/vfs.h>
#include <memlayout.h>
#include <param.h>

extern char data[]; // defined by kernel.ld
pdpte_t* kpgdir; // for use in scheduler()

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void seginit(void) {
	struct cpu* c;

	// Map "logical" addresses to virtual addresses using identity map.
	// Cannot share a CODE descriptor for both kernel and user
	// because it would have to have DPL_USR, but the CPU forbids
	// an interrupt from CPL=0 to DPL=3.
	c = &cpus[cpuid()];
	c->gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
	c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
	c->gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
	c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
	lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t* walkpgdir(pdpte_t* pdpte_table, const void* va, int alloc, int perm) {
	pte_t* pte_tab;

	pdpte_t* pdpte = &pdpte_table[PDPTX(va)];
	if (*pdpte & PTE_P) {
		pde_t* pde_tab = (pde_t*)P2V(PTE_ADDR(*pdpte));
		pde_t* pde = &pde_tab[PDX(va)];
		if (*pde & PTE_P) {
			pte_tab = (pte_t*)P2V(PTE_ADDR(*pde));
		} else {
			if (!alloc || (pte_tab = (pte_t*)kalloc()) == 0) {
				return 0;
			}
			memset(pte_tab, 0, PGSIZE);
			*pde = V2P(pte_tab) | PTE_P | perm;
		}
	} else {
		pde_t* pde_tab;
		if (!alloc || (pde_tab = (pde_t*)kalloc()) == 0) {
			return 0;
		}
		memset(pde_tab, 0, PGSIZE);
		*pdpte = V2P(pde_tab) | PTE_P;
		pde_t* pde = &pde_tab[PDX(va)];
		if (!alloc || (pte_tab = (pte_t*)kalloc()) == 0) {
			return 0;
		}
		memset(pte_tab, 0, PGSIZE);
		*pde = V2P(pte_tab) | PTE_P | perm;
	}
	return &pte_tab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
int mappages(pdpte_t* pgdir, void* va, unsigned int size, unsigned int pa, int perm) {
	char *a, *last;
	pte_t* pte;

	a = (char*)PGROUNDDOWN((unsigned int)va);
	last = (char*)PGROUNDDOWN(((unsigned int)va) + size - 1);
	for (;;) {
		if ((pte = walkpgdir(pgdir, a, 1, PTE_W | PTE_U)) == 0)
			return -1;
		if (*pte & PTE_P)
			panic("remap");
		*pte = pa | perm | PTE_P;
		if (a == last)
			break;
		a += PGSIZE;
		pa += PGSIZE;
	}
	return 0;
}

int unmappages(pdpte_t* pgdir, void* va, unsigned int size) {
	char *a, *last;
	pte_t* pte;

	a = (char*)PGROUNDDOWN((unsigned int)va);
	last = (char*)PGROUNDDOWN(((unsigned int)va) + size - 1);
	for (;;) {
		if ((pte = walkpgdir(pgdir, a, 1, PTE_W | PTE_U)) == 0)
			return -1;
		*pte = 0;
		if (a == last)
			break;
		a += PGSIZE;
	}
	return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap {
	void* virt;
	unsigned int phys_start;
	unsigned int phys_end;
	int perm;
} kmap[] = {
	{(void*)KERNBASE, 0, EXTMEM, PTE_W}, // I/O space
	{(void*)KERNLINK, V2P(KERNLINK), V2P(data), 0}, // kern text+rodata
	{(void*)data, V2P(data), PHYSTOP, PTE_W}, // kern data+memory
	{(void*)DEVSPACE, DEVSPACE, 0, PTE_W}, // more devices
};

extern void module_set_pgdir(pdpte_t* pgdir);

// Set up kernel part of a page table.
pdpte_t* setupkvm(void) {
	pdpte_t* pgdir;
	struct kmap* k;

	if ((pgdir = (pdpte_t*)kalloc()) == 0) {
		return 0;
	}
	memset(pgdir, 0, PGSIZE);
	if (P2V(PHYSTOP) > (void*)DEVSPACE) {
		panic("PHYSTOP too high");
	}
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++) {
		if (mappages(pgdir, k->virt, k->phys_end - k->phys_start, (unsigned int)k->phys_start,
					 k->perm) < 0) {
			freevm(pgdir);
			return 0;
		}
	}
	// copy kernel module map
	module_set_pgdir(pgdir);
	return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void kvmalloc(void) {
	kpgdir = setupkvm();
	switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void switchkvm(void) {
	lcr3(V2P(kpgdir)); // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void switchuvm(struct proc* p) {
	if (p == 0)
		panic("switchuvm: no process");
	if (p->kstack == 0)
		panic("switchuvm: no kstack");
	if (p->pgdir == 0)
		panic("switchuvm: no pgdir");

	pushcli();
	mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts, sizeof(mycpu()->ts) - 1, 0);
	mycpu()->gdt[SEG_TSS].s = 0;
	mycpu()->ts.ss0 = SEG_KDATA << 3;
	mycpu()->ts.esp0 = (unsigned int)p->kstack + KSTACKSIZE;
	// setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
	// forbids I/O instructions (e.g., inb and outb) from user space
	mycpu()->ts.iomb = (unsigned short)0xFFFF;
	ltr(SEG_TSS << 3);
	lcr3(V2P(p->pgdir)); // switch to process's address space
	popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void inituvm(pdpte_t* pgdir, char* init, unsigned int sz) {
	char* mem;

	if (sz >= PGSIZE)
		panic("inituvm: more than a page");
	mem = kalloc();
	memset(mem, 0, PGSIZE);
	mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
	memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int loaduvm(pdpte_t* pgdir, char* addr, struct FileDesc* fd, unsigned int offset, unsigned int sz) {
	unsigned int i, pa;
	int n;
	pte_t* pte;

	if ((unsigned int)addr % PGSIZE != 0)
		panic("loaduvm: addr must be page aligned");
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, addr + i, 0, PTE_W | PTE_U)) == 0)
			panic("loaduvm: address should exist");
		pa = PTE_ADDR(*pte);
		if (sz - i < PGSIZE)
			n = sz - i;
		else
			n = PGSIZE;
		if (vfs_fd_seek(fd, offset + i, SEEK_SET) < 0) {
			return -1;
		}
		if (vfs_fd_read(fd, P2V(pa), n) < 0) {
			return -1;
		}
	}
	return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int allocuvm(pdpte_t* pgdir, unsigned int oldsz, unsigned int newsz, int perm) {
	char* mem;
	unsigned int a;

	if (newsz < oldsz)
		return oldsz;

	a = PGROUNDUP(oldsz);
	for (; a < newsz; a += PGSIZE) {
		mem = kalloc();
		if (mem == 0) {
			cprintf("allocuvm out of memory\n");
			deallocuvm(pgdir, newsz, oldsz);
			return 0;
		}
		memset(mem, 0, PGSIZE);
		if (mappages(pgdir, (char*)a, PGSIZE, V2P(mem), perm) < 0) {
			cprintf("allocuvm out of memory (2)\n");
			deallocuvm(pgdir, newsz, oldsz);
			kfree(mem);
			return 0;
		}
	}
	return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int deallocuvm(pdpte_t* pgdir, unsigned int oldsz, unsigned int newsz) {
	pte_t* pte;
	unsigned int a, pa;

	if (newsz >= oldsz)
		return oldsz;

	a = PGROUNDUP(newsz);
	for (; a < oldsz; a += PGSIZE) {
		pte = walkpgdir(pgdir, (char*)a, 0, PTE_W | PTE_U);
		if (pte && (*pte & PTE_P)) {
			pa = PTE_ADDR(*pte);
			if (pa == 0)
				panic("kfree");
			char* v = P2V(pa);
			kfree(v);
			*pte = 0;
		}
	}
	return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void freevm(pdpte_t* pgdir) {
	unsigned int i;

	if (pgdir == 0)
		panic("freevm: no pgdir");
	deallocuvm(pgdir, KERNBASE, 0);
	for (i = 0; i < NPDENTRIES; i++) {
		if (pgdir[i] & PTE_P) {
			char* v = P2V(PTE_ADDR(pgdir[i]));
			kfree(v);
		}
	}
	kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void clearpteu(pdpte_t* pgdir, char* uva) {
	pte_t* pte;

	pte = walkpgdir(pgdir, uva, 0, PTE_W | PTE_U);
	if (pte == 0)
		panic("clearpteu");
	*pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
pdpte_t* copyuvm(pdpte_t* newpgdir, pdpte_t* oldpgdir, unsigned int begin, unsigned int end) {
	pte_t* pte;
	unsigned int pa, i, flags;
	char* mem;

	for (i = begin; i < end; i += PGSIZE) {
		if ((pte = walkpgdir(oldpgdir, (void*)i, 0, PTE_W | PTE_U)) == 0)
			panic("copyuvm: pte should exist");
		if (!(*pte & PTE_P))
			panic("copyuvm: page not present");
		pa = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);
		if ((mem = kalloc()) == 0)
			return 0;
		memmove(mem, (char*)P2V(pa), PGSIZE);
		if (mappages(newpgdir, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
			kfree(mem);
			return 0;
		}
	}
	return newpgdir;
}

// PAGEBREAK!
// Map user virtual address to kernel address.
char* uva2ka(pdpte_t* pgdir, char* uva) {
	pte_t* pte;

	pte = walkpgdir(pgdir, uva, 0, PTE_W | PTE_U);
	if ((*pte & PTE_P) == 0)
		return 0;
	if ((*pte & PTE_U) == 0)
		return 0;
	return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int copyout(pdpte_t* pgdir, unsigned int va, void* p, unsigned int len) {
	char *buf, *pa0;
	unsigned int n, va0;

	buf = (char*)p;
	while (len > 0) {
		va0 = (unsigned int)PGROUNDDOWN(va);
		pa0 = uva2ka(pgdir, (char*)va0);
		if (pa0 == 0)
			return -1;
		n = PGSIZE - (va - va0);
		if (n > len)
			n = len;
		memmove(pa0 + (va - va0), buf, n);
		len -= n;
		buf += n;
		va = va0 + PGSIZE;
	}
	return 0;
}

pdpte_t* copypgdir(pdpte_t* newpgdir, pdpte_t* oldpgdir, unsigned int begin, unsigned int end) {
	pte_t* pte;
	unsigned int pa, i, flags;

	for (i = begin; i < end; i += PGSIZE) {
		if ((pte = walkpgdir(oldpgdir, (void*)i, 0, PTE_W | PTE_U)) == 0)
			panic("copypgdir: pte should exist");
		if (!(*pte & PTE_P))
			panic("copypgdir: page not present");
		pa = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);
		if (mappages(newpgdir, (void*)i, PGSIZE, pa, flags) < 0) {
			return 0;
		}
	}
	return newpgdir;
}

// map physical memory to virtual memory
static void* map_region(phyaddr_t phyaddr, size_t size) {
	if (phyaddr < DEVSPACE)
		panic("map_region: phyaddr not within DEVSPACE");
	return (void*)phyaddr;
}

void* map_mmio_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}

void* map_ram_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}

void* map_rom_region(phyaddr_t phyaddr, size_t size) {
	return map_region(phyaddr, size);
}
