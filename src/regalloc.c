#include"regalloc.h"

int ralloc_alloc(RegisterAllocator *this, uint8_t size) {
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
	
	for(int b = 0; b < sizeof(r->aliasBitmap) * 8; b++) {
		if(b != i && (r->aliasBitmap & (1 << b))) {
			this->registers[b].state = REGISTER_ALIASED_ALLOCATED;
		}
	}
	
	return i;
}

void ralloc_free(RegisterAllocator *this, int i) {
	Register *r = &this->registers[i];
	r->state = REGISTER_FREE;
	
	for(int b = 0; b < sizeof(r->aliasBitmap) * 8; b++) {
		if(b != i && (r->aliasBitmap & (1 << b))) {
			this->registers[b].state = REGISTER_FREE;
		}
	}
}