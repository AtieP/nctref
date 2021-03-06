#include"types.h"

#include"utils.h"
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include"ntc.h"

TypePrimitive *primitiveDatabase[128];

TypePrimitive *primitive_parse(const char *src) {
	size_t hash = djb2(src) % 128;
	
	for(TypePrimitive *t = primitiveDatabase[hash]; t; t = t->next) {
		if(!strcmp(src, t->src)) {
			return t;
		}
	}
	
	TypePrimitive *ret = malloc(sizeof(*ret));
	ret->type = TYPE_TYPE_PRIMITIVE;
	ret->src = src;
	
	if(*src == 'n') {
		src++;
		ret->isNative = 1;
	} else {
		ret->isNative = 0;
	}
	
	if(*src == 'u') {
		src++;
		ret->isUnsigned = 1;
	} else if(*src == 's') {
		src++;
		ret->isUnsigned = 0;
	} else {
		free(ret);
		return NULL;
	}
	
	if(*src == 'm') {
		src++;
		ret->isMinimum = 1;
	} else {
		ret->isMinimum = 0;
	}
	
	if(*src == 'f') {
		src++;
		ret->isFloat = 1;
	} else {
		ret->isFloat = 0;
	}
	
	ret->width = strtol(src, (char**) &src, 10);
	
	if(ret->width == 0) {
		free(ret);
		return NULL;
	}
	
	if(*src == 'b') {
		src++;
		ret->base = strtol(src, (char**) &src, 10);
	} else {
		ret->base = 2;
	}
	
	if(*src == 'v') {
		src++;
		ret->vector = strtol(src, (char**) &src, 10);
	} else {
		ret->vector = 1;
	}
	
	ret->next = primitiveDatabase[hash];
	primitiveDatabase[hash] = ret;
	
	return ret;
}

size_t type_size(Type *t) {
	if(t->type == TYPE_TYPE_PRIMITIVE) {
		/* Round to nearest highest power of two. */
		uint16_t w = (t->primitive.width + 7) / 8;
		w--;
		w |= w >> 1;
		w |= w >> 2;
		w |= w >> 4;
		w |= w >> 8;
		w++;
		
		return w;
	} else if(t->type == TYPE_TYPE_POINTER) {
		switch(Xinst.mode) {
			case X86_MODE_16: return 2;
			case X86_MODE_32: return 4;
			case X86_MODE_64: return 8;
#ifdef DEBUG
			default: abort();
#endif
		}
	} else if(t->type == TYPE_TYPE_FUNCTION) {
		return 1; /* TODO: improve. */
	}
	
	abort();
	return -1;
}

int type_equal(Type *O, Type *T) {
	if(O == T) return 1;
	if(O->type != T->type) return 0;
	
	if(O->type == TYPE_TYPE_PRIMITIVE) {
		TypePrimitive *o = &O->primitive, *t = &T->primitive;
		return o->width == t->width \
		    && o->base == t->base \
			&& o->isFloat == t->isFloat \
			&& o->isUnsigned == t->isUnsigned \
			&& o->isNative == t->isNative \
			&& o->isMinimum == t->isMinimum \
			&& o->vector == t->vector;
	} else if(O->type == TYPE_TYPE_POINTER) {
		return type_equal(O->pointer.of, T->pointer.of);
	}
	
	/* Don't deep-compare compound types; Nectar has nominal typing. */
	
	return 0;
}

/* TODO: cache */
Type *type_pointer_wrap(Type *t) {
	TypePointer *ret = malloc(sizeof(*ret));
	ret->type = TYPE_TYPE_POINTER;
	ret->of = t;
	return (Type*) ret;
}
