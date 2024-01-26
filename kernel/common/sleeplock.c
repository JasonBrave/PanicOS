/*
 * Sleeping Locks
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

#include <common/sleeplock.h>
#include <common/spinlock.h>
#include <common/x86.h>
#include <arch/x86/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

void initsleeplock(struct sleeplock* lk, const char* name) {
	initlock(&lk->lk, "sleep lock");
	lk->name = name;
	lk->locked = 0;
	lk->pid = 0;
}

void acquiresleep(struct sleeplock* lk) {
	acquire(&lk->lk);
	while (lk->locked) {
		sleep(lk, &lk->lk);
	}
	lk->locked = 1;
	lk->pid = myproc()->pid;
	release(&lk->lk);
}

void releasesleep(struct sleeplock* lk) {
	acquire(&lk->lk);
	lk->locked = 0;
	lk->pid = 0;
	wakeup(lk);
	release(&lk->lk);
}

int holdingsleep(struct sleeplock* lk) {
	int r;

	acquire(&lk->lk);
	r = lk->locked && (lk->pid == myproc()->pid);
	release(&lk->lk);
	return r;
}
