/*
 * Widget Toolkit Window base class
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
#include <iostream>
#include <libwm/wm.h>

#include "wmbase.hpp"

int GUI::WmBase::wm_init_count = 0;

GUI::WmBase::WmBase() {
	if (wm_init_count == 0) {
		if (!wm_init()) {
			std::cerr << "Cannot connect to Window Manager\n";
			std::exit(1);
		}
		wm_init_count = 1;
	}
}
