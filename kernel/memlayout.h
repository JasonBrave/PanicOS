// Memory layout

#ifndef _MEMLAYOUT_H
#define _MEMLAYOUT_H

#define EXTMEM 0x100000 // Start of extended memory
#define PHYSTOP 0x8000000 // Top physical memory
#define DEVSPACE 0xB0000000 // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000 // First kernel virtual address
#define KERNLINK (KERNBASE + EXTMEM) // Address where kernel is linked
#define INITRAMFS_BASE 0x80400000 // initramfs load address

#define V2P(a) (((unsigned int)(a)) - KERNBASE)
#define P2V(a) ((void*)((unsigned int)(a) + KERNBASE))

#define V2P_WO(x) ((x)-KERNBASE) // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE) // same as P2V, but without casts

#endif
