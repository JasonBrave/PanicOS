#include <stdint.h>

#include "elf.h"
#include "uefi.h"

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
		system_table->out->output_string(system_table->out,u"Kernel Load Success\n");
	}

	kernel_entry_point();
	
	return 0;
}
