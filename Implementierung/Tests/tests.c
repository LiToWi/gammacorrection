#include <math.h>
#include <stdio.h>

#include "../Implementations/sisd_implementations.h"
#include "../Implementations/pow.h"
#include "../Implementations/simd_implementations.h"
#include "gamma_correction_tests.h"
#include "pow_tests.h"

static float test_pow_lookup_table(float base, float exponent) {
    init_lookup_table(exponent);
    return pow_lookup_table(base);
}

struct pow_test_diff diff_pow_lookup_table(float result, float expected) {
    uint8_t really_expected = (uint8_t)(expected * 255.f);
    uint8_t difference = really_expected - (uint8_t)result;
    struct pow_test_diff diff = {really_expected, difference, !difference};
    return diff;
}

static float test_pow(float base, float exponent) {
    return init_pow(exponent)(base, exponent);
}

struct pow_test_diff diff_pow(float result, float expected) {
    float difference = fabsf(result - expected);
    struct pow_test_diff diff = {expected, difference, difference < 0.000001f};
    return diff;
}

static void update_combined(struct Test_result *combined, struct Test_result current) {
    combined->failed += current.failed;
    combined->total += current.total;
}

void test() {
    struct Test_result combined = {0, 0};
    update_combined(&combined, pow_tests(test_pow, diff_pow, "pow_float"));
    update_combined(&combined, pow_tests(test_pow_lookup_table, diff_pow_lookup_table, "pow_lookup_table"));

    for (long i = 0; i < AMOUNT_OF_VERSIONS; i++) {
        update_combined(&combined, gamma_corr_tests(VERSIONS[i]));
    }

    if (combined.failed) {
        printf("\n%zu/%zu test%s FAILED\n", combined.failed, combined.total, combined.failed == 1 ? "" : "s");

    } else {
        printf("\nAll tests PASSED\n");
    }
}