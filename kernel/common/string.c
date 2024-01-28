/*
 * Common string functions
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

#ifndef __riscv
#include <common/x86.h>
#endif

void *memset(void *dst, int c, unsigned int n) {
#ifndef __riscv
	if ((int)dst % 4 == 0 && n % 4 == 0) {
		c &= 0xFF;
		stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
	} else
		stosb(dst, c, n);
#else
	char *p = dst;
	while (n--) {
		*p = c;
		p++;
	}
#endif
	return dst;
}

volatile void *memset_volatile(volatile void *dst, int c, unsigned int n) {
	volatile char *p = dst;
	while (n--) {
		*p = c;
		p++;
	}
	return dst;
}

int memcmp(const void *v1, const void *v2, unsigned int n) {
	const unsigned char *s1, *s2;

	s1 = v1;
	s2 = v2;
	while (n-- > 0) {
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;
	}

	return 0;
}

int memcmp_volatile(const volatile void *v1, const volatile void *v2, unsigned int n) {
	const volatile unsigned char *s1, *s2;

	s1 = v1;
	s2 = v2;
	while (n-- > 0) {
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;
	}

	return 0;
}

void *memmove(void *dst, const void *src, unsigned int n) {
	const char *s;
	char *d;

	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}

volatile void *memmove_volatile(volatile void *dst, const volatile void *src, unsigned int n) {
	const volatile char *s;
	volatile char *d;

	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void *memcpy(void *dst, const void *src, unsigned int n) {
	return memmove(dst, src, n);
}

int strncmp(const char *p, const char *q, unsigned int n) {
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	return (unsigned char)*p - (unsigned char)*q;
}

char *strncpy(char *s, const char *t, int n) {
	char *os;

	os = s;
	while (n-- > 0 && (*s++ = *t++) != 0)
		;
	while (n-- > 0)
		*s++ = 0;
	return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char *safestrcpy(char *s, const char *t, int n) {
	char *os;

	os = s;
	if (n <= 0)
		return os;
	while (--n > 0 && (*s++ = *t++) != 0)
		;
	*s = 0;
	return os;
}

int strlen(const char *s) {
	int n;

	for (n = 0; s[n]; n++)
		;
	return n;
}
