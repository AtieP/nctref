#include"reporting.h"

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>

/* Abort immediately on first error (for now) */
void stahp(const char *error, ...) {
	va_list l;
	va_start(l, error);
	
	fputs("error ??:??: ", stderr);
	vfprintf(stderr, error, l);
	fputc('\n', stderr);
	
	va_end(l);
	
	exit(1);
}