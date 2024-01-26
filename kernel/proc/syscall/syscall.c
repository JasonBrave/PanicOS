/*
 * Default system call handler
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
#include <arch/x86/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int fetchint(unsigned int addr, int* ip) {
	*ip = *(int*)(addr);
	return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(unsigned int addr, char** pp) {
	char* s;

	*pp = (char*)addr;
	for (s = *pp;; s++) {
		if (*s == 0)
			return s - *pp;
	}
	return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int* ip) {
	return fetchint((myproc()->tf->esp) + 4 + 4 * n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char** pp, int size) {
	int i;

	if (argint(n, &i) < 0)
		return -1;
	*pp = (char*)i;
	return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char** pp) {
	int addr;
	if (argint(n, &addr) < 0)
		return -1;
	return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_getcwd(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_dir_open(void);
extern int sys_dir_read(void);
extern int sys_dir_close(void);
extern int sys_file_get_size(void);
extern int sys_lseek(void);
extern int sys_file_get_mode(void);
extern int sys_dynamic_load(void);
extern int sys_kcall(void);
extern int sys_message_send(void);
extern int sys_message_receive(void);
extern int sys_message_wait(void);
extern int sys_getppid(void);
extern int sys_proc_search(void);
extern int sys_pty_create(void);
extern int sys_pty_read_output(void);
extern int sys_pty_write_input(void);
extern int sys_pty_close(void);
extern int sys_pty_switch(void);
extern int sys_proc_status(void);
extern int sys_module_load(void);

static int (*syscalls[])(void) = {
	[SYS_fork] = sys_fork,
	[SYS_exit] = sys_exit,
	[SYS_wait] = sys_wait,
	[SYS_pipe] = sys_pipe,
	[SYS_read] = sys_read,
	[SYS_kill] = sys_kill,
	[SYS_exec] = sys_exec,
	[SYS_getcwd] = sys_getcwd,
	[SYS_chdir] = sys_chdir,
	[SYS_dup] = sys_dup,
	[SYS_getpid] = sys_getpid,
	[SYS_sbrk] = sys_sbrk,
	[SYS_sleep] = sys_sleep,
	[SYS_uptime] = sys_uptime,
	[SYS_open] = sys_open,
	[SYS_write] = sys_write,
	[SYS_mknod] = sys_mknod,
	[SYS_unlink] = sys_unlink,
	[SYS_link] = sys_link,
	[SYS_mkdir] = sys_mkdir,
	[SYS_close] = sys_close,
	[SYS_dir_open] = sys_dir_open,
	[SYS_dir_read] = sys_dir_read,
	[SYS_dir_close] = sys_dir_close,
	[SYS_file_get_size] = sys_file_get_size,
	[SYS_lseek] = sys_lseek,
	[SYS_file_get_mode] = sys_file_get_mode,
	[SYS_dynamic_load] = sys_dynamic_load,
	[SYS_kcall] = sys_kcall,
	[SYS_message_send] = sys_message_send,
	[SYS_message_receive] = sys_message_receive,
	[SYS_message_wait] = sys_message_wait,
	[SYS_getppid] = sys_getppid,
	[SYS_proc_search] = sys_proc_search,
	[SYS_pty_create] = sys_pty_create,
	[SYS_pty_read_output] = sys_pty_read_output,
	[SYS_pty_write_input] = sys_pty_write_input,
	[SYS_pty_close] = sys_pty_close,
	[SYS_pty_switch] = sys_pty_switch,
	[SYS_proc_status] = sys_proc_status,
	[SYS_module_load] = sys_module_load,
};

void syscall(void) {
	unsigned int num;
	struct proc* curproc = myproc();

	num = curproc->tf->eax;
	if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
		curproc->tf->eax = syscalls[num]();
	} else {
		cprintf("%d %s: unknown sys call %d\n", curproc->pid, curproc->name, num);
		curproc->tf->eax = -1;
	}
}
