#include "mbc2.h"

#include "../cartridge.h"

#include <iostream>

byte_t MBC2::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < 0x4000) {
		return cartridge->cartridge_data.at(addr);
	} else if (0x4000 <= addr && addr < 0x8000) {
		return cartridge->cartridge_data.at(0x4000 * rom_bank + (addr - 0x4000));
	} else if (0xA000 <= addr && addr < 0xC000) {
		return read_ram(cartridge, addr & 0x1FF);
	} else {
		return 0xFF;
	}
}

void MBC2::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (addr < 0x4000) {
		if (addr & (1 << 8)) {
			rom_bank = data & 0xF;
			if (rom_bank == 0) {
				rom_bank++;
			}
		} else {
			ram_enabled = (data == 0xA);
		}
	} else if (0xA000 <= addr && addr < 0xA200) {
		write_ram(cartridge, addr & 0x1FF, data);
	}
}

byte_t MBC2::read_ram(Cartridge *cartridge, addr_t offset)
{
	if (!ram_enabled) {
		return 0xFF;
	}

	return cartridge->read_ram(offset) & 0xF;
}

void MBC2::write_ram(Cartridge *cartridge, addr_t offset, byte_t data)
{	
	if (!ram_enabled) {
		return;
	}

	cartridge->write_ram(offset, data);
}
