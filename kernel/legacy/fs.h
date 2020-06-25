// On-disk file system format.
// Both the kernel and user programs use this header file.

#ifndef _FS_H
#define _FS_H

#define ROOTINO 1 // root i-number
#define BSIZE 512 // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
	unsigned int size; // Size of file system image (blocks)
	unsigned int nblocks; // Number of data blocks
	unsigned int ninodes; // Number of inodes.
	unsigned int nlog; // Number of log blocks
	unsigned int logstart; // Block number of first log block
	unsigned int inodestart; // Block number of first inode block
	unsigned int bmapstart; // Block number of first free map block
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(unsigned int))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
	short type; // File type
	short major; // Major device number (T_DEV only)
	short minor; // Minor device number (T_DEV only)
	short nlink; // Number of links to inode in file system
	unsigned int size; // Size of file (bytes)
	unsigned int addrs[NDIRECT + 1]; // Data block addresses
};

// Inodes per block.
#define IPB (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb) ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB (BSIZE * 8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b / BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	unsigned short inum;
	char name[DIRSIZ];
};

#endif
