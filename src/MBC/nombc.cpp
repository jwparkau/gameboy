#include "nombc.h"

#include "../cartridge.h"

#include <iostream>

static const int CARTRIDGE_ROM_END = 0x8000;
static const int CARTRIDGE_RAM_START = 0xA000;
static const int CARTRIDGE_RAM_END = 0xC000;

byte_t NoMBC::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < CARTRIDGE_ROM_END) {
		return cartridge->cartridge_data.at(addr);
	} else if (CARTRIDGE_RAM_START <= addr && addr < CARTRIDGE_RAM_END) {
		return cartridge->cartridge_ram.at(addr  - CARTRIDGE_RAM_START);
	} else {
		fprintf(stderr, "invalid addr %02X in read no MBC\n", addr);
		return 0xFF;
	}
}

void NoMBC::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (CARTRIDGE_RAM_START <= addr && addr < CARTRIDGE_RAM_END) {
		cartridge->cartridge_ram.at(addr -  CARTRIDGE_RAM_START) = data;
	} else {
		fprintf(stderr, "ignoring addr %02X in write no MBC\n", addr);
	}
}
