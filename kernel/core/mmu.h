/*
 * x86 MMU header
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

#ifndef _MMU_H
#define _MMU_H

// Eflags register
#define FL_IF 0x00000200 // Interrupt Enable

// Control Register flags
#define CR0_PE 0x00000001 // Protection Enable
#define CR0_WP 0x00010000 // Write Protect
#define CR0_PG 0x80000000 // Paging

#define CR4_PSE 0x00000010 // Page size extension

// various segment selectors.
#define SEG_KCODE 1 // kernel code
#define SEG_KDATA 2 // kernel data+stack
#define SEG_UCODE 3 // user code
#define SEG_UDATA 4 // user data+stack
#define SEG_TSS 5 // this process's task state

// cpu->gdt[NSEGS] holds the above segments.
#define NSEGS 6

#ifndef __ASSEMBLER__
// Segment Descriptor
struct segdesc {
	unsigned int lim_15_0 : 16; // Low bits of segment limit
	unsigned int base_15_0 : 16; // Low bits of segment base address
	unsigned int base_23_16 : 8; // Middle bits of segment base address
	unsigned int type : 4; // Segment type (see STS_ constants)
	unsigned int s : 1; // 0 = system, 1 = application
	unsigned int dpl : 2; // Descriptor Privilege Level
	unsigned int p : 1; // Present
	unsigned int lim_19_16 : 4; // High bits of segment limit
	unsigned int avl : 1; // Unused (available for software use)
	unsigned int rsv1 : 1; // Reserved
	unsigned int db : 1; // 0 = 16-bit segment, 1 = 32-bit segment
	unsigned int g : 1; // Granularity: limit scaled by 4K when set
	unsigned int base_31_24 : 8; // High bits of segment base address
};

// Normal segment
#define SEG(type, base, lim, dpl)                                                      \
	(struct segdesc) {                                                                 \
		((lim) >> 12) & 0xffff, (unsigned int)(base)&0xffff,                           \
			((unsigned int)(base) >> 16) & 0xff, type, 1, dpl, 1,                      \
			(unsigned int)(lim) >> 28, 0, 0, 1, 1, (unsigned int)(base) >> 24          \
	}
#define SEG16(type, base, lim, dpl)                                                    \
	(struct segdesc) {                                                                 \
		(lim) & 0xffff, (unsigned int)(base)&0xffff,                                   \
			((unsigned int)(base) >> 16) & 0xff, type, 1, dpl, 1,                      \
			(unsigned int)(lim) >> 16, 0, 0, 1, 0, (unsigned int)(base) >> 24          \
	}
#endif

#define DPL_USER 0x3 // User DPL

// Application segment type bits
#define STA_X 0x8 // Executable segment
#define STA_W 0x2 // Writeable (non-executable segments)
#define STA_R 0x2 // Readable (executable segments)

// System segment type bits
#define STS_T32A 0x9 // Available 32-bit TSS
#define STS_IG32 0xE // 32-bit Interrupt Gate
#define STS_TG32 0xF // 32-bit Trap Gate

// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

// page directory index
#define PDX(va) (((unsigned int)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va) (((unsigned int)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((unsigned int)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES 1024 // # directory entries per page directory
#define NPTENTRIES 1024 // # PTEs per page table
#define PGSIZE 4096 // bytes mapped by a page

#define PTXSHIFT 12 // offset of PTX in a linear address
#define PDXSHIFT 22 // offset of PDX in a linear address

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

// Page table/directory entry flags.
#define PTE_P 0x001 // Present
#define PTE_W 0x002 // Writeable
#define PTE_U 0x004 // User
#define PTE_PS 0x080 // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte) ((unsigned int)(pte) & ~0xFFF)
#define PTE_FLAGS(pte) ((unsigned int)(pte)&0xFFF)

#ifndef __ASSEMBLER__
typedef unsigned int pde_t;
typedef unsigned int pte_t;

// Task state segment format
struct taskstate {
	unsigned int link; // Old ts selector
	unsigned int esp0; // Stack pointers and segment selectors
	unsigned short ss0; //   after an increase in privilege level
	unsigned short padding1;
	unsigned int* esp1;
	unsigned short ss1;
	unsigned short padding2;
	unsigned int* esp2;
	unsigned short ss2;
	unsigned short padding3;
	void* cr3; // Page directory base
	unsigned int* eip; // Saved state from last task switch
	unsigned int eflags;
	unsigned int eax; // More saved state (registers)
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int* esp;
	unsigned int* ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned short es; // Even more saved state (segment selectors)
	unsigned short padding4;
	unsigned short cs;
	unsigned short padding5;
	unsigned short ss;
	unsigned short padding6;
	unsigned short ds;
	unsigned short padding7;
	unsigned short fs;
	unsigned short padding8;
	unsigned short gs;
	unsigned short padding9;
	unsigned short ldt;
	unsigned short padding10;
	unsigned short t; // Trap on task switch
	unsigned short iomb; // I/O map base address
};

// Gate descriptors for interrupts and traps
struct gatedesc {
	unsigned int off_15_0 : 16; // low 16 bits of offset in segment
	unsigned int cs : 16; // code segment selector
	unsigned int args : 5; // # args, 0 for interrupt/trap gates
	unsigned int rsv1 : 3; // reserved(should be zero I guess)
	unsigned int type : 4; // type(STS_{IG32,TG32})
	unsigned int s : 1; // must be 0 (system)
	unsigned int dpl : 2; // descriptor(meaning new) privilege level
	unsigned int p : 1; // Present
	unsigned int off_31_16 : 16; // high bits of offset in segment
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                                             \
	{                                                                                  \
		(gate).off_15_0 = (unsigned int)(off)&0xffff;                                  \
		(gate).cs = (sel);                                                             \
		(gate).args = 0;                                                               \
		(gate).rsv1 = 0;                                                               \
		(gate).type = (istrap) ? STS_TG32 : STS_IG32;                                  \
		(gate).s = 0;                                                                  \
		(gate).dpl = (d);                                                              \
		(gate).p = 1;                                                                  \
		(gate).off_31_16 = (unsigned int)(off) >> 16;                                  \
	}

#endif

#endif
