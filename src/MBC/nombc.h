#ifndef GB_MBC_NOMBC_H
#define GB_MBC_NOMBC_H

#include "mbc.h"

class NoMBC : public MBC {
	public:
		byte_t read(Cartridge *cartridge, addr_t addr);
		void write(Cartridge *cartridge, addr_t addr, byte_t data);
};

#endif
