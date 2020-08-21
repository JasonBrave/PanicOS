/*
 * System call numbers
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

#ifndef _SYSCALL_H
#define _SYSCALL_H

// System call numbers
#define SYS_fork 1
#define SYS_exit 2
#define SYS_wait 3
#define SYS_pipe 4
#define SYS_read 5
#define SYS_kill 6
#define SYS_exec 7
#define SYS_getcwd 8
#define SYS_chdir 9
#define SYS_dup 10
#define SYS_getpid 11
#define SYS_sbrk 12
#define SYS_sleep 13
#define SYS_uptime 14
#define SYS_open 15
#define SYS_write 16
#define SYS_mknod 17
#define SYS_unlink 18
#define SYS_link 19
#define SYS_mkdir 20
#define SYS_close 21
#define SYS_dir_open 22
#define SYS_dir_read 23
#define SYS_dir_close 24
#define SYS_file_get_size 25
#define SYS_lseek 26
#define SYS_file_get_mode 27
#define SYS_dynamic_load 28
#define SYS_kcall 29
#define SYS_message_send 30
#define SYS_message_receive 31
#define SYS_message_wait 32
#define SYS_getppid 33
#define SYS_proc_search 34
#define SYS_pty_create 35
#define SYS_pty_read_output 36
#define SYS_pty_write_input 37
#define SYS_pty_close 38
#define SYS_pty_switch 39
#define SYS_proc_status 40
#define SYS_module_load 41

#endif
