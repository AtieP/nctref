#include"vartable.h"

#include"utils.h"
#include<stdlib.h>
#include<string.h>

VarTable *vartable_new(VarTable *parent) {
	VarTable *ret = malloc(sizeof(*ret));
	ret->parent = parent;
	memset(ret->buckets, 0, sizeof(ret->buckets));
	
	return ret;
}

VarTableEntry *vartable_get(VarTable *this, const char *name) {
	size_t hash = djb2(name) % VARTABLE_BUCKETS;
	
	VarTableEntry *entry = this->buckets[hash];
	while(entry && strcmp(name, entry->name)) {
		entry = entry->next;
	}
	
	return entry;
}

VarTableEntry *vartable_find(VarTable *this, const char *name) {
	VarTable *tbl = this;
	while(tbl) {
		VarTableEntry *entry = vartable_get(tbl, name);
		if(entry) {
			return entry;
		}
		tbl = tbl->parent;
	}
	return NULL;
}

VarTableEntry *vartable_set(VarTable *this, VarTableEntry *e) {
	size_t hash = djb2(e->name) % VARTABLE_BUCKETS;
	
	e->next = this->buckets[hash];
	this->buckets[hash] = e;
	
	return e;
}
