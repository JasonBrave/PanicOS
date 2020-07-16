#ifndef _PROG_KCALL_H
#define _PROG_KCALL_H

extern struct KCallTable {
	char name[32];
	int (*handler)(unsigned int);
} kcall_table[256];

void kcall_init(void);
void kcall_set(const char* name, int (*handler)(unsigned int));
int kcall(const char* name, unsigned int arg);

#endif
