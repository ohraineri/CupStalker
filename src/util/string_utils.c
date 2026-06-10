#include "string_utils.h"

#include <string.h>

void string_copy(char *destination, size_t destination_size, const char *source)
{
    if (destination == NULL || destination_size == 0) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

size_t string_display_width(const char *text)
{
    if (text == NULL) {
        return 0;
    }

    size_t width = 0;
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor; ++cursor) {
        if (cursor[0] == 0x1B && cursor[1] == '[') {
            cursor += 2;
            while (*cursor && (*cursor < 0x40 || *cursor > 0x7E)) {
                ++cursor;
            }
            if (*cursor == '\0') {
                break;
            }
            continue;
        }

        if ((*cursor & 0xC0) != 0x80) {
            ++width;
        }
    }

    return width;
}

int string_equals(const char *text, const char *other)
{
    if (text == NULL || other == NULL) {
        return text == other;
    }

    return strcmp(text, other) == 0;
}
