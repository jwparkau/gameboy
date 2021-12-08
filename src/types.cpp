#include "types.h"

int8_t sign(uint8_t x)
{
	return (x > 127) ? x - 256 : x;
}

uint8_t unsign(int8_t x)
{
	return (x >= 0) ? x : x + 256;
}

uint16_t unsign16(int16_t x)
{
	return (x >= 0) ? x : x + 0x10000;
}
