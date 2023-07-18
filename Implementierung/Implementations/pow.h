#include <stdint.h>

float pow_float(float base, float exponent);

void init_lookup_table(float gamma);
uint8_t pow_lookup_table(float base);

float (*init_pow(float gamma))(float, float);