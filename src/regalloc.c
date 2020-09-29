#include"regalloc.h"

#include<stdlib.h>
#include<string.h>

int ralloc_alloc(RegisterAllocator *this, void *userdata, uint8_t size) {
	uint8_t i;
	for(i = 0; i < this->registersCount; i++) {
		if(this->registers[i].state == REGISTER_FREE && size == this->registers[i].size) {
			goto found;
		}
	}
	
	return -1;
found:;
	Register *r = &this->registers[i];
	r->state = REGISTER_ALLOCATED;
	r->userdata = userdata;
	
	for(size_t b = 0; b < (sizeof(r->aliasBitmap) * 8) && b < this->registersCount; b++) {
		if(b != i && (r->aliasBitmap & (1 << b))) {
			if(this->registers[b].state != REGISTER_DOESNT_EXIST) {
				this->registers[b].state = REGISTER_ALIASED_ALLOCATED;
				this->registers[b].userdata = userdata;
			}
		}
	}
	
	return i;
}

void ralloc_free(RegisterAllocator *this, int i) {
	Register *r = &this->registers[i];
	r->state = REGISTER_FREE;
	r->userdata = NULL;
	
	for(int b = 0; b < (int) sizeof(r->aliasBitmap) * 8 && b < this->registersCount; b++) {
		if(b != i && (r->aliasBitmap & (1 << b))) {
			if(this->registers[b].state != REGISTER_DOESNT_EXIST) {
				this->registers[b].state = REGISTER_FREE;
				this->registers[b].userdata = NULL;
			}
		}
	}
}

int ralloc_findname(RegisterAllocator *this, const char *name) {
	for(int i = 0; i < this->registersCount; i++) {
		if(!strcmp(this->registers[i].name, name)) return i;
	}
	return -1;
}