#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Image {
    size_t width;
    size_t height;
    uint8_t *pixels;
};

extern const float RED_FACTOR, GREEN_FACTOR, BLUE_FACTOR;

extern const long V_DEFAULT, B_DEFAULT;
extern const float G_DEFAULT;
extern char *O_DEFAULT;

struct Test_result {
    size_t failed;
    size_t total;
};

struct Implementation {
    const char *name;
    void (*func)(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result);
    bool is_simd;
};
extern struct Implementation VERSIONS[];
extern long AMOUNT_OF_VERSIONS;
