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
	uint32_t aliasBitmap;
	uint8_t dereferencable;
	
	void *userdata;
} Register;

typedef struct {
	uint8_t registersCount;
	Register registers[];
} RegisterAllocator;

int ralloc_alloc(RegisterAllocator*, uint8_t size, int dereferencable);
void ralloc_free(RegisterAllocator*, int);
int ralloc_findname(RegisterAllocator*, const char*);
void ralloc_setuserdata(RegisterAllocator*, int, void*);

#endif
