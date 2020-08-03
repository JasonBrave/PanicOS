/*
 * C++ standard streams
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

#include <cstdio>
#include <ostream>
#include <streambuf>

namespace {
	std::streambuf cout_buf(stdout);
	std::streambuf cerr_buf(stderr);
	std::streambuf clog_buf(stderr);
} // namespace

namespace std {
	ostream cout(&cout_buf);
	ostream cerr(&cerr_buf);
	ostream clog(&clog_buf);
} // namespace std
