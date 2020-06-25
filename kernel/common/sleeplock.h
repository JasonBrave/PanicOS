#ifndef _SLEEPLOCK_H
#define _SLEEPLOCK_H

#include <common/spinlock.h>

// Long-term locks for processes
struct sleeplock {
	unsigned int locked; // Is the lock held?
	struct spinlock lk; // spinlock protecting this sleep lock

	// For debugging:
	const char* name; // Name of lock.
	int pid; // Process holding lock
};

void acquiresleep(struct sleeplock*);
void releasesleep(struct sleeplock*);
int holdingsleep(struct sleeplock*);
void initsleeplock(struct sleeplock*, const char*);

#endif
