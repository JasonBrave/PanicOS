/*
 * exec call
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

#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <defs.h>

int exec(char* path, char** argv) {
	char *s, *last;
	unsigned int argc, sp, ustack[3 + MAXARG + 1];
	int sz;
	pdpte_t *pgdir = 0, *oldpgdir;
	struct proc* curproc = myproc();
	unsigned int entry, dynamic, interp = 0;

	// get a new page directory
	if ((pgdir = setupkvm()) == 0)
		goto bad;

	// Load program into memory.
	if ((sz = proc_elf_load(pgdir, 0, path, &entry, &dynamic, &interp)) < 0) {
		goto bad;
	}

	// create the process stack
	if (allocuvm(pgdir, PROC_STACK_BOTTOM - PGSIZE, PROC_STACK_BOTTOM, PTE_W | PTE_U) == 0) {
		goto bad;
	}
	sp = PROC_STACK_BOTTOM;

	// Push argument strings, prepare rest of stack in ustack.
	for (argc = 0; argv[argc]; argc++) {
		if (argc >= MAXARG)
			goto bad;
		sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
		if (copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
			goto bad;
		ustack[5 + argc] = sp;
	}
	ustack[5 + argc] = 0;

	ustack[0] = 0xffffffff; // fake return PC
	ustack[1] = argc; // int argc
	ustack[2] = sp - (argc + 1) * 4; // char* argv[]
	ustack[3] = 0; // char* envp[]
	ustack[4] = interp; // const char* interp

	sp -= (5 + argc + 1) * 4;
	if (copyout(pgdir, sp, ustack, (5 + argc + 1) * 4) < 0)
		goto bad;

	// Save program name for debugging.
	for (last = s = path; *s; s++)
		if (*s == '/')
			last = s + 1;
	safestrcpy(curproc->name, last, sizeof(curproc->name));

	// Commit to the user image.
	oldpgdir = curproc->pgdir;
	curproc->pgdir = pgdir;
	curproc->sz = sz;
	curproc->stack_size = PGSIZE;
	curproc->heap_size = 0;
	curproc->dyn_base = PROC_DYNAMIC_BOTTOM;
	curproc->tf->eip = entry; // _start
	curproc->tf->esp = sp;
	switchuvm(curproc);
	freevm(oldpgdir);
	return 0;

bad:
	if (pgdir)
		freevm(pgdir);
	return -1;
}
