/*
 * exec call
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

#include "elf.h"

int exec(char* path, char** argv) {
	char *s, *last;
	int i, off;
	unsigned int argc, sz, sp, ustack[3 + MAXARG + 1];
	struct elfhdr elf;
	struct inode* ip;
	struct proghdr ph;
	pde_t *pgdir, *oldpgdir;
	struct proc* curproc = myproc();

	begin_op();

	if ((ip = namei(path)) == 0) {
		end_op();
		cprintf("exec: fail\n");
		return -1;
	}
	ilock(ip);
	pgdir = 0;

	// Check ELF header
	if (readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
		goto bad;
	if (elf.magic != ELF_MAGIC)
		goto bad;

	if ((pgdir = setupkvm()) == 0)
		goto bad;

	// Load program into memory.
	sz = 0;
	for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
		if (readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
			goto bad;
		if (ph.type != ELF_PROG_LOAD)
			continue;
		if (ph.memsz < ph.filesz)
			goto bad;
		if (ph.vaddr + ph.memsz < ph.vaddr)
			goto bad;
		if ((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
			goto bad;
		if (ph.vaddr % PGSIZE != 0)
			goto bad;
		if (loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
			goto bad;
	}
	iunlockput(ip);
	end_op();
	ip = 0;

	// Allocate two pages at the next page boundary.
	// Make the first inaccessible.  Use the second as the user stack.
	sz = PGROUNDUP(sz);
	if ((sz = allocuvm(pgdir, sz, sz + 2 * PGSIZE)) == 0)
		goto bad;
	clearpteu(pgdir, (char*)(sz - 2 * PGSIZE));
	sp = sz;

	// Push argument strings, prepare rest of stack in ustack.
	for (argc = 0; argv[argc]; argc++) {
		if (argc >= MAXARG)
			goto bad;
		sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
		if (copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
			goto bad;
		ustack[3 + argc] = sp;
	}
	ustack[3 + argc] = 0;

	ustack[0] = 0xffffffff; // fake return PC
	ustack[1] = argc;
	ustack[2] = sp - (argc + 1) * 4; // argv pointer

	sp -= (3 + argc + 1) * 4;
	if (copyout(pgdir, sp, ustack, (3 + argc + 1) * 4) < 0)
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
	curproc->tf->eip = elf.entry; // main
	curproc->tf->esp = sp;
	switchuvm(curproc);
	freevm(oldpgdir);
	return 0;

bad:
	if (pgdir)
		freevm(pgdir);
	if (ip) {
		iunlockput(ip);
		end_op();
	}
	return -1;
}
