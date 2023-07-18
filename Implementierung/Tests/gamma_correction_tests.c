#include "../common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static const uint8_t IMG_1[] = {10, 50,  200, 20,  70,  100, 30, 90, 150, 5, 200, 9,   46, 97,
                                48, 238, 48,  167, 139, 69,  19, 0,  139, 0, 238, 230, 133};
// case all channels 0
static const uint8_t IMG_2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// case all channels 255
static const uint8_t IMG_3[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
// 5 pixels
static const uint8_t IMG_4[] = {46, 97, 48, 41, 100, 52, 42, 112, 53, 47, 115, 56, 48, 109, 50};
// 12 pixels
static const uint8_t IMG_5[] = {10,  50, 200, 20, 70,  100, 30,  90,  150, 5,  200, 9,  46, 97,  48, 238, 48,  167,
                                139, 69, 19,  0,  139, 0,   238, 230, 133, 46, 97,  48, 41, 100, 52, 42,  112, 53};
// edge case base values
static const uint8_t IMG_6[] = {0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1};
static const uint8_t IMG_7[] = {254, 255, 255, 255, 254, 255, 255, 255, 254, 254, 255, 254};

static const uint8_t IMG_1_EXP_FOR_GAMMA_0_5[] = {115, 125, 144, 192, 145, 157, 143, 159, 239};
static const uint8_t IMG_1_EXP_FOR_GAMMA_1_5[] = {23, 30, 46, 109, 47, 59, 45, 62, 210};
static const uint8_t IMG_1_EXP_FOR_GAMMA_2_0[] = {10, 14, 26, 82, 26, 36, 25, 38, 197};
static const uint8_t IMG_1_EXP_FOR_GAMMA_1_0[] = {52, 61, 81, 144, 82, 96, 80, 99, 224};
static const uint8_t IMG_1_EXP_FOR_GAMMA_0_0001_AND_0_000001[] = {254, 254, 254, 254, 254, 254, 254, 254, 254};
static const uint8_t IMG_1_EXP_FOR_GAMMA_0_1234567[] = {209, 213, 221, 237, 221, 226, 221, 227, 251};
static const uint8_t IMG_1_EXP_FOR_GAMMA_3_333[] = {1, 2, 5, 38, 5, 10, 5, 11, 167};
static const uint8_t IMG_1_EXP_FOR_GAMMA_4_789[] = {0, 0, 1, 16, 1, 2, 1, 2, 139};
static const uint8_t IMG_2_EXP_FOR_GAMMA_0_5_AND_1_5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t IMG_3_EXP_FOR_GAMMA_0_5_AND_1_5[] = {255, 255, 255, 255, 255, 255, 255, 255, 255};
static const uint8_t IMG_4_EXP_FOR_GAMMA_0_375[] = {167, 168, 174, 176, 173};
static const uint8_t IMG_5_EXP_FOR_GAMMA_0_5[] = {115, 125, 144, 192, 145, 157, 143, 159, 239, 145, 146, 153};
static const uint8_t IMG_6_EXP_FOR_GAMMA_0_5[] = {4, 13, 7, 8};
static const uint8_t IMG_6_EXP_FOR_GAMMA_2_33[] = {0, 0, 0, 0};
static const uint8_t IMG_7_EXP_FOR_GAMMA_0_5[] = {254, 254, 254, 254};
static const uint8_t IMG_7_EXP_FOR_GAMMA_2_33[] = {254, 253, 254, 254};

struct Test_input {
    const char *additional_information;
    const uint8_t *img;
    const uint8_t *expected;
    size_t height;
    size_t width;
    float gamma;
};

static const struct Test_input TEST_INPUTS[] = {
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_0_5, 3, 3, .5f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_1_5, 3, 3, 1.5f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_2_0, 3, 3, 2.f},
    {" with gamma 1", IMG_1, IMG_1_EXP_FOR_GAMMA_1_0, 3, 3, 1.f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_0_0001_AND_0_000001, 3, 3, .0001f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_0_0001_AND_0_000001, 3, 3, .000001f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_0_1234567, 3, 3, .1234567f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_3_333, 3, 3, 3.333f},
    {"", IMG_1, IMG_1_EXP_FOR_GAMMA_4_789, 3, 3, 4.789f},
    {" with all channels 0", IMG_2, IMG_2_EXP_FOR_GAMMA_0_5_AND_1_5, 3, 3, .5f},
    {" with all channels 0", IMG_2, IMG_2_EXP_FOR_GAMMA_0_5_AND_1_5, 2, 4, 1.5f},
    {" with all channels 255", IMG_3, IMG_3_EXP_FOR_GAMMA_0_5_AND_1_5, 3, 3, .5f},
    {" with all channels 255", IMG_3, IMG_3_EXP_FOR_GAMMA_0_5_AND_1_5, 2, 4, 1.5f},
    {"", IMG_4, IMG_4_EXP_FOR_GAMMA_0_375, 5, 1, .375f},
    {"", IMG_5, IMG_5_EXP_FOR_GAMMA_0_5, 3, 4, .5f},
    {" with low channel sum", IMG_6, IMG_6_EXP_FOR_GAMMA_0_5, 2, 2, .5f},
    {" with low channel sum", IMG_6, IMG_6_EXP_FOR_GAMMA_2_33, 2, 2, 2.33f},
    {" with high channel sum", IMG_7, IMG_7_EXP_FOR_GAMMA_0_5, 2, 2, .5f},
    {" with high channel sum", IMG_7, IMG_7_EXP_FOR_GAMMA_2_33, 2, 2, 2.33f}};
