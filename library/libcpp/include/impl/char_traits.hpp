/*
 * C++ std::char_traits
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

#ifndef _LIBCPP_IMPL_CHAR_TRAITS_HPP
#define _LIBCPP_IMPL_CHAR_TRAITS_HPP

#include <cstddef>
#include <iosfwd>

namespace std {
	template <class CharT>
	struct char_traits {
		typedef CharT char_type;
		typedef int int_type;
		typedef std::streamoff off_type;
		typedef std::streampos pos_type;
		typedef void state_type;

		static constexpr std::size_t length(const char_type* s) {
			std::size_t i = 0;
			while (*s) {
				i++;
				s++;
			}
			return i;
		}

		static constexpr int_type to_int_type(char_type c) noexcept {
			return static_cast<int_type>(c);
		}
	};

	template <>
	struct char_traits<char> {
		typedef char char_type;
		typedef int int_type;
		typedef std::streamoff off_type;
		typedef std::streampos pos_type;
		typedef void state_type;

		static constexpr void assign(char_type& r, const char_type& a) noexcept {
			r = a;
		}

		static constexpr char_type* assign(char_type* p, std::size_t count,
										   char_type a) {
			while (count--) {
				p[count] = a;
			}
			return p;
		}

		static constexpr bool eq(char_type a, char_type b) noexcept {
			return (a == b);
		}

		static constexpr bool lt(char_type a, char_type b) noexcept {
			return (a < b);
		}

		static constexpr char_type* move(char_type* dest, const char_type* src,
										 std::size_t count) {
			while (count--) {
				dest[count] = src[count];
			}
			return dest;
		}

		static constexpr char_type* copy(char_type* dest, const char_type* src,
										 std::size_t count) {
			while (count--) {
				dest[count] = src[count];
			}
			return dest;
		}

		static constexpr int compare(const char_type* s1, const char_type* s2,
									 std::size_t count) {
			for (std::size_t i = 0; i < count; i++) {
				if (lt(s1[i], s2[i])) {
					return -1;
				} else if (lt(s2[i], s1[i])) {
					return 1;
				}
			}
			return 0;
		}

		static constexpr std::size_t length(const char_type* s) {
			std::size_t i = 0;
			while (*s) {
				i++;
				s++;
			}
			return i;
		}

		static constexpr const char_type* find(const char_type* p, std::size_t count,
											   const char_type& ch) {
			for (std::size_t i = 0; i < count; i++) {
				if (eq(p[i], ch)) {
					return p + i;
				}
			}
			return nullptr;
		}

		static constexpr char_type to_char_type(int_type c) noexcept {
			return static_cast<char_type>(c);
		}

		static constexpr int_type to_int_type(char_type c) noexcept {
			return static_cast<int_type>(c);
		}

		static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept {
			return eq(to_char_type(c1), to_char_type(c2));
		}

		static constexpr int_type eof() noexcept {
			return -1;
		}

		static constexpr int_type not_eof(int_type e) noexcept {
			if (!eq_int_type(e, eof())) {
				return e;
			} else {
				return 0;
			}
		}
	};
} // namespace std

#endif
