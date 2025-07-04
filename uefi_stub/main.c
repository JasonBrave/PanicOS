#include <stdint.h>

#include "uefi.h"

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian

// File header
struct elfhdr {
	uint32_t magic; // must equal ELF_MAGIC
	uint8_t elf[12];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} __attribute__((packed));

// Program section header
struct proghdr {
	uint32_t type;
	uint32_t off;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
} __attribute__((packed));

// Values for Proghdr type
#define ELF_PROG_LOAD 1
#define ELF_PROG_DYNAMIC 2
#define ELF_PROG_INTERP 3

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

// clang-format off
const uint8_t kernel_image[] = {
#embed "../kernel/kernel"
};
//clang-format on

void* memcpy(void* restrict s1, const void* restrict s2, volatile size_t n) {
	char* dest = s1;
	const char* from = s2;
	while (n--) {
		*dest++ = *from++;
	}
	return s1;
}

void (*kernel_entry_point)(void);

int kernel_load(void) {
	struct elfhdr elf;
	memcpy(&elf, kernel_image, sizeof(elf));
	if (elf.magic != ELF_MAGIC) {
		return 1;
	}
	kernel_entry_point = (void (*)(void))(elf.entry-0x80000000);

	struct proghdr ph;

	for (unsigned int off = elf.phoff; off < elf.phoff + elf.phnum * sizeof(ph);
		 off += sizeof(ph)) {
		memcpy(&ph, kernel_image + off,sizeof(ph));
		if (ph.type != ELF_PROG_LOAD) {
			continue;
		}

		memcpy((void*)ph.paddr, kernel_image + ph.off, ph.filesz);
	}

	return 0;
}

efi_status_t efi_main(efi_handle_t handle, struct efi_system_table *system_table) {
	efi_status_t status;

	system_table->out->output_string(system_table->out, u"Hello World!\n");

	if(kernel_load()){
		system_table->out->output_string(system_table->out, u"Kernel Load Failed\n");
	}else{
		system_table->out->output_string(system_table->out, u"Kernel Load Success\n");
	}

	kernel_entry_point();
	
	return 0;
}
