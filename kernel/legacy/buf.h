#ifndef _BUF_H
#define _BUF_H

#include <common/sleeplock.h>
#include <legacy/fs.h>

struct buf {
	int flags;
	unsigned int dev;
	unsigned int blockno;
	struct sleeplock lock;
	unsigned int refcnt;
	struct buf* prev; // LRU cache list
	struct buf* next;
	struct buf* qnext; // disk queue
	unsigned char data[BSIZE];
};
#define B_VALID 0x2 // buffer has been read from disk
#define B_DIRTY 0x4 // buffer needs to be written to disk

#endif
