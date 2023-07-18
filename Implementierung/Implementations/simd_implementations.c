// DUPLICATIONS IN THIS FILE ARE A DESIGN DECISION: EACH IMPLEMENTATION SHALL BE WORKING STANDALONE AND FOR PERFORMANCE REASONS NOT CALL COMMON FUNCTIONALITY (EXCEPT FOR POW)

#include <emmintrin.h>
#include "../common.h"
#include "pow.h"

static void gamma_one(const uint8_t *img, size_t width, size_t height, uint8_t *result) {
    size_t length = width * height, packed_length = length - length % 4;

    __m128 red_factor = { RED_FACTOR, RED_FACTOR, RED_FACTOR, RED_FACTOR };
    __m128 green_factor = { GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR };
    __m128 blue_factor = { BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR };

    __m128i extraction_mask = { 0xFF000000FF, 0xFF000000FF };

    __m128i pixels;
    __m128 red_float, green_float, blue_float, res;

    const __m128i* img_m128_pointer = (const __m128i*)(img);

    for (uint8_t *end_pointer = result + packed_length; result < end_pointer; img_m128_pointer++, result += 4) {
        // load channels as floats
        pixels = _mm_loadu_si128(img_m128_pointer);
        red_float = _mm_cvtepi32_ps(_mm_and_si128(pixels, extraction_mask));
        green_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 8), extraction_mask));
        blue_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 16), extraction_mask));

        // multiply each channel by corresponding factor
        red_float = _mm_mul_ps(red_float, red_factor);
        green_float = _mm_mul_ps(green_float, green_factor);
        blue_float = _mm_mul_ps(blue_float, blue_factor);

        // add channels
        res = _mm_add_ps(_mm_add_ps(red_float, green_float), blue_float);

        // gamma correction and write back values
        result[0] = (uint8_t)res[0];
        result[1] = (uint8_t)res[1];
        result[2] = (uint8_t)res[2];
        result[3] = (uint8_t)res[3];
    }

    // handle remaining pixels (analog non simd implementation)
    float channel;
    length = length * 4;
    for (size_t i = packed_length * 4; i < length; i += 4, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // write back pixel (floor channel values)
        *result = (uint8_t)channel;
    }
}

void gamma_corr_simd(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result) {
    // direct compare without delta is fine because one is accurately represented by floats and there hasn't been a
    // calculation before, which could create rounding errors and only the one is relevant for the comparison
    if(gamma == 1.f) {
        gamma_one(img, width, height, result);
        return;
    }

    size_t length = width * height, packed_length = length - length % 4;
    float (*pow)(float, float) = init_pow(gamma);

    __m128 red_factor = { RED_FACTOR, RED_FACTOR, RED_FACTOR, RED_FACTOR };
    __m128 green_factor = { GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR };
    __m128 blue_factor = { BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR };

    __m128i extraction_mask = { 0xFF000000FF, 0xFF000000FF };
    __m128 division_mask = { 0xFF, 0xFF, 0xFF, 0xFF };

    __m128i pixels;
    __m128 red_float, green_float, blue_float, res;

    const __m128i* img_m128_pointer = (const __m128i*)(img);

    for (uint8_t *end_pointer = result + packed_length; result < end_pointer; img_m128_pointer++, result += 4) {
        // load channels as floats
        pixels = _mm_loadu_si128(img_m128_pointer);
        red_float = _mm_cvtepi32_ps(_mm_and_si128(pixels, extraction_mask));
        green_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 8), extraction_mask));
        blue_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 16), extraction_mask));

        // multiply each channel by corresponding factor
        red_float = _mm_mul_ps(red_float, red_factor);
        green_float = _mm_mul_ps(green_float, green_factor);
        blue_float = _mm_mul_ps(blue_float, blue_factor);

        // add channels
        res = _mm_div_ps(_mm_add_ps(_mm_add_ps(red_float, green_float), blue_float), division_mask);

        // gamma correction and write back values
        result[0] = (uint8_t) (pow(res[0], gamma) * 255.f);
        result[1] = (uint8_t) (pow(res[1], gamma) * 255.f);
        result[2] = (uint8_t) (pow(res[2], gamma) * 255.f);
        result[3] = (uint8_t) (pow(res[3], gamma) * 255.f);
    }

    // handle remaining pixels (analog implementation 1)
    float channel;
    length = length * 4;
    for (size_t i = packed_length * 4; i < length; i += 4, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // gamma correction and write back
        channel = pow(channel / 255.f, gamma) * 255.f;

        // gamma correction and write back
        *result = (uint8_t) channel;
    }
}

void gamma_corr_simd_lookup_table(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result) {
    // direct compare without delta is fine because one is accurately represented by floats and there hasn't been a
    // calculation before, which could create rounding errors and only the one is relevant for the comparison
    if(gamma == 1.f	) {
        gamma_one(img, width, height, result);
        return;
    }

    size_t length = width * height, packed_length = length - length % 4;
    init_lookup_table(gamma);

    __m128 red_factor = { RED_FACTOR, RED_FACTOR, RED_FACTOR, RED_FACTOR };
    __m128 green_factor = { GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR, GREEN_FACTOR };
    __m128 blue_factor = { BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR, BLUE_FACTOR };

    __m128i extraction_mask = { 0xFF000000FF, 0xFF000000FF };
    __m128 division_mask = { 0xFF, 0xFF, 0xFF, 0xFF };

    __m128i pixels;
    __m128 red_float, green_float, blue_float, res;

    const __m128i* img_m128_pointer = (const __m128i*)(img);

    for (uint8_t *end_pointer = result + packed_length; result < end_pointer; img_m128_pointer++, result += 4) {
        // load channels as floats
        pixels = _mm_loadu_si128(img_m128_pointer);
        red_float = _mm_cvtepi32_ps(_mm_and_si128(pixels, extraction_mask));
        green_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 8), extraction_mask));
        blue_float = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(pixels, 16), extraction_mask));

        // multiply each channel by corresponding factor
        red_float = _mm_mul_ps(red_float, red_factor);
        green_float = _mm_mul_ps(green_float, green_factor);
        blue_float = _mm_mul_ps(blue_float, blue_factor);

        // add channels
        res = _mm_div_ps(_mm_add_ps(_mm_add_ps(red_float, green_float), blue_float), division_mask);

        // gamma correction and write back values
        result[0] = pow_lookup_table(res[0]);
        result[1] = pow_lookup_table(res[1]);
        result[2] = pow_lookup_table(res[2]);
        result[3] = pow_lookup_table(res[3]);
    }

    // handle remaining pixels (analog implementation 1)
    float channel;
    length = length * 4;
    for (size_t i = packed_length * 4; i < length; i += 4, result++) {
        // convert to greyscale
        channel = (RED_FACTOR * (float)img[i] + GREEN_FACTOR * (float)img[i + 1] + BLUE_FACTOR * (float)img[i + 2]);

        // gamma correction and write back
        *result = pow_lookup_table(channel / 255.f);
    }
}
