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

#ifndef _LIBGUI_WMBASE_HPP
#define _LIBGUI_WMBASE_HPP

namespace GUI {

	class WmBase {
		static int wm_init_count;

	public:
		WmBase();
		WmBase(const WmBase&) = delete;
		WmBase(WmBase&&) = delete;
	};

} // namespace GUI

#endif
