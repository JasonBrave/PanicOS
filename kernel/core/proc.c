/*
 * Process management
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

#include <arch/x86/lapic.h>
#include <arch/x86/mmu.h>
#include <common/spinlock.h>
#include <common/x86.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

struct ProcTable ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void pinit(void) {
	initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int cpuid() {
	return mycpu() - cpus;
}

void proc_free(struct proc *p) {
	kfree(p->kstack);
	p->kstack = 0;
	freevm(p->pgdir);
	kfree(p->cwd.pathbuf);
	p->pid = 0;
	p->parent = 0;
	p->name[0] = 0;
	p->killed = 0;
	p->state = UNUSED;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *mycpu(void) {
	unsigned int apicid, i;

	if (readeflags() & FL_IF) {
		panic("mycpu called with interrupts enabled\n");
	}

	apicid = lapicid();
	// APIC IDs are not guaranteed to be contiguous. Maybe we should have
	// a reverse map, or reserve a register to store &cpus[i].
	for (i = 0; i < ncpu; ++i) {
		if (cpus[i].apicid == apicid) {
			return &cpus[i];
		}
	}
	panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *myproc(void) {
	struct cpu *c;
	struct proc *p;
	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();
	return p;
}

// PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *allocproc(void) {
	struct proc *p;
	char *sp;

	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state == UNUSED) {
			goto found;
		}
	}

	release(&ptable.lock);
	return 0;

found:
	p->state = EMBRYO;
	p->pid = nextpid++;

	release(&ptable.lock);

	// Allocate kernel stack.
	if ((p->kstack = kalloc()) == 0) {
		p->state = UNUSED;
		return 0;
	}

	sp = p->kstack + KSTACKSIZE;
	// Leave room for trap frame.
	sp -= sizeof *p->tf;
	p->tf = (struct trapframe *)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(unsigned int *)sp = (unsigned int)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context *)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (unsigned int)forkret;

	// empty file table
	memset(p->files, 0, sizeof(p->files));
	p->dyn_base = PROC_DYNAMIC_BOTTOM;
	p->pty = 0;
	// empty the message queue
	p->msgqueue.begin = 0;
	p->msgqueue.end = 0;
	initlock(&p->msgqueue.lock, "msgqueue");

	return p;
}

// PAGEBREAK: 32
// Set up first user process.
void userinit(void) {
	struct proc *p;
	extern char _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc();

	initproc = p;
	if ((p->pgdir = setupkvm()) == 0) {
		panic("userinit: out of memory?");
	}
	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
	p->sz = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
	p->tf->es = p->tf->ds;
	p->tf->ss = p->tf->ds;
	p->tf->eflags = FL_IF;
	p->tf->esp = PGSIZE;
	p->tf->eip = 0; // beginning of initcode.S

	safestrcpy(p->name, "initcode", sizeof(p->name));

	p->cwd.parts = 0; // root directory
	p->cwd.pathbuf = kalloc();

	// this assignment to p->state lets other cores
	// run this process. the acquire forces the above
	// writes to be visible, and the lock is also needed
	// because the assignment might not be atomic.
	acquire(&ptable.lock);

	p->state = RUNNABLE;

	release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n) {
	struct proc *curproc = myproc();

	if (n > 0) {
		if (allocuvm(
				curproc->pgdir,
				PROC_HEAP_BOTTOM + curproc->heap_size,
				PROC_HEAP_BOTTOM + curproc->heap_size + n,
				PTE_W | PTE_U
			) == 0) {
			return -1;
		}
	} else if (n < 0) {
		if (deallocuvm(
				curproc->pgdir,
				PROC_HEAP_BOTTOM + curproc->heap_size,
				PROC_HEAP_BOTTOM + curproc->heap_size + n
			) == 0) {
			return -1;
		}
	}
	curproc->heap_size += n;
	switchuvm(curproc);
	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int fork(void) {
	int pid;
	struct proc *np;
	struct proc *curproc = myproc();

	// Allocate process.
	if ((np = allocproc()) == 0) {
		return -1;
	}
	// create new page directory
	if ((np->pgdir = setupkvm()) == 0) {
		return -1;
	}
	// Copy process executable image
	if (copyuvm(np->pgdir, curproc->pgdir, 0, curproc->sz) == 0) {
		freevm(np->pgdir);
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	// copy dynamic libraries
	if (copyuvm(np->pgdir, curproc->pgdir, PROC_DYNAMIC_BOTTOM, curproc->dyn_base) == 0) {
		freevm(np->pgdir);
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	// copy process stack
	if (copyuvm(
			np->pgdir, curproc->pgdir, PROC_STACK_BOTTOM - curproc->stack_size, PROC_STACK_BOTTOM
		) == 0) {
		freevm(np->pgdir);
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	// copy process heap
	if (copyuvm(
			np->pgdir, curproc->pgdir, PROC_HEAP_BOTTOM, PROC_HEAP_BOTTOM + curproc->heap_size
		) == 0) {
		freevm(np->pgdir);
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}

	np->sz = curproc->sz;
	np->stack_size = curproc->stack_size;
	np->heap_size = curproc->heap_size;
	np->dyn_base = curproc->dyn_base;
	np->pty = curproc->pty;
	np->parent = curproc;
	*np->tf = *curproc->tf;

	// Clear %eax so that fork returns 0 in the child.
	np->tf->eax = 0;

	safestrcpy(np->name, curproc->name, sizeof(curproc->name));

	// copy working directory
	np->cwd.parts = curproc->cwd.parts;
	np->cwd.pathbuf = kalloc();
	memmove(np->cwd.pathbuf, curproc->cwd.pathbuf, np->cwd.parts * 128);

	pid = np->pid;

	acquire(&ptable.lock);

	np->state = RUNNABLE;

	release(&ptable.lock);

	return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void exit(int status) {
	struct proc *curproc = myproc();
	struct proc *p;

	if (curproc == initproc) {
		panic("init exiting");
	}

	curproc->exit_status = status;
	// Close all open files.
	for (int i = 0; i < PROC_FILE_MAX; i++) {
		if (curproc->files[i].used) {
			if (curproc->files[i].dir) {
				vfs_dir_close(&curproc->files[i]);
			} else {
				vfs_fd_close(&curproc->files[i]);
			}
		}
	}

	acquire(&ptable.lock);

	// Parent might be sleeping in wait().
	wakeup1(curproc->parent);

	// Pass abandoned children to init.
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->parent == curproc) {
			p->parent = initproc;
			if (p->state == ZOMBIE) {
				wakeup1(initproc);
			}
		}
	}

	// Jump into the scheduler, never to return.
	curproc->state = ZOMBIE;
	sched();
	panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(void) {
	struct proc *p;
	int havekids, pid;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (;;) {
		// Scan through table looking for exited children.
		havekids = 0;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->parent != curproc) {
				continue;
			}
			havekids = 1;
			if (p->state == ZOMBIE) {
				// Found one.
				pid = p->pid;
				proc_free(p);
				// free message queue
				acquire(&p->msgqueue.lock);
				for (int msg = p->msgqueue.end; msg != p->msgqueue.begin; msg++) {
					if (msg == MESSAGE_MAX) {
						msg = 0;
					}
					struct Message *thismsg = &p->msgqueue.queue[msg];
					pgfree(thismsg->addr, PGROUNDUP(thismsg->size) / 4096);
				}
				release(&p->msgqueue.lock);
				release(&ptable.lock);
				return pid;
			}
		}

		// No point waiting if we don't have any children.
		if (!havekids || curproc->killed) {
			release(&ptable.lock);
			return -1;
		}

		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
		sleep(curproc, &ptable.lock); // DOC: wait-sleep
	}
}

// PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void) {
	struct proc *p;
	struct cpu *c = mycpu();
	c->proc = 0;

	for (;;) {
		// Enable interrupts on this processor.
		sti();

		// Loop over process table looking for process to run.
		acquire(&ptable.lock);
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->state != RUNNABLE) {
				continue;
			}

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;

			swtch(&(c->scheduler), p->context);
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			c->proc = 0;
		}
		release(&ptable.lock);
	}
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
	int intena;
	struct proc *p = myproc();

	if (!holding(&ptable.lock)) {
		panic("sched ptable.lock");
	}
	if (mycpu()->ncli != 1) {
		panic("sched locks");
	}
	if (p->state == RUNNING) {
		panic("sched running");
	}
	if (readeflags() & FL_IF) {
		panic("sched interruptible");
	}
	intena = mycpu()->intena;
	swtch(&p->context, mycpu()->scheduler);
	mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
	acquire(&ptable.lock); // DOC: yieldlock
	myproc()->state = RUNNABLE;
	sched();
	release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void) {
	// Still holding ptable.lock from scheduler.
	release(&ptable.lock);
	// Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk) {
	struct proc *p = myproc();

	if (p == 0) {
		panic("sleep");
	}

	if (lk == 0) {
		panic("sleep without lk");
	}

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	if (lk != &ptable.lock) { // DOC: sleeplock0
		acquire(&ptable.lock); // DOC: sleeplock1
		release(lk);
	}
	// Go to sleep.
	p->chan = chan;
	p->state = SLEEPING;

	sched();

	// Tidy up.
	p->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) { // DOC: sleeplock2
		release(&ptable.lock);
		acquire(lk);
	}
}

// PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void wakeup1(void *chan) {
	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state == SLEEPING && p->chan == chan) {
			p->state = RUNNABLE;
		}
	}
}

// Wake up all processes sleeping on chan.
void wakeup(void *chan) {
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int kill(int pid) {
	struct proc *p;

	acquire(&ptable.lock);
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->pid == pid) {
			p->killed = 1;
			p->state = ZOMBIE;
			release(&ptable.lock);
			return 0;
		}
	}
	release(&ptable.lock);
	return -1;
}

// PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void) {
	static char *states[] = {
		[UNUSED] = "unused",
		[EMBRYO] = "embryo",
		[SLEEPING] = "sleep ",
		[RUNNABLE] = "runble",
		[RUNNING] = "run   ",
		[ZOMBIE] = "zombie"
	};
	int i;
	struct proc *p;
	char *state;
	unsigned int pc[10];

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state == UNUSED) {
			continue;
		}
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state]) {
			state = states[p->state];
		} else {
			state = "???";
		}
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			getcallerpcs((unsigned int *)p->context->ebp + 2, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++) {
				cprintf(" %p", pc[i]);
			}
		}
		cprintf("\n");
	}
}

struct proc *proc_search_pid(int pid) {
	acquire(&ptable.lock);
	for (int i = 0; i < NPROC; i++) {
		if (ptable.proc[i].state != UNUSED && ptable.proc[i].pid == pid) {
			release(&ptable.lock);
			return &ptable.proc[i];
		}
	}
	release(&ptable.lock);
	return 0;
}
