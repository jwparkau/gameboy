#ifndef GB_MBC_MBC2_H
#define GB_MBC_MBC2_H

#include "mbc.h"

class MBC2 : public MBC {
	public:
		byte_t rom_bank = 1;
		bool ram_enabled = false;

		byte_t read(Cartridge *cartridge, addr_t addr);
		void write(Cartridge *cartridge, addr_t addr, byte_t data);

		byte_t read_ram(Cartridge *cartridge, addr_t offset);
		void write_ram(Cartridge *cartridge, addr_t offset, byte_t data);
};

#endif
