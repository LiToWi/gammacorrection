#include <stddef.h>
#include <stdint.h>

void gamma_corr(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result);
void gamma_corr_sisd_lookup_table(const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result);