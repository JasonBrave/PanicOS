#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include <stdint-gcc.h>

typedef uint16_t ioport_t;
typedef uint32_t phyaddr_t;
typedef unsigned int size_t;

#define PACKED __attribute__((packed))

// helper functions
static inline const char* BOOL2SIGN(int b) {
	if (b) {
		return "+";
	} else {
		return "-";
	}
}

#endif
