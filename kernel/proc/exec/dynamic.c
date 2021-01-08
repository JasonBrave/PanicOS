/*
 * load dynamic library
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

#include "elf.h"
#include <core/proc.h>
#include <defs.h>

int proc_load_dynamic(struct proc* proc, const char* name, unsigned int* dynamic,
					  unsigned int* entry) {
	int sz;
	unsigned int interp;
	unsigned int load_base = myproc()->dyn_base;
	if ((sz = proc_elf_load(proc->pgdir, load_base, name, entry, dynamic, &interp)) < 0) {
		return 0;
	}
	myproc()->dyn_base += PGROUNDUP(sz);
	return load_base;
}
