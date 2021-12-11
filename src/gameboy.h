#ifndef GB_GAMEBOY_H
#define GB_GAMEBOY_H

#include <memory>
#include <string>

class Platform;
class Cartridge;
class Memory;
class CPU;
class Timer;
class PPU;

/*
 * Represents the Game Boy as a whole.
 * Keeps track of all the components of the Game Boy
 */

enum input_buttons : char {
	B_RIGHT,
	B_LEFT,
	B_UP,
	B_DOWN,
	B_A,
	B_B,
	B_SELECT,
	B_START,
	NUM_BUTTONS,
	NOT_MAPPED
};

class GameBoy {
	public:
		Platform *platform{};
		bool running = false;

		std::unique_ptr<Cartridge> cartridge;
		std::unique_ptr<Memory> memory;
		std::unique_ptr<Timer> timer;
		std::unique_ptr<PPU> ppu;
		std::unique_ptr<CPU> cpu;

		GameBoy(Platform *platform);
		~GameBoy();

		void load_cartridge(std::string filename);
		void map_bootrom(std::string filename);
		void start_emulation();
		void on_quit();

		void tick_mcycle_no_cpu();

		void set_button(enum input_buttons button, bool down);
};

#endif
