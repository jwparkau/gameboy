#include "cartridge.h"

#include "MBC/mbc.h"
#include "MBC/nombc.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

Cartridge::Cartridge()
{
	// TODO: make proper MBC by reading the ROM!
	mbc = std::make_unique<NoMBC>();
}

Cartridge::~Cartridge() = default;

void Cartridge::load_cartridge_from_file(std::string filename)
{
	cartridge_data.resize(MAX_CARTRIDGE_SIZE);

	std::ifstream f(filename, std::ios_base::binary);

	f.read(reinterpret_cast<char *>(cartridge_data.data()), cartridge_data.size());
	std::streamsize bytes_read = f.gcount();

	if (bytes_read == 0) {
		throw std::runtime_error("ERROR while reading cartridge file: file " + filename + " was 0 bytes");
	}

	cartridge_data.resize(bytes_read);
}

byte_t Cartridge::read(addr_t addr)
{
	return mbc->read(this, addr);
}

void Cartridge::write(addr_t addr, byte_t data)
{
	mbc->write(this, addr, data);
}
