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
		bool has_ram = false;
		bool has_battery = false;
		bool is_mbc2 = false;
		std::unique_ptr<MBC> mbc;
		std::string cartridge_filename;

		Cartridge();
		~Cartridge();

		void load_cartridge_from_file(std::string filename);
		void load_cartridge_ram();
		void save_cartridge_ram();
		void read_cartridge_header();
		std::string get_save_filename();
		std::size_t rom_size();
		std::size_t ram_size();

		void on_quit();

		byte_t read(addr_t addr);
		void write(addr_t addr, byte_t data);
		byte_t read_ram(addr_t offset);
		void write_ram(addr_t offset, byte_t data);
};

#endif
