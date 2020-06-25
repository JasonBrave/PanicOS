#ifndef _SPINLOCK_H
#define _SPINLOCK_H

// Mutual exclusion lock.
struct spinlock {
	unsigned int locked; // Is the lock held?

	// For debugging:
	const char* name; // Name of lock.
	struct cpu* cpu; // The cpu holding the lock.
	unsigned int pcs[10]; // The call stack (an array of program counters)
						  // that locked the lock.
};

void acquire(struct spinlock*);
void getcallerpcs(void*, unsigned int*);
int holding(struct spinlock*);
void initlock(struct spinlock*, const char*);
void release(struct spinlock*);
void pushcli(void);
void popcli(void);

#endif
