#ifndef _MODLIB_KLIBC_H
#define _MODLIB_KLIBC_H

static inline void memset(void* s, char c, int n) {
	char* ss = s;
	for (int i = 0; i < n; i++) {
		ss[i] = c;
	}
}

static inline void memcpy(void* dest, const void* src, int n) {
	char* d = dest;
	const char* s = src;
	for (int i = 0; i < n; i++) {
		d[i] = s[i];
	}
}

#endif
