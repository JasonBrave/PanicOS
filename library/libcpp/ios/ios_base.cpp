/*
 * C++ std::ios_base
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

#include <ios>
#include <iostream>
#include <streambuf>

std::ios_base::ios_base() : format_flag(std::ios_base::dec) {}

std::ios_base::~ios_base() {
	if (callback_func) {
		callback_func(event::erase_event, *this, callback_index);
	}
}

void std::ios_base::register_callback(event_callback function, int index) {
	callback_func = function;
	callback_index = index;
}

bool std::ios_base::sync_with_stdio(bool sync) {
	return true;
}
