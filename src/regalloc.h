#ifndef NCTREF_REGALLOC_H
#define NCTREF_REGALLOC_H

#include<stdint.h>

/* This would've been *much* prettier in C++, any ideas on how to make this less ugly while keeping it general-purpose? */

typedef enum {
	REGISTER_FREE, REGISTER_ALLOCATED, REGISTER_ALIASED_ALLOCATED, REGISTER_DOESNT_EXIST
} RegisterState;

typedef struct {
	char name[8];
	RegisterState state;
	uint8_t size;
	uint64_t aliasBitmap;
	
	void *userdata;
} Register;

typedef struct {
	uint8_t registersCount;
	Register registers[];
} RegisterAllocator;

int ralloc_alloc(RegisterAllocator*, void*, uint8_t size);
void ralloc_free(RegisterAllocator*, int);
int ralloc_findname(RegisterAllocator*, const char*);

#endif
