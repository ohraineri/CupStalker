#ifndef CUP_STALKER_MEMORY_H
#define CUP_STALKER_MEMORY_H

#include <stddef.h>

void *memory_alloc(size_t size);
void *memory_realloc(void *ptr, size_t size);
char *memory_strdup(const char *text);
void memory_free(void **ptr);

#endif
