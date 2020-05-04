

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "pixtend_2s.h"

#define LIMIT_PAR(value, min, max) \
    if(*value < min || *value > max) { *value = 0; } else {}

uint16_t crc16(uint16_t crc, uint8_t * data, size_t len);

uint16_t crc16_calc(uint16_t crc, uint8_t data);


union pixtOut {
    struct pixtOutV2S v2s;
};

union pixtIn {
    struct pixtInV2S v2s;
};

// eof

