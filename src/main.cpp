#include "gameboy.h"
#include "platform.h"

#include <SDL2/SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv)
{
	std::cout << "Gameboy Emulator..." << std::endl;

	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " BOOT_ROM_FILE CARTRIDGE_ROM_FILE\n";
		return EXIT_FAILURE;
	}
	std::string bootrom(argv[1]);
	std::string filename(argv[2]);

	std::unique_ptr<Platform> platform = std::make_unique<Platform>();
	std::unique_ptr<GameBoy> gameboy = std::make_unique<GameBoy>(platform.get());

	gameboy->load_cartridge(filename);
	gameboy->map_bootrom(bootrom);

	gameboy->start_emulation();

	return 0;
}
