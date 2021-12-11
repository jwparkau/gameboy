#include "mbc3.h"

#include "../cartridge.h"

#include <iostream>

byte_t MBC3::read(Cartridge *cartridge, addr_t addr)
{
	if (addr < 0x4000) {
		return cartridge->cartridge_data.at(addr);
	} else if (0x4000 <= addr && addr < 0x8000) {
		return cartridge->cartridge_data.at(0x4000 * rom_bank + (addr - 0x4000));
	} else if (0xA000 <= addr && addr < 0xC000) {
		if (ram_bank <= 0x3) {
			return read_ram(cartridge, addr - 0xA000);
		} else {
			// read rtc
			return 0xFF;
		}
	} else {
		return 0xFF;
	}
}

void MBC3::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	if (addr < 0x2000) {
		if ((data & 0xF) == 0xA) {
			ram_enabled = true;
		} else {
			ram_enabled = false;
		}
	} else if (0x2000 <= addr && addr < 0x4000) {
		int banks_required = cartridge->rom_size() / 1024 / 16;
		rom_bank = (data & 0x7F) % banks_required;
		if (rom_bank == 0) {
			rom_bank++;
		}
	} else if (0x4000 <= addr && addr < 0x6000) {
		ram_bank = data;
	} else if (0x6000 <= addr && addr < 0x8000) {
		// latch
	} else if (0xA000 <= addr && addr < 0xC000) {
		if (ram_bank <= 0x3) {
			write_ram(cartridge, addr - 0xA000, data);
		} else {
			// write rtc
		}
	}
}

byte_t MBC3::read_ram(Cartridge *cartridge, addr_t offset)
{
	if (!ram_enabled) {
		return 0xFF;
	}

	addr_t real_offset = translate_ram_addr(offset);
	return cartridge->read_ram(real_offset);
}

void MBC3::write_ram(Cartridge *cartridge, addr_t offset, byte_t data)
{	
	if (!ram_enabled) {
		return;
	}

	addr_t real_offset = translate_ram_addr(offset);
	cartridge->write_ram(real_offset, data);
}

addr_t MBC3::translate_ram_addr(addr_t offset)
{
	return ram_bank * 8192 + offset;
}
