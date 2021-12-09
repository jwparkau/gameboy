#include "nombc.h"

#include "../cartridge.h"

#include <iostream>

byte_t NoMBC::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < 0x8000) {
		return cartridge->cartridge_data.at(addr);
	} else if (0xA000 <= addr && addr < 0xC000) {
		if (addr - 0xA000 < cartridge->ram_size()) {
			return cartridge->cartridge_ram.at(addr - 0xA000);
		} else {
			return 0xFF;
		}
	} else {
		fprintf(stderr, "invalid addr %02X in read no MBC\n", addr);
		return 0xFF;
	}
}

void NoMBC::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (0xA000 <= addr && addr < 0xC000) {
		if (addr - 0xA000 < cartridge->ram_size()) {
			cartridge->cartridge_ram.at(addr - 0xA000) = data;
		}
	} else {
		fprintf(stderr, "ignoring addr %02X in write no MBC\n", addr);
	}
}
