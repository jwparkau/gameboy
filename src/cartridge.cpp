#include "cartridge.h"

#include "MBC/mbc.h"
#include "MBC/nombc.h"
#include "MBC/mbc1.h"
#include "MBC/mbc3.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

Cartridge::Cartridge() :
	mbc(std::make_unique<NoMBC>())
{
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

	set_mbc();
}

void Cartridge::set_mbc()
{
	byte_t cartridge_type = read(0x147);
	switch (cartridge_type) {
		case 0x0:
		case 0x8:
		case 0x9:
			mbc = std::make_unique<NoMBC>();
			break;
		case 0x1:
		case 0x2:
		case 0x3:
			mbc = std::make_unique<MBC1>();
			break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			mbc = std::make_unique<MBC3>();
			break;
		default:
			throw std::runtime_error("ERROR while reading cartridge: cartridge type " + std::to_string(cartridge_type) + " is not supported");
	}

	byte_t ram_type = read(0x149);
	switch (ram_type) {
		case 0:
		case 1:
			break;
		case 2:
			cartridge_ram.resize(0x2000);
			break;
		case 3:
			cartridge_ram.resize(0x8000);
			break;
		case 4:
			cartridge_ram.resize(0x20000);
			break;
		case 5:
			cartridge_ram.resize(0x10000);
			break;
		default:
			throw std::runtime_error("ERROR while reading cartridge: ram size " + std::to_string(ram_type) + " is invalid");
	}
}

std::size_t Cartridge::rom_size()
{
	return cartridge_data.size();
}

std::size_t Cartridge::ram_size()
{
	return cartridge_ram.size();
}


byte_t Cartridge::read(addr_t addr)
{
	return mbc->read(this, addr);
}

void Cartridge::write(addr_t addr, byte_t data)
{
	mbc->write(this, addr, data);
}
