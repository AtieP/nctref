#ifndef NCTREF_TYPES_H
#define NCTREF_TYPES_H

#include<stddef.h>

typedef enum {
	TYPE_TYPE_PRIMITIVE, TYPE_TYPE_COMPOUND
} TypeType;

typedef struct TypePrimitive {
	TypeType type;
	
	const char *src;
	
	int width;
	int base;
	
	int isFloat;
	int isUnsigned;
	int isNative;
	int isMinimum;
	
	int vector; /* 1 for no vector. */
	
	struct TypePrimitive *next;
} TypePrimitive;

typedef union {
	TypeType type;
	
	TypePrimitive primitive;
} Type;

TypePrimitive *primitive_parse(const char*);

size_t type_size(Type*);

#endif
