#ifndef GB_MBC_MBC_H
#define GB_MBC_MBC_H

#include "../types.h"

class Cartridge;

class MBC {
	public:
		virtual byte_t read(Cartridge *cartridge, addr_t addr) = 0;
		virtual void write(Cartridge *cartridge, addr_t addr, byte_t data) = 0;
};

#endif
