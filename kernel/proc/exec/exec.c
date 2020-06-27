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
#include <filesystem/vfs/vfs.h>
#include <memlayout.h>
#include <param.h>

#include "elf.h"

int exec(char* path, char** argv) {
	char *s, *last;
	int i, off;
	unsigned int argc, sz, sp, ustack[3 + MAXARG + 1];
	struct elfhdr elf;
	// struct inode* ip;
	struct proghdr ph;
	pde_t *pgdir, *oldpgdir;
	struct proc* curproc = myproc();
	struct FileDesc fd;

	if (*path == '/') { // remove slash at beginning
		path++;
	}

	if (vfs_fd_open(&fd, path, O_READ) < 0) {
		cprintf("exec: fail\n");
		return -1;
	}

	pgdir = 0;

	// Check ELF header
	if (vfs_fd_read(&fd, &elf, sizeof(elf)) != sizeof(elf))
		goto bad;
	if (elf.magic != ELF_MAGIC)
		goto bad;

	if ((pgdir = setupkvm()) == 0)
		goto bad;

	// Load program into memory.
	sz = 0;
	for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
		if (vfs_fd_seek(&fd, off, SEEK_SET) < 0) {
			vfs_fd_close(&fd);
			return -1;
		}
		if (vfs_fd_read(&fd, (char*)&ph, sizeof(ph)) != sizeof(ph)) {
			vfs_fd_close(&fd);
			return -1;
		}
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
		if (loaduvm(pgdir, (char*)ph.vaddr, &fd, ph.off, ph.filesz) < 0)
			goto bad;
	}
	vfs_fd_close(&fd);

	// create the process stack
	if (allocuvm(pgdir, PROC_STACK_BOTTOM - PGSIZE, PROC_STACK_BOTTOM) == 0) {
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
	curproc->stack_size = PGSIZE;
	curproc->heap_size = 0;
	curproc->tf->eip = elf.entry; // main
	curproc->tf->esp = sp;
	switchuvm(curproc);
	freevm(oldpgdir);
	return 0;

bad:
	if (pgdir)
		freevm(pgdir);
	if (fd.used)
		vfs_fd_close(&fd);
	return -1;
}
