#ifndef CUP_STALKER_TEST_UTIL_H
#define CUP_STALKER_TEST_UTIL_H

#include <stdio.h>

extern int g_tests_run;
extern int g_tests_failed;

#define CHECK(condition, description)                                      \
    do {                                                                   \
        g_tests_run += 1;                                                  \
        if (!(condition)) {                                                \
            g_tests_failed += 1;                                           \
            fprintf(stderr, "FAIL: %s (%s:%d)\n",                          \
                    (description), __FILE__, __LINE__);                    \
        }                                                                  \
    } while (0)

void tests_match(void);
void tests_parser(void);
void tests_display(void);

#endif
