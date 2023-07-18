struct pow_test_diff {
    float expected;
    float diff;
    bool passed;
};

struct Test_result pow_tests(float (*own_pow)(float base, float exponent),
                             struct pow_test_diff (*check_diff)(float result, float expected), const char *name);