#include "memory.h"

#include "logger.h"

#include <stdlib.h>
#include <string.h>

void *memory_alloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    void *block = calloc(1, size);
    if (block == NULL) {
        logger_fatal("memory_alloc: out of memory requesting %zu bytes", size);
    }

    return block;
}

void *memory_realloc(void *ptr, size_t size)
{
    void *block = realloc(ptr, size);
    if (block == NULL && size != 0) {
        logger_fatal("memory_realloc: out of memory requesting %zu bytes", size);
    }

    return block;
}

char *memory_strdup(const char *text)
{
    if (text == NULL) {
        return NULL;
    }

    size_t length = strlen(text);
    char *copy = memory_alloc(length + 1);
    memcpy(copy, text, length + 1);

    return copy;
}

void memory_free(void **ptr)
{
    if (ptr == NULL || *ptr == NULL) {
        return;
    }

    free(*ptr);
    *ptr = NULL;
}
