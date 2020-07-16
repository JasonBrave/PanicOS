#ifndef _DEFS_H
#define _DEFS_H

#include <common/types.h>
#include <core/mmu.h>

struct context;
struct proc;
struct spinlock;
struct FileDesc;

// console.c
void consoleinit(void);
void cprintf(const char*, ...);
void consoleintr(int (*)(void));
void panic(const char*) __attribute__((noreturn));
int consoleread(char* dst, int n);
int consolewrite(char* buf, int n);

// exec.c
int exec(char*, char**);

// elf.c
int proc_elf_load(pde_t* pgdir, unsigned int base, const char* name,
				  unsigned int* entry, unsigned int* dynamic, unsigned int* interp);

// dynamic.c
int proc_load_dynamic(struct proc* proc, const char* name, unsigned int* dynamic,
					  unsigned int* entry);

// ioapic.c
void ioapicenable(int irq, int cpu);
extern unsigned char ioapicid;
void ioapicinit(void);

// kalloc.c
void* kalloc(void);
void kfree(void*);
void kinit1(void*, void*);
void kinit2(void*, void*);

// kbd.c
void kbdintr(void);

// lapic.c
int lapicid(void);
extern volatile uint32_t* lapic;
void lapiceoi(void);
void lapicinit(void);
void lapicstartap(unsigned char, unsigned int);
void microdelay(int);

// mp.c
extern int ismp;
void mpinit(void);

// picirq.c
void picenable(int);
void picinit(void);

// proc.c
int cpuid(void);
void exit(void);
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

// swtch.S
void swtch(struct context**, struct context*);

// string.c
int memcmp(const void*, const void*, unsigned int);
void* memmove(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
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

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);

// vm.c
void seginit(void);
void kvmalloc(void);
pde_t* setupkvm(void);
char* uva2ka(pde_t*, char*);
int allocuvm(pde_t*, unsigned int, unsigned int, int perm);
int deallocuvm(pde_t*, unsigned int, unsigned int);
void freevm(pde_t*);
void inituvm(pde_t*, char*, unsigned int);
int loaduvm(pde_t*, char*, struct FileDesc* fd, unsigned int, unsigned int);
pde_t* copyuvm(pde_t* newpgdir, pde_t* oldpgdir, unsigned int begin, unsigned int end);
void switchuvm(struct proc*);
void switchkvm(void);
int copyout(pde_t*, unsigned int, void*, unsigned int);
void clearpteu(pde_t* pgdir, char* uva);
int mappages(pde_t* pgdir, void* va, unsigned int size, unsigned int pa, int perm);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#endif
