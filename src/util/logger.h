#ifndef CUP_STALKER_LOGGER_H
#define CUP_STALKER_LOGGER_H

#if defined(__GNUC__)
#define LOGGER_PRINTF_LIKE(fmt_index, args_index) \
    __attribute__((format(printf, fmt_index, args_index)))
#else
#define LOGGER_PRINTF_LIKE(fmt_index, args_index)
#endif

void logger_debug(const char *format, ...) LOGGER_PRINTF_LIKE(1, 2);
void logger_error(const char *format, ...) LOGGER_PRINTF_LIKE(1, 2);
void logger_fatal(const char *format, ...) LOGGER_PRINTF_LIKE(1, 2);

#endif
