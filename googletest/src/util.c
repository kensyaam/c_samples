#include <stdlib.h>

void *malloc_wrapped(size_t size) {
    return malloc(size);
}

void *realloc_wrapped(void *p, size_t size) {
    return realloc(p, size);
}

void free_wrapped(void *p) {
    free(p);
}

