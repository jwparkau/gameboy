#include "memory.h"

#include "cartridge.h"

#include <fstream>
#include <iostream>

static const int CARTRIDGE_ROM_END = 0x8000;
static const int VRAM_END = 0xA000;
static const int EXTERNAL_RAM_END = 0xC000;
static const int WORK_RAM_END = 0xE000;
static const int MIRROR_END = 0xFE00;
static const int OAM_END = 0xFEA0;
static const int UNUSABLE_END = 0xFF00;
static const int IO_REG_END = 0xFF80;
static const int HRAM_END = 0xFFFF;
static const int IE_END = 0x10000;

static int MAX_BOOTROM_SIZE = 2048;

Memory::Memory(Cartridge *cartridge)
{
	this->cartridge = cartridge;
}

byte_t Memory::read(addr_t addr)
{
	if (addr < CARTRIDGE_ROM_END) {
		if (bootrom_mapped && addr < bootrom_data.size()) {
			return bootrom_data.at(addr);
		} else {
			return cartridge->read(addr);
		}
	} else if (addr < VRAM_END) {
		return memory.at(addr);
	} else if (addr < EXTERNAL_RAM_END) {
		return cartridge->read(addr);
	} else {
		if (addr == RJOYP) {
			byte_t rjoyp = memory.at(RJOYP);
			if (!(rjoyp & JOY_DIRECTION)) {
				return (rjoyp & ~0xF) + direction_buttons;
			} else {
				return (rjoyp & ~0xF) + action_buttons;
			}
		} else {
			return memory.at(addr);
		}
	}
}

void Memory::write(addr_t addr, byte_t data)
{
	if (addr == 0xFF02 && data == 0x81) {
		std::cout << memory.at(0xFF01) << std::flush;
	}
	if (addr == 0xFF50 && bootrom_mapped) {
		bootrom_mapped = false;
	}

	if (addr < CARTRIDGE_ROM_END) {
		if (bootrom_mapped && addr < bootrom_data.size()) {
			// cannot write to boot rom!
		} else {
			cartridge->write(addr, data);
		}
	} else if (addr < VRAM_END) {
		memory.at(addr) = data;
	} else if (addr < EXTERNAL_RAM_END) {
		cartridge->write(addr, data);
	} else {
		if (addr == RDMA) {
			do_dma_transfer(data * 0x100);
		}

		if (addr == RSTAT) {
			memory.at(addr) = (data & ~0x3) + (memory.at(addr) & 0x3);
		} else {
			memory.at(addr) = data;
		}
	}
		
}

byte_t Memory::direct_read(addr_t r)
{
	return memory.at(r);
}

void Memory::direct_write(addr_t r, byte_t data)
{

	memory.at(r) = data;
}

void Memory::request_interrupt(enum interrupt_flag i)
{
	memory.at(RIF) |= i;
}

void Memory::do_dma_transfer(addr_t source)
{
	addr_t destination = 0xFE00;
	for (int i = 0; i < 0xA0; i++) {
		memory.at(destination+i) = read(source+i);
	}
}

void Memory::map_bootrom(std::string filename)
{
	bootrom_data.resize(MAX_BOOTROM_SIZE);

	std::ifstream f(filename, std::ios_base::binary);
	
	f.read(reinterpret_cast<char *>(bootrom_data.data()), bootrom_data.size());
	std::streamsize bytes_read = f.gcount();

	if (bytes_read == 0) {
		throw std::runtime_error("ERROR while reading boot rom file: file " + filename + "was 0 bytes");
	}

	bootrom_data.resize(bytes_read);
}
