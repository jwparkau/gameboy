#ifndef GB_MEMORY_H
#define GB_MEMORY_H

#include "types.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum io_reg {
	RIE = 0xFFFF,
	RIF = 0xFF0F,
	RJOYP = 0xFF00,
	RDIV = 0xFF04,
	RTIMA = 0xFF05,
	RTMA = 0xFF06,
	RTAC = 0xFF07,
	RLCDC = 0xFF40,
	RSTAT = 0xFF41,
	RSCY = 0xFF42,
	RSCX = 0xFF43,
	RLY = 0xFF44,
	RLYC = 0xFF45,
	RDMA = 0xFF46,
	RBGP = 0xFF47,
	ROBP0 = 0xFF48,
	ROBP1 = 0xFF49,
	RWY = 0xFF4A,
	RWX = 0xFF4B
};

enum joypad_flags {
	JOY_DIRECTION = 0x10,
	JOY_ACTION = 0x20
};

enum interrupt_flag {
	INT_VBLANK = 1,
	INT_STAT = 1 << 1,
	INT_TIMER =  1 << 2,
	INT_SERIAL = 1 << 3,
	INT_JOYPAD = 1 << 4
};

class Cartridge;

class Memory {
	public:
		std::array<byte_t, 0x10000> memory{};
		std::vector<byte_t> bootrom_data;
		Cartridge *cartridge{};
		bool timer_written = false;
		bool bootrom_mapped = true;

		byte_t action_buttons{0xF};
		byte_t direction_buttons{0xF};

		Memory(Cartridge *Cartridge);

		byte_t read(addr_t addr);
		void write(addr_t addr, byte_t data);

		byte_t direct_read(addr_t r);
		void direct_write(addr_t r, byte_t data);

		void request_interrupt(enum interrupt_flag i);

		void do_dma_transfer(addr_t source);
		void dump_vram();
		void map_bootrom(std::string filename);
};

#endif
