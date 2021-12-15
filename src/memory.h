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
	RWY = 0xFF4A,
	RWX = 0xFF4B,
	RDMA = 0xFF46,
	RBGP = 0xFF47,
	ROBP0 = 0xFF48,
	ROBP1 = 0xFF49,
	NR10 = 0xFF10,
	NR11 = 0xFF11,
	NR12 = 0xFF12,
	NR13 = 0xFF13,
	NR14 = 0xFF14,
	NR21 = 0xFF16,
	NR22 = 0xFF17,
	NR23 = 0xFF18,
	NR24 = 0xFF19,
	NR30 = 0xFF1A,
	NR31 = 0xFF1B,
	NR32 = 0xFF1C,
	NR33 = 0xFF1D,
	NR34 = 0xFF1E,
	WAVE_PATTERN_RAM_START = 0xFF30,
	WAVE_PATTERN_RAM_END = 0xFF40,
	NR41 = 0xFF20,
	NR42 = 0xFF21,
	NR43 = 0xFF22,
	NR44 = 0xFF23,
	NR50 = 0xFF24,
	NR51 = 0xFF25,
	NR52 = 0xFF26
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

enum sound_status_flags {
	CH1_ON = 0x1,
	CH2_ON = 0x2,
	CH3_ON = 0x4,
	CH4_ON = 0x8,
	SOUND_ON = 0x80
};

enum nrx4_flags {
	CH_INIT = 0x80,
	CH_LENGTH_ENABLE = 0x40,
};

enum sound_pan_flags {
	CH1_RIGHT = 0x1,
	CH2_RIGHT = 0x2,
	CH3_RIGHT = 0x4,
	CH4_RIGHT = 0x8,
	CH1_LEFT = 0x10,
	CH2_LEFT = 0x20,
	CH3_LEFT = 0x40,
	CH4_LEFT = 0x80
};

class APU;
class Cartridge;

class Memory {
	public:
		std::array<byte_t, 0x10000> memory{};
		std::vector<byte_t> bootrom_data;
		APU *apu{};
		Cartridge *cartridge{};
		bool timer_written = false;
		bool bootrom_mapped = true;

		byte_t action_buttons{0xF};
		byte_t direction_buttons{0xF};

		Memory(Cartridge *Cartridge);
		void init_APU(APU *apu);

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
