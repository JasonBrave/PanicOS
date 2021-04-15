#ifndef _ARCH_X86_MSI_H
#define _ARCH_X86_MSI_H

#include <common/types.h>

struct MSIMessage {
	uint64_t addr;
	uint32_t data;
};

void msi_init(void);
void msi_intr(int vector);
int msi_alloc_vector(struct MSIMessage* msg, void (*handler)(void*), void* private);
void msi_free_vector(const struct MSIMessage* msg);

#endif
