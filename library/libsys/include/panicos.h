/*
 * System call header
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

#ifndef _LIBSYS_PANICOS_H
#define _LIBSYS_PANICOS_H

#ifdef __cplusplus
extern "C" {
#endif

int fork(void);
#ifdef __cplusplus
[[noreturn]] int proc_exit(int);
#else
_Noreturn int proc_exit(int);
#endif
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, const char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int getcwd(char* dir);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int dir_open(const char* dirname);
int dir_read(int handle, char* buffer);
int dir_close(int handle);
int file_get_size(const char* filename);
int lseek(int fd, int offset, int whence);
int file_get_mode(const char* filename);
void* dynamic_load(const char* name, void** dynamic, void** entry);
int kcall(const char* name, unsigned int arg);
int message_send(int pid, int size, const void* data);
int message_receive(void* data);
int message_wait(void* data);
int getppid(void);
int proc_search(const char* name);
int pty_create(void);
int pty_read_output(int pty, char* buffer, int size);
int pty_write_input(int pty, const char* buffer, int size);
int pty_close(int pty);
int pty_switch(int pty);
int proc_status(int pid, int* exit_status);
int module_load(const char* name);

enum OpenMode {
	O_READ = 1,
	O_WRITE = 2,
	O_CREATE = 4,
	O_APPEND = 8,
	O_TRUNC = 16,
};

enum FileSeekMode {
	FILE_SEEK_SET,
	FILE_SEEK_CUR,
	FILE_SEEK_END,
};

enum ProcStatus {
	PROC_RUNNING,
	PROC_EXITED,
	PROC_NOT_EXIST,
};

#ifdef __cplusplus
}
#endif

#endif
