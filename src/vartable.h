#ifndef NCTREF_VARTABLE_H
#define NCTREF_VARTABLE_H

#include"types.h"

#define VARTABLE_BUCKETS 32

typedef enum {
	VARTABLEENTRY_SYMBOL, VARTABLEENTRY_TYPE, VARTABLEENTRY_VAR
} VarTableEntryKind;

typedef struct VarTableEntry {
	struct VarTableEntry *next;
	
	const char *name;
	
	Type *type;
	
	VarTableEntryKind kind;
	union {
		struct {
			const char *linkName;
			int isLocal;
			int isExternal;
		} symbol;
	} data;
	
	int isDereferenced; /* Used by optimizations to determine whether to put in dereferencable register */
	
	void *userdata;
} VarTableEntry;

typedef struct VarTable {
	struct VarTable *parent;
	
	VarTableEntry *buckets[VARTABLE_BUCKETS];
} VarTable;

VarTable *vartable_new(VarTable*);
VarTableEntry *vartable_get(VarTable*, const char*);
VarTableEntry *vartable_find(VarTable*, const char*);
VarTableEntry *vartable_set(VarTable*, VarTableEntry*);

#endif
