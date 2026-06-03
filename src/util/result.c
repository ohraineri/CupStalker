#include "result.h"

#include <string.h>

Result result_error(const char *message)
{
    Result result = { .success = false, .message = "" };

    if (message != NULL) {
        strncpy(result.message, message, RESULT_MESSAGE_CAPACITY - 1);
        result.message[RESULT_MESSAGE_CAPACITY - 1] = '\0';
    }

    return result;
}
