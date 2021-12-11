#include "nombc.h"

#include "../cartridge.h"

#include <iostream>

byte_t NoMBC::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < 0x8000) {
		return cartridge->cartridge_data.at(addr);
	} else if (0xA000 <= addr && addr < 0xC000) {
		return cartridge->read_ram(addr - 0xA000);
	} else {
		return 0xFF;
	}
}

void NoMBC::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (0xA000 <= addr && addr < 0xC000) {
		cartridge->write_ram(addr - 0xA000, data);
	}
}
