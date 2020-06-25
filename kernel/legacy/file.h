#ifndef _FILE_H
#define _FILE_H

#include <common/sleeplock.h>
#include <legacy/fs.h>

struct file {
	enum { FD_NONE, FD_PIPE, FD_INODE } type;
	int ref; // reference count
	char readable;
	char writable;
	struct pipe* pipe;
	struct inode* ip;
	unsigned int off;
};

// in-memory copy of an inode
struct inode {
	unsigned int dev; // Device number
	unsigned int inum; // Inode number
	int ref; // Reference count
	struct sleeplock lock; // protects everything below here
	int valid; // inode has been read from disk?

	short type; // copy of disk inode
	short major;
	short minor;
	short nlink;
	unsigned int size;
	unsigned int addrs[NDIRECT + 1];
};

// table mapping major device number to
// device functions
struct devsw {
	int (*read)(struct inode*, char*, int);
	int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1

#endif
