/*
 * Interrupt handler
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

// Should be x86 arch-specific

#include <arch/x86/lapic.h>
#include <arch/x86/mmu.h>
#include <arch/x86/msi.h>
#include <common/spinlock.h>
#include <common/x86.h>
#include <core/proc.h>
#include <defs.h>
#include <driver/ata/ata.h>
#include <driver/pci/pci.h>
#include <driver/ps2/ps2.h>
#include <driver/x86/uart.h>
#include <memlayout.h>
#include <param.h>

#include "traps.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern unsigned int vectors[]; // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
unsigned int ticks;

void tvinit(void) {
	int i;

	for (i = 0; i < 256; i++)
		SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

	initlock(&tickslock, "time");
}

void idtinit(void) {
	lidt(idt, sizeof(idt));
}

// PAGEBREAK: 41
void trap(struct trapframe *tf) {
	if (tf->trapno == T_SYSCALL) {
		if (myproc()->killed)
			exit(-1);
		myproc()->tf = tf;
		syscall();
		if (myproc()->killed)
			exit(-1);
		return;
	}

	switch (tf->trapno) {
	case T_IRQ0 + IRQ_TIMER:
		if (cpuid() == 0) {
			acquire(&tickslock);
			ticks++;
			wakeup(&ticks);
			release(&tickslock);
		}
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_MOUSE:
		ps2_mouse_intr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_IDE:
	case T_IRQ0 + IRQ_IDE + 1:
		pata_adapter_legacy_intr(tf->trapno - T_IRQ0);
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_KBD:
		ps2_keyboard_intr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_COM1:
	case T_IRQ0 + IRQ_COM2:
		uart_intr(tf->trapno - T_IRQ0);
		lapiceoi();
		break;
	case T_IRQ0 + 5:
		pci_interrupt(5);
		lapiceoi();
		break;
	case T_IRQ0 + 9:
		pci_interrupt(9);
		lapiceoi();
		break;
	case T_IRQ0 + 10:
		pci_interrupt(10);
		lapiceoi();
		break;
	case T_IRQ0 + 11:
		pci_interrupt(11);
		lapiceoi();
		break;
	case T_IRQ0 + 7:
	case T_IRQ0 + IRQ_SPURIOUS:
		cprintf("cpu%d: spurious interrupt at %x:%x\n", cpuid(), tf->cs, tf->eip);
		lapiceoi();
		break;
	case T_MSI ... T_MSI_END:
		msi_intr(tf->trapno);
		lapiceoi();
		break;
	// PAGEBREAK: 13
	default:
		if (myproc() == 0 || (tf->cs & 3) == 0) {
			// In kernel, it must be our mistake.
			cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n", tf->trapno, cpuid(),
					tf->eip, rcr2());
			panic("trap");
		}
		// In user space, assume process misbehaved.
		cprintf("pid %d %s: trap %d err %d on cpu %d "
				"eip 0x%x addr 0x%x--kill proc\n",
				myproc()->pid, myproc()->name, tf->trapno, tf->err, cpuid(), tf->eip, rcr2());
		myproc()->killed = 1;
	}

	// Force process exit if it has been killed and is in user space.
	// (If it is still executing in the kernel, let it keep running
	// until it gets to the regular system call return.)
	if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
		exit(-1);

	// Force process to give up CPU on clock tick.
	// If interrupts were on while locks held, would need to check nlock.
	if (myproc() && myproc()->state == RUNNING && tf->trapno == T_IRQ0 + IRQ_TIMER)
		yield();

	// Check if the process has been killed since we yielded
	if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
		exit(-1);
}
