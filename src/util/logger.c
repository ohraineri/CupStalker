#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void logger_emit(const char *level, const char *format, va_list args)
{
    fprintf(stderr, "[cup-stalker][%s] ", level);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

void logger_debug(const char *format, ...)
{
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    logger_emit("debug", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

void logger_error(const char *format, ...)
{
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    logger_emit("error", format, args);
    va_end(args);
#else
    (void)format;
#endif
}

void logger_fatal(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logger_emit("fatal", format, args);
    va_end(args);

    abort();
}
