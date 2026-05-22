#include <stdio.h>
#include "test_framework.h"

int tests_passes = 0;
int tests_echoues = 0;

int main(void) {
    run_tests_histogram();
    run_tests_series();
    run_tests_tree();
    run_tests_integration();
    int total = tests_passes + tests_echoues;
    printf("\nResultat : %d/%d tests passes\n", tests_passes, total);
    return (tests_echoues == 0) ? 0 : 1;
}