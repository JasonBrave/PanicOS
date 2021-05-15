/*
 * Process management header
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

#ifndef _PROC_H
#define _PROC_H

#include <common/spinlock.h>
#include <core/mmu.h>
#include <filesystem/vfs/vfs.h>
#include <param.h>

// Per-CPU state
struct cpu {
	unsigned char apicid; // Local APIC ID
	struct context* scheduler; // swtch() here to enter scheduler
	struct taskstate ts; // Used by x86 to find stack for interrupt
	struct segdesc gdt[NSEGS]; // x86 global descriptor table
	volatile unsigned int started; // Has the CPU started?
	int ncli; // Depth of pushcli nesting.
	int intena; // Were interrupts enabled before pushcli?
	struct proc* proc; // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern unsigned int ncpu;

// PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
	unsigned int edi;
	unsigned int esi;
	unsigned int ebx;
	unsigned int ebp;
	unsigned int eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

#define MESSAGE_MAX 64

struct Message {
	int pid, size;
	void* addr;
};

struct MessageQueue {
	struct spinlock lock;
	int begin, end;
	struct Message queue[MESSAGE_MAX];
};

// Per-process state
struct proc {
	unsigned int sz; // size of executable image (bytes)
	unsigned int stack_size; // size of process stack (bytes)
	unsigned int heap_size; // size of process heap (bytes)
	pdpte_t* pgdir; // Page table
	char* kstack; // Bottom of kernel stack for this process
	enum procstate state; // Process state
	int pid; // Process ID
	struct proc* parent; // Parent process
	struct trapframe* tf; // Trap frame for current syscall
	struct context* context; // swtch() here to run process
	void* chan; // If non-zero, sleeping on chan
	int killed; // If non-zero, have been killed
	char name[16]; // Process name (debugging)
	struct FileDesc files[PROC_FILE_MAX]; // open files
	struct VfsPath cwd; // working directory
	unsigned int dyn_base; // dynamic library load base
	struct MessageQueue msgqueue; // message queue
	int pty; // Pseudoterminal
	int exit_status;
};

#endif
