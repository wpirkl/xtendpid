

#pragma once

#include <stdint.h>
#include <stddef.h>

uint16_t crc16(uint16_t crc, uint8_t * data, size_t len);

uint16_t crc16_calc(uint16_t crc, uint8_t data);

// eof

