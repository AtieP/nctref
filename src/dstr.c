#include"dstr.h"

#include<stdarg.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

static int ilen(int i) {
	if(i == 0) return 1;
	
	int ret = 0;
	if(i < 0) {
		ret = 1;
		i = -i;
	}
	while(i > 0) {
		ret++;
		i = i / 10;
	}
	return ret;
}

static char *myitoa(int src) {
	static char ret[12];
	snprintf(ret, 12, "%i", src);
	return ret;
}

typedef struct {
	size_t length;
	char data[];
} dstrInternal;

dstr dstrempty() {
	dstrInternal *i = malloc(sizeof(dstrInternal) + 0);
	i->length = 0;
	return (dstr) i + sizeof(dstrInternal);
}

dstr dstrz(const char *src) {
	size_t len = strlen(src);
	dstrInternal *i = malloc(sizeof(dstrInternal) + len + 1);
	i->length = len;
	memcpy(i->data, src, len + 1);
	return (dstr) i + sizeof(dstrInternal);
}

dstr dstrfmt(dstr original, const char *fmt, ...) {
	dstrInternal *originalInternal = (dstrInternal*) (original - sizeof(dstrInternal));
	
	const char *start = fmt;
	
	va_list list;
	va_start(list, fmt);
	
	size_t totalLength = 0;
	
	while(*fmt) {
		if(*fmt == '%') {
			switch(*++fmt) {
			case 's':
				totalLength += strlen(va_arg(list, char*));
				break;
			case 'S': {
				dstrInternal *i = (dstrInternal*) (va_arg(list, dstr) - sizeof(dstrInternal));
				totalLength += i->length;
				break;
			}
			case 'i':
				totalLength += ilen(va_arg(list, int));
				break;
			default: {
				totalLength++;
			}
			}
		} else totalLength++;
		
		fmt++;
	}
	
	va_end(list);
	
	fmt = start;
	
	originalInternal = realloc(originalInternal, sizeof(dstrInternal) + originalInternal->length + totalLength + 1);
	
	va_start(list, fmt);
	
	char *dst = originalInternal->data + originalInternal->length;
	originalInternal->length += totalLength;
	originalInternal->data[originalInternal->length] = 0;
	
	while(*fmt) {
		if(*fmt == '%') {
			switch(*++fmt) {
			case 's': {
				char *asdfasdf = va_arg(list, char*);
				strcpy(dst, asdfasdf);
				dst += strlen(asdfasdf);
				break;
			}
			case 'S': {
				dstrInternal *i = (dstrInternal*) (va_arg(list, dstr) - sizeof(dstrInternal));
				memcpy(dst, i->data, i->length);
				dst += i->length;
				break;
			}
			case 'i': {
				const char *asdf = myitoa(va_arg(list, int));
				strcpy(dst, asdf);
				dst += strlen(asdf);
				break;
			}
			default: {
				*(dst++) = *fmt;
			}
			}
		} else {
			*(dst++) = *fmt;
		}
		fmt++;
	}
	va_end(list);
	
	return (dstr) originalInternal + sizeof(dstrInternal);
}

void dstrfree(dstr s) {
	free(s - sizeof(dstrInternal));
}
