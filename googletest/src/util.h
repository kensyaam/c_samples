
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>

/* Wrappers for custom memory functions */
void *malloc_wrapped(size_t size);
void *realloc_wrapped(void *p, size_t size);
void free_wrapped(void *p);

#endif