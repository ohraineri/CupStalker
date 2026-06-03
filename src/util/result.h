#ifndef CUP_STALKER_RESULT_H
#define CUP_STALKER_RESULT_H

#include <stdbool.h>

#define RESULT_MESSAGE_CAPACITY 256

typedef struct {
    bool success;
    char message[RESULT_MESSAGE_CAPACITY];
} Result;

#define RESULT_OK ((Result){ .success = true, .message = "" })

#define RESULT_ERROR(msg) result_error(msg)

Result result_error(const char *message);

#endif
