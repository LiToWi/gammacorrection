#include "../common.h"
#include <math.h>

// converting faster
__attribute__((const)) static double own_ln(double x) {
    double result = (x - 1) / (x + 1), sqr = result * result, factor = result * sqr, summand;

    for (int i = 3; i < 2000; i += 2) {
        summand = factor / i;
        // break if there are only little changes
        if (fabs(summand) < 0.000001) {
            break;
        }
        result += summand;
        factor *= sqr;
    }

    return 2. * result;
}

__attribute__((const)) static double own_exp(double x) {
    // own_ln(0,0000001)
    if (x < -16) {
        return 0.;
    }

    double result = 1., summand = 1.;

    for (int i = 1; i < 1000; i++) {
        summand *= x / i;
        // break if there are only little changes
        if (fabs(summand) < 0.000001) {
            break;
        }
        result += summand;
    }

    return result < 0 ? -result : result;
}

// base^exponent = exp(exponent * own_ln(base))
__attribute__((const)) float pow_float(float base, float exponent) {
    return base < 0.000001f ? 0.f : (float)own_exp(exponent * own_ln(base));
}

__attribute__((const)) float pow_integer(float base, float exponent) {
    int exp = (int)exponent;
    // gamma == 0 not allowed -> no need to return 1
    float res = base;
    // optimization with `i += i` not needed, because values are mostly very small
    for (int i = 1; i < exp; i++) {
        res *= base;
    }

    return res;
}

__attribute__((const)) float (*init_pow(float gamma))(float, float) {
    // direct compare without delta is fine because the max value of gamma is in range of max safe integer of float and
    // there hasn't been a calculation before, which could create rounding errors and only integer values are relevant
    // for the comparison
    return (float)(int)gamma == gamma ? pow_integer : pow_float;
}

float cache[255];
// init a cache
void init_lookup_table(float gamma) {
    float exponent = 1.f / gamma;
    for (unsigned char i = 1; i; i++) {
        cache[i - 1] = pow_float((float)i / 255.f, exponent);
    }
}

// get value from cache
uint8_t pow_lookup_table(float base) {
    if (base <= cache[0]) {
        return 0;
    }

    uint8_t left = 1, right = 254, mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (base < cache[mid]) {
            right = mid - 1;

        } else {
            left = mid + 1;
        }
    }

    return left;
}
