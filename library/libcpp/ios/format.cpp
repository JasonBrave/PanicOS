/*
 * C++ ios formatting functions
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

namespace std {
	ios_base::fmtflags ios_base::flags() const {
		return format_flag;
	}

	ios_base::fmtflags ios_base::flags(ios_base::fmtflags flags) {
		fmtflags oldflg = format_flag;
		format_flag = flags;
		return oldflg;
	}

	ios_base::fmtflags ios_base::setf(ios_base::fmtflags flags) {
		fmtflags oldflg = format_flag;
		format_flag |= flags;
		return oldflg;
	}

	ios_base::fmtflags ios_base::setf(ios_base::fmtflags flags, ios_base::fmtflags mask) {
		fmtflags oldflg = format_flag;
		format_flag = (format_flag & ~mask) | (flags & mask);
		return oldflg;
	}

	void ios_base::unsetf(ios_base::fmtflags flags) {
		format_flag &= ~flags;
	}

	ios_base& dec(std::ios_base& str) {
		str.unsetf(std::ios_base::basefield);
		str.setf(std::ios_base::dec);
		return str;
	}

	ios_base& hex(std::ios_base& str) {
		str.unsetf(std::ios_base::basefield);
		str.setf(std::ios_base::hex);
		return str;
	}

	ios_base& oct(std::ios_base& str) {
		str.unsetf(std::ios_base::basefield);
		str.setf(std::ios_base::oct);
		return str;
	}
} // namespace std
