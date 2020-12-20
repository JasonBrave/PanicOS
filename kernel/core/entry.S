# Kernel entry point

#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
.p2align 2
.text
.globl multiboot_header
multiboot_header:
	#define magic 0x1badb002
	#define flags 4
	.long magic
	.long flags
	.long (-magic-flags)
# address field
 	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
# graphic field vga text
	.long 1
	.long 0
	.long 0
	.long 0

# Entering xv6 on boot processor, with paging off.
.globl _start
_start:
	# Load the real GDT
	lgdt gdtdesc - KERNBASE
	ljmp $(SEG_KCODE << 3), $(entry - KERNBASE)
entry:
	movl	%eax, %esi
	movl	%ebx, %edi
	# Set up the protected-mode data segment registers
	movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
	movw    %ax, %ds                # -> DS: Data Segment
	movw    %ax, %es                # -> ES: Extra Segment
	movw    %ax, %ss                # -> SS: Stack Segment
	movw    $0, %ax                 # Zero segments not ready for use
	movw    %ax, %fs                # -> FS
	movw    %ax, %gs                # -> GS
	# Turn on page size extension for 4Mbyte pages
	movl    %cr4, %eax
	orl     $(CR4_PSE), %eax
	movl    %eax, %cr4
	# Set page directory
	movl    $(V2P_WO(entrypgdir)), %eax
	movl    %eax, %cr3
	# Turn on paging.
	movl    %cr0, %eax
	orl     $(CR0_PG|CR0_WP), %eax
	movl    %eax, %cr0

	# Set up the stack pointer.
	movl $(stack + KSTACKSIZE), %esp

	# Jump to main(), and switch to executing at
	# high addresses. The indirect call is needed because
	# the assembler produces a PC-relative instruction
	# for a direct jump.
	pushl	%edi
	pushl	%esi
	mov		$kmain, %eax
	call	*%eax

# Bootstrap GDT
.section .rodata
.p2align 2                                # force 4 byte alignment
gdt:
	SEG_NULLASM                             # null seg
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
	SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg

gdtdesc:
	.word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
	.long   gdt - KERNBASE    
	
.comm stack, KSTACKSIZE
