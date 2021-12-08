#ifndef GB_TYPES_H
#define GB_TYPES_H

#include <cstdint>

typedef std::uint8_t byte_t;
typedef std::uint16_t addr_t;

int8_t sign(uint8_t x);
uint8_t unsign(int8_t x);
uint16_t unsign16(int16_t x);

#endif
