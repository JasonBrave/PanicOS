/*
 * C++ std::allocator
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

#ifndef _LIBCPP_IMPL_ALLOCATOR_HPP
#define _LIBCPP_IMPL_ALLOCATOR_HPP

#include <cstddef>
#include <type_traits>

namespace std {

	template <class T>
	struct allocator {
		typedef T value_type;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef std::true_type propagate_on_container_move_assignment;
		typedef std::true_type is_always_equal;

	public:
		// constructor
		constexpr allocator() noexcept = default;
		constexpr allocator(const allocator& other) noexcept = default;
		template <class U>
		constexpr allocator(const allocator<U>& other) noexcept {};

		// destructor
		constexpr ~allocator() = default;

		[[nodiscard]] constexpr T* allocate(std::size_t n) {
			return reinterpret_cast<T*>(::operator new(n * sizeof(T)));
		}

		constexpr void deallocate(T* p, std::size_t n) {
			::operator delete(p);
		}
	};

	template <class T1, class T2>
	constexpr bool operator==(const allocator<T1>& lhs,
							  const allocator<T2>& rhs) noexcept {
		return true;
	}

} // namespace std

#endif
