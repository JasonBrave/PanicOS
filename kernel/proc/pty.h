#ifndef _CORE_PTY_H
#define _CORE_PTY_H

#include <common/spinlock.h>
#include <param.h>

struct PseudoTerminal {
	struct proc* owner;
	struct spinlock lock;
	char *input_buffer, *output_buffer;
	int input_begin, input_end;
	int output_begin, output_end;
};

extern struct PseudoTerminal pty[PTY_MAX];

void pty_init(void);
int pty_create(void);
int pty_close(int ptyid);
int pty_read(int ptyid, char* buf, int n);
int pty_write(int ptyid, const char* buf, int n);
int pty_read_output(int ptyid, char* buf, int n);
int pty_write_input(int ptyid, const char* buf, int n);

#endif
