#include "mbc1.h"

#include "../cartridge.h"

#include <iostream>

byte_t MBC1::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < 0x4000) {
		if (advanced_mode && cartridge->rom_size() > 0x80000) {
			return cartridge->cartridge_data.at(0x4000 * ((ram_bank << 5) + rom_bank) + addr);
		} else {
			return cartridge->cartridge_data.at(addr);
		}
	} else if (0x4000 <= addr && addr < 0x8000) {
		return cartridge->cartridge_data.at(0x4000 * rom_bank + (addr - 0x4000));
	} else if (0xA000 <= addr && addr < 0xC000) {
		return read_ram(cartridge, addr - 0xA000);
	} else {
		return 0xFF;
	}
}

void MBC1::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (addr < 0x2000) {
		if ((data & 0xF) == 0xA) {
			ram_enabled = true;
		} else {
			ram_enabled = false;
		}
	} else if (0x2000 <= addr && addr < 0x4000) {
		int banks_required = cartridge->rom_size() / 1024 / 16;
		rom_bank = (data & 0x1F) % banks_required;
		if (rom_bank == 0) {
			rom_bank++;
		}
	} else if (0x4000 <= addr && addr < 0x6000) {
		ram_bank = data & 0x3;
	} else if (0x6000 <= addr && addr < 0x8000) {
		advanced_mode = data & 0x1;
	} else if (0xA000 <= addr && addr < 0xC000) {
		write_ram(cartridge, addr - 0xA000, data);
	}
}

byte_t MBC1::read_ram(Cartridge *cartridge, addr_t offset)
{
	if (!ram_enabled) {
		return 0xFF;
	}

	addr_t real_offset = translate_ram_addr(cartridge, offset);
	if (real_offset >= cartridge->ram_size()) {
		return 0xFF;
	}

	return cartridge->cartridge_ram.at(real_offset);
}

void MBC1::write_ram(Cartridge *cartridge, addr_t offset, byte_t data)
{	
	if (!ram_enabled) {
		return;
	}

	addr_t real_offset = translate_ram_addr(cartridge, offset);
	if (real_offset >= cartridge->ram_size()) {
		return;
	}

	cartridge->cartridge_ram.at(real_offset) = data;
}

addr_t MBC1::translate_ram_addr(Cartridge *cartridge, addr_t offset)
{
	if (advanced_mode && cartridge->ram_size() > 8192) {
		return ram_bank * 8192 + offset;
	} else {
		return offset;
	}
}
