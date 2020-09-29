#include"reporting.h"

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>

/* Abort immediately on first error (for now) */
void stahp(int row, int column, const char *error, ...) {
	va_list l;
	va_start(l, error);
	
	fprintf(stderr, "error %i:%i: ", row, column);
	vfprintf(stderr, error, l);
	fputc('\n', stderr);
	
	va_end(l);
	
	exit(1);
}