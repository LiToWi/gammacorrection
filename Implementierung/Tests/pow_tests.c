#include "../common.h"
#include <stdio.h>

struct pow_test_diff {
    float expected;
    float diff;
    bool passed;
};

struct Test_input {
    float base;
    float exponent;
    float expected;
};
static const struct Test_input TEST_INPUTS[] = {{0.0f, 1.0f, 0.0f},
                                                {0.0f, 0.5412789f, 0.0f},
                                                {1.0f, 1.0f, 1.0f},
                                                {1.0f, 0.5412789f, 1.0f},
                                                {0.1f, 5.0f, 0.00001f},
                                                {0.1f, 0.1f, 0.7943282347f},
                                                {0.5f, 5.0f, 0.03125f},
                                                {0.999f, 5.0f, 0.995009990004999f},
                                                {0.12345f, 3.0f, 0.001881365963625f},
                                                {0.12345f, 0.6789f, 0.241664748f},
                                                {0.6789f, 0.12345f, 0.9533150f},
                                                {0.54321f, 3.0f, 0.160288833718161f},
                                                {0.54321f, 0.54321f, 0.717847f},
                                                {0.987654321f, 0.123456789f, 0.9984675310f},
                                                {0.123456789f, 0.987654321f, 0.126686647f},
                                                {0.01f, 0.9876f, 0.0105877f},
                                                {0.3f, 0.3f, 0.6968453019f}};
static const size_t NUMBER_OF_TESTS = sizeof(TEST_INPUTS) / sizeof(TEST_INPUTS[0]);

// constraints for pow_float: base in [0,1], exponent > 0
struct Test_result pow_tests(float (*own_pow)(float base, float exponent),
                             struct pow_test_diff (*check_diff)(float result, float expected), const char *name) {
    size_t failed_tests = 0;
    float result;
    struct pow_test_diff diff;

    for (size_t i = 0; i < NUMBER_OF_TESTS; i++) {
        result = own_pow(TEST_INPUTS[i].base, TEST_INPUTS[i].exponent);

        // check difference
        diff = check_diff(result, TEST_INPUTS[i].expected);
        if (diff.passed) {
            continue;
        }

        // print error message
        if (failed_tests == 0) {
            printf("Pow tests for '%s' failed on:\n", name);
        }
        printf("- base: %f, exponent: %f; got: %f, expected: %f, difference: %f\n", TEST_INPUTS[i].base,
               TEST_INPUTS[i].exponent, result, diff.expected, diff.diff);

        failed_tests++;
    }

    if (failed_tests) {
        printf("%zu/%zu pow test%s FAILED\n\n", failed_tests, NUMBER_OF_TESTS, failed_tests == 1 ? "" : "s");

    } else {
        printf("All pow tests PASSED for '%s'\n", name);
    }

    struct Test_result test_result = {failed_tests, NUMBER_OF_TESTS};
    return test_result;
}