#ifndef NCTREF_UTILS_H
#define NCTREF_UTILS_H

#include<stddef.h>

inline static size_t djb2(const char *str) {
	size_t hash = 5381;

	int c;
	while((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

#endif
