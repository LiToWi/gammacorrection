#include <stdbool.h>

#include "Implementations/sisd_implementations.h"
#include "Implementations/simd_implementations.h"

// image handling
struct Image {
    size_t width;
    size_t height;
    uint8_t *pixels;
};

// implementations values
const float RED_FACTOR = 0.2126f, GREEN_FACTOR = 0.7152f, BLUE_FACTOR = 0.0722f;

// CLI defaults
const long V_DEFAULT = 0, B_DEFAULT = 10;
const float G_DEFAULT = 2.2f;
char *O_DEFAULT = "./out.ppm";

struct Test_result {
    size_t failed;
    size_t total;
};

// specifies all available implementations
struct Implementation {
    const char *name;
    void (*func)(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result);
    bool is_simd;
};

struct Implementation VERSIONS[] = {
    {"SIMD + lookup table", gamma_corr_simd_lookup_table, true},
    {"SISD + lookup table", gamma_corr_sisd_lookup_table, false},
    {"SIMD", gamma_corr_simd, true},
    {"SISD", gamma_corr, false},
};
const long AMOUNT_OF_VERSIONS = (long)(sizeof VERSIONS / sizeof VERSIONS[0]);