static const size_t NUMBER_OF_TESTS = sizeof(TEST_INPUTS) / sizeof(TEST_INPUTS[0]);

static const uint8_t *add_padding(const uint8_t *array, size_t length) {
    length *= 4;
    // adding one prevents valgrind to show error (will be added to not miss any other valgrind errors and the adding
    // doesn't cause any problems for the tests)
    uint8_t *new_array = malloc(length + 1);
    if (new_array == NULL) {
        printf("Failed: run out of memory.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < length; i++, array++) {
        // add padding byte on every fourth entry
        if (i % 4 == 3) {
            i++;
        }
        new_array[i] = *array;
    }

    return new_array;
}

// to store the result of the array comparison
struct Result {
    bool passed;
    size_t index;
    uint8_t actual;
    uint8_t expected;
};

// compares two arrays for equality
// if equal returns -1 else returns the first index where the arrays differ, as well as the differing values
static struct Result arrays_equal(const uint8_t *actual, const uint8_t *expected, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (actual[i] != expected[i]) {
            // arrays are not equal, current index is written in the struct
            struct Result result = {false, i, actual[i], expected[i]};
            return result;
        }
    }

    // arrays are equal, return true
    struct Result result = {true, 0, 0, 0};
    return result;
}

struct Test_result gamma_corr_tests(struct Implementation implementation) {
    unsigned failed_tests = 0;

    for (size_t i = 0; i < NUMBER_OF_TESTS; i++) {
        struct Test_input test_input = TEST_INPUTS[i];
        size_t length = test_input.height * test_input.width;

        uint8_t *result = malloc(length * 3);
        if (result == NULL) {
            printf("Error: program run out of memory!");
        }

        // execute implementation and check
        // !! img has to be freed on simd
        const uint8_t *img = implementation.is_simd ? add_padding(test_input.img, length) : test_input.img;
        implementation.func(img, test_input.width, test_input.height, test_input.gamma, result);
        struct Result check = arrays_equal(result, test_input.expected, length);

        // free resources
        free(result);
        if (implementation.is_simd) {
            // the cast removes the const identifier. This is needed, because malloc only accepts non const pointers.
            free((void *)img);
        }

        if (check.passed) {
            continue;
        }

        // print error message
        if (failed_tests == 0) {
            printf("\nGamma correction tests for '%s' failed on:\n", implementation.name);
        }
        printf("- test %zu%s: Difference at index %zu, got: %hhu, expected: %hhu\n", i + 1,
               test_input.additional_information, check.index, check.actual, check.expected);

        failed_tests++;
    }

    if (failed_tests) {
        printf("%u/%zu gamma correction test%s FAILED\n\n", failed_tests, NUMBER_OF_TESTS,
               failed_tests == 1 ? "" : "s");

    } else {
        printf("All gamma correction tests PASSED for '%s'\n", implementation.name);
    }

    struct Test_result test_result = {failed_tests, NUMBER_OF_TESTS};
    return test_result;
}