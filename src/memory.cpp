#include "memory.h"

#include "apu.h"
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

Memory::Memory(Cartridge *cartridge) :
	cartridge(cartridge)
{
}

void Memory::init_APU(APU *apu)
{
	this->apu = apu;
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
		} else if (addr == NR10) {
			return memory.at(addr) | 0x80;
		} else if (addr == NR11 || addr == NR21) {
			return memory.at(addr) | 0x3F;
		} else if (addr == NR13 || addr == NR23 || addr == NR31 || addr == NR33 || addr == NR41) {
			return 0xFF;
		} else if (addr == NR14 || addr == NR24 || addr == NR34 || addr == NR44) {
			return memory.at(addr) | 0xBF;
		} else if (addr == NR30) {
			return memory.at(addr) | 0x7F;
		} else if (addr == NR32) {
			return memory.at(addr) | 0x9F;
		} else if (0xFF27 <= addr && addr <= 0xFF2F) {
			return 0xFF;
		} else if (addr == NR52) {
			return memory.at(addr) | 0x70;
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
		} else if (addr == NR12 || addr == NR22 || addr == NR42) {
			apu->on_envelope_write(addr);
		} else if (addr == NR11 || addr == NR21 || addr == NR41) {
			apu->on_length_write(addr);
		} else if (addr == NR31) {
			apu->on_length_write(addr);
		} else if (addr == NR14 || addr == NR24 || addr == NR34 || addr == NR44) {
			if ((data & CH_INIT)) {
				apu->trigger(addr);
			}
		} else if (addr == NR13 || addr == NR23 || addr == NR33) {
			apu->on_freq_write(addr);
		} else if (addr == NR14 || addr == NR24 || addr == NR34) {
			apu->on_freq_write(addr);
		} else if (addr == NR43) {
			apu->on_freq_write(addr);
		}

		if (addr == RSTAT) {
			memory.at(addr) = (data & ~0x3) + (memory.at(addr) & 0x3);
		} else if (addr == RLY) {
			// do nothing
		} else if (addr == NR52) {
			memory.at(addr) = (data & 0xF0) + (memory.at(addr) & 0x0F);
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

	std::cerr << "reading boot rom file...\n";

	std::ifstream f(filename, std::ios_base::binary);
	
	f.read(reinterpret_cast<char *>(bootrom_data.data()), bootrom_data.size());
	std::streamsize bytes_read = f.gcount();

	if (bytes_read == 0) {
		throw std::runtime_error("ERROR while reading boot rom file: file " + filename + "was 0 bytes");
	}

	std::cerr << "boot rom size - " << bytes_read << " bytes\n";

	bootrom_data.resize(bytes_read);
}
