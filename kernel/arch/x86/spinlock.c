/*
 * Spinlock implementation
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
#include <arch/x86/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

void initlock(struct spinlock* lk, const char* name) {
	lk->name = name;
	lk->locked = 0;
	lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void acquire(struct spinlock* lk) {
	pushcli(); // disable interrupts to avoid deadlock.
	if (holding(lk))
		panic("acquire");

	// The xchg is atomic.
	while (xchg(&lk->locked, 1) != 0)
		;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();

	// Record info about lock acquisition for debugging.
	lk->cpu = mycpu();
	getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void release(struct spinlock* lk) {
	if (!holding(lk))
		panic("release");

	lk->pcs[0] = 0;
	lk->cpu = 0;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that all the stores in the critical
	// section are visible to other cores before the lock is released.
	// Both the C compiler and the hardware may re-order loads and
	// stores; __sync_synchronize() tells them both not to.
	__sync_synchronize();

	// Release the lock, equivalent to lk->locked = 0.
	// This code can't use a C assignment, since it might
	// not be atomic. A real OS would use C atomics here.
	__asm__ volatile("movl $0, %0" : "+m"(lk->locked) :);

	popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void getcallerpcs(void* v, unsigned int pcs[]) {
	unsigned int* ebp;
	int i;

	ebp = (unsigned int*)v - 2;
	for (i = 0; i < 10; i++) {
		if (ebp == 0 || ebp < (unsigned int*)KERNBASE || ebp == (unsigned int*)0xffffffff)
			break;
		pcs[i] = ebp[1]; // saved %eip
		ebp = (unsigned int*)ebp[0]; // saved %ebp
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock* lock) {
	int r;
	pushcli();
	r = lock->locked && lock->cpu == mycpu();
	popcli();
	return r;
}

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void pushcli(void) {
	int eflags;

	eflags = readeflags();
	cli();
	if (mycpu()->ncli == 0)
		mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;
}

void popcli(void) {
	if (readeflags() & FL_IF)
		panic("popcli - interruptible");
	if (--mycpu()->ncli < 0)
		panic("popcli");
	if (mycpu()->ncli == 0 && mycpu()->intena)
		sti();
}
