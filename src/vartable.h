#ifndef NCTREF_VARTABLE_H
#define NCTREF_VARTABLE_H

#include"types.h"

typedef enum {
	VARTABLEENTRY_SYMBOL, VARTABLEENTRY_TYPE
} VarTableEntryKind;

typedef struct VarTableEntry {
	struct VarTableEntry *next;
	
	const char *name;
	
	Type *type;
	
	VarTableEntryKind kind;
	union {
		struct {
			const char *linkName;
		} symbol;
	} data;
} VarTableEntry;

typedef struct VarTable {
	struct VarTable *parent;
	
	VarTableEntry *buckets[128];
} VarTable;

VarTable *vartable_new(VarTable*);
VarTableEntry *vartable_get(VarTable*, const char*);
VarTableEntry *vartable_find(VarTable*, const char*);
VarTableEntry *vartable_set(VarTable*, VarTableEntry*);

#endif
