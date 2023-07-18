// DUPLICATIONS IN THIS FILE ARE A DESIGN DECISION: EACH IMPLEMENTATION SHALL BE WORKING STANDALONE AND FOR PERFORMANCE
// REASONS NOT CALL COMMON FUNCTIONALITY (EXCEPT FOR POW)

#include "../common.h"
#include "pow.h"

static void gamma_one(const uint8_t *img, size_t width, size_t height, uint8_t *result) {
    size_t length = width * height * 3;
    float channel;

    for (size_t i = 0; i < length; i += 3, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // write back pixel (floor channel values)
        *result = (uint8_t)channel;
    }
}

void gamma_corr(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result) {
    // direct compare without delta is fine because one is accurately represented by floats and there hasn't been a
    // calculation before, which could create rounding errors and only the one is relevant for the comparison
    if (gamma == 1.f) {
        gamma_one(img, width, height, result);
        return;
    }

    size_t length = width * height * 3;
    float channel;
    float (*pow)(float, float) = init_pow(gamma);

    for (size_t i = 0; i < length; i += 3, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // gamma correction
        channel = pow(channel / 255.f, gamma) * 255.f;

        // write back pixel (floor channel values)
        *result = (uint8_t)channel;
    }
}

void gamma_corr_sisd_lookup_table(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result) {
    // direct compare without delta is fine because one is accurately represented by floats and there hasn't been a
    // calculation before, which could create rounding errors and only the one is relevant for the comparison
    if (gamma == 1.f) {
        gamma_one(img, width, height, result);
        return;
    }

    size_t length = width * height * 3;
    float channel;

    init_lookup_table(gamma);

    for (size_t i = 0; i < length; i += 3, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // gamma correction and write back
        *result = pow_lookup_table(channel / 255.f);
    }
}
