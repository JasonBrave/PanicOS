/*
 * Dynamic Linker
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

#include <panicos.h>
#include <string.h>

#include "elf.h"

#define DL_INFO_MAX 64
#define SYMBOL_TABLE_MAX 1024

typedef struct {
	const char* name; // name of the library
	void* load; // load base address
	Elf32_Dyn* dynamic; // address of the dynamic section
	Elf32_Sym* dynsym; // address of dynamic symbol table
	int dynsym_num; // number of dynamic symbol
	char* dynstr; // dynamic symbol string table
	Elf32_Rel* reldyn; // relocation table
	int reldyn_num; // number of relocation entry
	Elf32_Rel* relplt; // PLT relocation tab
	int relplt_num; // number of PLT relocation
	void (**fini_array)(void); // global destructor
	int fini_array_size;
} DlInfo;

typedef struct {
	const char* name;
	void* addr;
} Symbol;

DlInfo dl_info[DL_INFO_MAX];
Symbol symbol_table[SYMBOL_TABLE_MAX];

Symbol* ld_insert_symbol(const char* name, void* sym) {
	for (int i = 0; i < SYMBOL_TABLE_MAX; i++) {
		if (!symbol_table[i].name) { // enpty entry
			symbol_table[i].name = name;
			symbol_table[i].addr = sym;
			return symbol_table + i;
		}
		if (strcmp(symbol_table[i].name, name) == 0) { // already defined
			symbol_table[i].addr = sym;
			return symbol_table + i;
		}
	}
	return NULL;
}

void* ld_lookup_symbol(const char* name) {
	for (int i = 0; i < SYMBOL_TABLE_MAX; i++) {
		if (strcmp(symbol_table[i].name, name) == 0) {
			return symbol_table[i].addr;
		}
	}
	return NULL;
}

// relocate whole .rel.dyn or .rel.plt section
// rel_table: pointer to relocation table
// dynsym: pointer to dynamic symbol table
// num: number of relocation
void ld_relocate(void* load_base, Elf32_Rel* rel_table, Elf32_Sym* dynsym, char* strtab, int num) {
	for (int i = 0; i < num; i++) {
		Elf32_Rel* rel = rel_table + i;
		switch (ELF32_R_TYPE(rel->r_info)) {
		case R_386_COPY: {
			void* sym = ld_lookup_symbol(strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name);
			if (!sym) {
				write(2, strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name,
					  strlen(strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name));
				proc_exit(-1);
			}
			memcpy(load_base + rel->r_offset, sym, dynsym[ELF32_R_SYM(rel->r_info)].st_size);
			break;
		}
		case R_386_GLOB_DAT:
		case R_386_JMP_SLOT: {
			void* sym = ld_lookup_symbol(strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name);
			if (!sym) {
				write(2, strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name,
					  strlen(strtab + dynsym[ELF32_R_SYM(rel->r_info)].st_name));
				proc_exit(-1);
			}
			*(void**)(load_base + rel->r_offset) = sym;
			break;
		}
		case R_386_RELATIVE: {
			*(void**)(load_base + rel->r_offset) += (unsigned int)load_base;
			break;
		}
		}
	}
}

// name: name of the library, e.g. libc.so
// return pointer to dlinfo, on error, returns null
DlInfo* ld_load_library(const char* name) {
	// check if the library is loaded or not
	for (int i = 0; i < DL_INFO_MAX; i++) {
		if (strcmp(dl_info[i].name, name) == 0) {
			return dl_info + i;
		}
	}
	// find a dl_info entry
	DlInfo* dl = NULL;
	for (int i = 0; i < DL_INFO_MAX; i++) {
		if (!dl_info[i].name) {
			dl = dl_info + i;
		}
	}
	if (!dl) {
		return NULL;
	}
	// load the library
	Elf32_Dyn* dyn;
	void* entry;
	char lib_file[50] = "/lib/";
	strcat(lib_file, name);
	dl->load = dynamic_load(lib_file, (void**)&dyn, &entry);
	if (!dl->load) {
		return NULL;
	}
	dl->name = name;
	dl->dynamic = dyn;

	int needed[16];
	memset(needed, 0, sizeof(needed));
	void (**init_array)(void) = 0;
	int init_array_size = 0;
	// load data into dl_info
	while (dyn->d_tag) {
		switch (dyn->d_tag) {
		case DT_NEEDED:
			for (int i = 0; i < 16; i++) {
				if (!needed[i]) {
					needed[i] = dyn->d_un.d_val;
					break;
				}
			}
			break;
		case DT_SYMTAB:
			dl->dynsym = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_STRTAB:
			dl->dynstr = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_REL:
			dl->reldyn = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_JMPREL:
			dl->relplt = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_RELSZ:
			dl->reldyn_num = dyn->d_un.d_val / sizeof(Elf32_Rel);
			break;
		case DT_PLTRELSZ:
			dl->relplt_num = dyn->d_un.d_val / sizeof(Elf32_Rel);
			break;
		case DT_INIT_ARRAY:
			init_array = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_INIT_ARRAYSZ:
			init_array_size = dyn->d_un.d_val / sizeof(void (*)(void));
			break;
		case DT_FINI_ARRAY:
			dl->fini_array = dl->load + dyn->d_un.d_ptr;
			break;
		case DT_FINI_ARRAYSZ:
			dl->fini_array_size = dyn->d_un.d_val / sizeof(void (*)(void));
			break;
		}
		dyn++;
	}
	dl->dynsym_num = ((int)dl->dynstr - (int)dl->dynsym) / sizeof(Elf32_Sym);
	// load needed library
	for (int i = 0; i < 16; i++) {
		if (!needed[i]) {
			break;
		}
		if (!ld_load_library(dl->dynstr + needed[i])) {
			write(2, dl->dynstr + needed[i], strlen(dl->dynstr + needed[i]));
			proc_exit(-1);
		}
	}
	// relocate PLT
	if (dl->relplt) {
		ld_relocate(dl->load, dl->relplt, dl->dynsym, dl->dynstr, dl->relplt_num);
	}
	// relocate GOT
	if (dl->reldyn) {
		ld_relocate(dl->load, dl->reldyn, dl->dynsym, dl->dynstr, dl->reldyn_num);
	}
	// add symbol to global symbol table
	for (int i = 0; i < dl->dynsym_num; i++) {
		if (dl->dynsym[i].st_shndx == STN_UNDEF) {
			continue;
		}
		if ((ELF32_ST_BIND(dl->dynsym[i].st_info) != STB_GLOBAL) &&
			(ELF32_ST_BIND(dl->dynsym[i].st_info) != STB_WEAK)) {
			continue;
		}
		if (!ld_insert_symbol(dl->dynstr + dl->dynsym[i].st_name,
							  dl->load + dl->dynsym[i].st_value)) {
			return NULL;
		}
	}
	// call global constructors
	if (init_array && init_array_size) {
		for (int i = 0; i < init_array_size; i++) {
			init_array[i]();
		}
	}

	return dl;
}

// call global destructor of shared library
void _dl_fini(void) {
	for (int i = DL_INFO_MAX - 1; i >= 0; i--) {
		if (dl_info[i].fini_array && dl_info[i].fini_array_size) {
			for (int j = 0; j < dl_info[i].fini_array_size; j++) {
				dl_info[i].fini_array[j]();
			}
		}
	}
}

// dynamic: dynamic segment of the executeable
void ld_main(Elf32_Dyn* dynamic) {
	memset(dl_info, 0, sizeof(dl_info));
	memset(symbol_table, 0, sizeof(symbol_table));

	Elf32_Rel *relplt = 0, *reldyn = 0; // PLT relocation tab
	int relplt_num, reldyn_num; // number of PLT relocation
	Elf32_Sym* dynsym;
	char* dynstr = 0;
	void (**fini_array)(void) = 0;
	int fini_array_size = 0;
	int needed[16];
	memset(needed, 0, sizeof(needed));

	Elf32_Dyn* dyn = dynamic;
	while (dyn->d_tag) {
		switch (dyn->d_tag) {
		case DT_NEEDED:
			for (int i = 0; i < 16; i++) {
				if (!needed[i]) {
					needed[i] = dyn->d_un.d_val;
					break;
				}
			}
			break;
		case DT_SYMTAB:
			dynsym = (Elf32_Sym*)dyn->d_un.d_ptr;
			break;
		case DT_STRTAB:
			dynstr = (char*)dyn->d_un.d_ptr;
			break;
		case DT_REL:
			reldyn = (Elf32_Rel*)dyn->d_un.d_ptr;
			break;
		case DT_JMPREL:
			relplt = (Elf32_Rel*)dyn->d_un.d_ptr;
			break;
		case DT_RELSZ:
			reldyn_num = dyn->d_un.d_val / sizeof(Elf32_Rel);
			break;
		case DT_PLTRELSZ:
			relplt_num = dyn->d_un.d_val / sizeof(Elf32_Rel);
			break;
		case DT_FINI_ARRAY:
			fini_array = (void*)dyn->d_un.d_ptr;
			break;
		case DT_FINI_ARRAYSZ:
			fini_array_size = dyn->d_un.d_val / sizeof(void (*)(void));
			break;
		}
		dyn++;
	}
	// add some symbol to global symbol table
	if (fini_array && fini_array_size) {
		ld_insert_symbol("__fini_array_start", fini_array);
		ld_insert_symbol("__fini_array_end", fini_array + fini_array_size);
	} else {
		ld_insert_symbol("__fini_array_start", (void*)0xffffffff);
		ld_insert_symbol("__fini_array_end", (void*)0xffffffff);
	}
	ld_insert_symbol("_dl_fini", _dl_fini);
	// load library
	for (int i = 0; i < 16; i++) {
		if (!needed[i]) {
			break;
		}
		if (!ld_load_library(dynstr + needed[i])) {
			write(2, dynstr + needed[i], strlen(dynstr + needed[i]));
			proc_exit(-1);
		}
	}
	// relocate symbol in main executeable
	if (relplt) {
		ld_relocate(0, relplt, dynsym, dynstr, relplt_num);
	}
	if (reldyn) {
		ld_relocate(0, reldyn, dynsym, dynstr, reldyn_num);
	}
}
