#ifndef _DSTR_H
#define _DSTR_H

#include<stddef.h>

/* Originally used sds, but it didn't support OpenWatcom. This isn't as optimized, but it's good enough. */

typedef char *dstr;

dstr dstrempty();
dstr dstrraw(const char*);
dstr dstrfmt(dstr, const char*, ...);
void dstrfree(dstr);

#endif
