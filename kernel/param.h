#ifndef _PARAM_H
#define _PARAM_H

#define NPROC 64 // maximum number of processes
#define KSTACKSIZE 4096 // size of per-process kernel stack
#define NCPU 8 // maximum number of CPUs
#define NOFILE 16 // open files per process
#define NFILE 100 // open files per system
#define NINODE 50 // maximum number of active i-nodes
#define NDEV 10 // maximum major device number
#define ROOTDEV 1 // device number of file system root disk
#define MAXARG 32 // max exec arguments
#define MAXOPBLOCKS 10 // max # of blocks any FS op writes
#define LOGSIZE (MAXOPBLOCKS * 3) // max data blocks in on-disk log
#define NBUF (MAXOPBLOCKS * 3) // size of disk block cache
#define FSSIZE 1000 // size of file system in blocks
#define PROC_FILE_MAX 8 // maxium number of file for a process
#define PTY_MAX 8 // maxnum number of Pseudo Terminal

#define PROC_STACK_BOTTOM 0x20000000 // bottom of stack in user space
#define PROC_HEAP_BOTTOM 0x20000000 // bottom of process heap
#define PROC_DYNAMIC_BOTTOM 0x40000000 // bottom of dynamic library space
#define PROC_MMAP_BOTTOM 0x70000000 // bottom of process mmap
#define PROC_MODULE_BOTTOM 0x90000000

#endif
