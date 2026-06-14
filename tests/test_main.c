#include "test_util.h"

#include <stdio.h>

int g_tests_run = 0;
int g_tests_failed = 0;

int main(void)
{
    tests_match();
    tests_parser();
    tests_display();

    printf("\n%d checks, %d failed\n", g_tests_run, g_tests_failed);
    return (g_tests_failed == 0) ? 0 : 1;
}
