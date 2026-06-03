#ifndef CUP_STALKER_STRING_UTILS_H
#define CUP_STALKER_STRING_UTILS_H

#include <stddef.h>

void string_copy(char *destination, size_t destination_size, const char *source);
size_t string_display_width(const char *text);
int string_equals(const char *text, const char *other);

#endif
