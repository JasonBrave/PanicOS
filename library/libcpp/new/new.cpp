/*
 * C++ operator new
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

#include <cstdlib>
#include <new>

namespace {
	std::new_handler new_handler_func = nullptr;
}

[[nodiscard]] void* operator new(std::size_t count) {
	void* ptr;
	while (!(ptr = std::malloc(count))) {
		if (new_handler_func) {
			new_handler_func();
		} else {
			// throw std::bad_alloc;
		}
	}
	return ptr;
}

[[nodiscard]] void* operator new[](std::size_t count) {
	return ::operator new(count);
}

std::new_handler std::get_new_handler() noexcept {
	return new_handler_func;
}

std::new_handler std::set_new_handler(std::new_handler new_p) noexcept {
	std::new_handler prev_handler = new_handler_func;
	new_handler_func = new_p;
	return prev_handler;
}
