#include "nombc.h"

#include "../cartridge.h"

byte_t NoMBC::read(Cartridge *cartridge, addr_t addr)
{
	return cartridge->cartridge_data.at(addr);
}

void NoMBC::write(Cartridge *cartridge, addr_t addr, byte_t data)
{
	// no writes to ROM with no MBC!
}
