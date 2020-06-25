#ifndef _DEFS_H
#define _DEFS_H

#include <core/mmu.h>

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct rtcdate;
struct spinlock;
struct stat;
struct superblock;

// bio.c
void binit(void);
struct buf* bread(unsigned int, unsigned int);
void brelse(struct buf*);
void bwrite(struct buf*);

// console.c
void consoleinit(void);
void cprintf(const char*, ...);
void consoleintr(int (*)(void));
void panic(const char*) __attribute__((noreturn));

// exec.c
int exec(char*, char**);

// file.c
struct file* filealloc(void);
void fileclose(struct file*);
struct file* filedup(struct file*);
void fileinit(void);
int fileread(struct file*, char*, int n);
int filestat(struct file*, struct stat*);
int filewrite(struct file*, char*, int n);

// fs.c
void readsb(int dev, struct superblock* sb);
int dirlink(struct inode*, char*, unsigned int);
struct inode* dirlookup(struct inode*, char*, unsigned int*);
struct inode* ialloc(unsigned int, short);
struct inode* idup(struct inode*);
void iinit(int dev);
void ilock(struct inode*);
void iput(struct inode*);
void iunlock(struct inode*);
void iunlockput(struct inode*);
void iupdate(struct inode*);
int namecmp(const char*, const char*);
struct inode* namei(char*);
struct inode* nameiparent(char*, char*);
int readi(struct inode*, char*, unsigned int, unsigned int);
void stati(struct inode*, struct stat*);
int writei(struct inode*, char*, unsigned int, unsigned int);

// ide.c
void ideinit(void);
void ideintr(void);
void iderw(struct buf*);

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
void cmostime(struct rtcdate* r);
int lapicid(void);
extern volatile unsigned int* lapic;
void lapiceoi(void);
void lapicinit(void);
void lapicstartap(unsigned char, unsigned int);
void microdelay(int);

// log.c
void initlog(int dev);
void log_write(struct buf*);
void begin_op();
void end_op();

// mp.c
extern int ismp;
void mpinit(void);

// picirq.c
void picenable(int);
void picinit(void);

// pipe.c
int pipealloc(struct file**, struct file**);
void pipeclose(struct pipe*, int);
int piperead(struct pipe*, char*, int);
int pipewrite(struct pipe*, char*, int);

// PAGEBREAK: 16
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
int allocuvm(pde_t*, unsigned int, unsigned int);
int deallocuvm(pde_t*, unsigned int, unsigned int);
void freevm(pde_t*);
void inituvm(pde_t*, char*, unsigned int);
int loaduvm(pde_t*, char*, struct inode*, unsigned int, unsigned int);
pde_t* copyuvm(pde_t*, unsigned int);
void switchuvm(struct proc*);
void switchkvm(void);
int copyout(pde_t*, unsigned int, void*, unsigned int);
void clearpteu(pde_t* pgdir, char* uva);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#endif
