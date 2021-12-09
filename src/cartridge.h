#ifndef GB_CARTRIDGE_H
#define GB_CARTRIDGE_H

#include "types.h"

#include <cstddef>
#include <vector>
#include <string>
#include <memory>


#define MAX_CARTRIDGE_SIZE ((size_t) 0x800000)

class MBC;

class Cartridge {
	public:
		std::vector<byte_t> cartridge_data;
		std::vector<byte_t> cartridge_ram;
		std::unique_ptr<MBC> mbc;

		Cartridge();
		~Cartridge();

		void load_cartridge_from_file(std::string filename);
		void set_mbc();
		std::size_t rom_size();
		std::size_t ram_size();

		byte_t read(addr_t addr);
		void write(addr_t addr, byte_t data);
};

#endif
