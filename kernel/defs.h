#ifndef _DEFS_H
#define _DEFS_H

#include <common/sleeplock.h>
#include <common/types.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <memlayout.h>

struct context;
struct FileDesc;

extern struct ProcTable {
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

// console.c
void vgacon_init(void);
void fbcon_init(phyaddr_t fb_addr, unsigned int width, unsigned int height);
void cprintf(const char*, ...);
void consoleintr(int (*)(void));
void panic(const char*) __attribute__((noreturn));
int consoleread(char* dst, int n);
int consolewrite(char* buf, int n);

// module.c
void module_init(void);
int module_load(const char* name);
void module_print(void);

// exec.c
int exec(char*, char**);

// elf.c
int proc_elf_load(pdpte_t* pgdir, unsigned int base, const char* name, unsigned int* entry,
				  unsigned int* dynamic, unsigned int* interp);

// dynamic.c
int proc_load_dynamic(struct proc* proc, const char* name, unsigned int* dynamic,
					  unsigned int* entry);

// kalloc.c
void* pgalloc(unsigned int num_pages);
void pgfree(void* ptr, unsigned int num_pages);
static inline void* kalloc(void) {
	return pgalloc(1);
}
static inline void kfree(void* ptr) {
	return pgfree(ptr, 1);
}
void kinit1(void*, void*);
void kinit2(void*, void*);
void print_memory_usage(void);

// mp.c
extern int ismp;
void mpinit(void);

// proc.c
int cpuid(void);
void proc_free(struct proc* p);
void exit(int status);
int fork(void);
int growproc(int);
int kill(int);
struct cpu* mycpu(void);
struct proc* myproc();
void pinit(void);
void procdump(void);
void scheduler(void) __attribute__((noreturn));
void sched(void);
void setproc(struct proc*);
void sleep(void*, struct spinlock*);
void userinit(void);
int wait(void);
void wakeup(void*);
void yield(void);
struct proc* proc_search_pid(int pid);

// swtch.S
void swtch(struct context**, struct context*);

// string.c
void* memset(void* dst, int c, unsigned int n);
volatile void *memset_volatile(volatile void *dst, int c, unsigned int n);
int memcmp(const void *v1, const void *v2, unsigned int n);
int memcmp_volatile(const volatile void *v1, const volatile void *v2, unsigned int n);
void* memmove(void* dst, const void* src, unsigned int n);
volatile void* memmove_volatile(volatile void* dst, const volatile void* src, unsigned int n);
char* safestrcpy(char*, const char*, int);
int strlen(const char*);
int strncmp(const char*, const char*, unsigned int);
char* strncpy(char*, const char*, int);

// syscall.c
int argint(int, int*);
int argptr(int, char**, int);
int argstr(int, char**);
int fetchint(unsigned int, int*);
int fetchstr(unsigned int, char**);
void syscall(void);

// timer.c
void timerinit(void);

// trap.c
void idtinit(void);
extern unsigned int ticks;
void tvinit(void);
extern struct spinlock tickslock;

// vm.c
void seginit(void);
void kvmalloc(void);
pdpte_t* setupkvm(void);
char* uva2ka(pdpte_t*, char*);
int allocuvm(pdpte_t*, unsigned int, unsigned int, int perm);
int deallocuvm(pdpte_t*, unsigned int, unsigned int);
void freevm(pdpte_t*);
void inituvm(pdpte_t*, char*, unsigned int);
int loaduvm(pdpte_t*, char*, struct FileDesc* fd, unsigned int, unsigned int);
pdpte_t* copyuvm(pdpte_t* newpgdir, pdpte_t* oldpgdir, unsigned int begin, unsigned int end);
void switchuvm(struct proc*);
void switchkvm(void);
int copyout(pdpte_t*, unsigned int, void*, unsigned int);
void clearpteu(pdpte_t* pgdir, char* uva);
int mappages(pdpte_t* pgdir, void* va, unsigned int size, unsigned int pa, int perm);
pdpte_t* copypgdir(pdpte_t* newpgdir, pdpte_t* oldpgdir, unsigned int begin, unsigned int end);
void* map_mmio_region(phyaddr_t phyaddr, size_t size);
void* map_ram_region(phyaddr_t phyaddr, size_t size);
void* map_rom_region(phyaddr_t phyaddr, size_t size);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

extern struct BootGraphicsMode {
	enum {
		BOOT_GRAPHICS_MODE_VGA_TEXT,
		BOOT_GRAPHICS_MODE_FRAMEBUFFER,
		BOOT_GRAPHICS_MODE_HEADLESS,
	} mode;
	unsigned int width, height;
	phyaddr_t fb_addr;
} boot_graphics_mode;

#endif
