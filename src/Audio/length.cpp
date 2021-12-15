#include "length.h"

#include "../memory.h"

#include <stdexcept>

Length::Length(enum sound_channel ch, Memory *memory) :
	ch(ch),
	memory(memory)
{
	switch (ch) {
		case CH1:
			length_enable_reg = NR14;
			length_data_reg = NR11;
			break;
		case CH2:
			length_enable_reg = NR24;
			length_data_reg = NR21;
			break;
		case CH3:
			length_enable_reg = NR34;
			length_data_reg = NR31;
			break;
		case CH4:
			length_enable_reg = NR44;
			length_data_reg = NR41;
			break;
		default:
			throw std::runtime_error("invalid ch for length");
	}
}

void Length::on_trigger()
{
	if (length_timer == 0) {
		length_timer = (ch != CH3) ? 64 : 256;
	}
}

void Length::on_clock()
{
	if (get_length_enable()) {
		length_timer--;

		if (length_timer == 0) {
			disable_ch();
		}
	}
}

void Length::disable_ch()
{
	byte_t status = memory->direct_read(NR52);
	status &= ~(1 << ch);
	memory->direct_write(NR52, status);
}

bool Length::get_length_enable()
{
	return memory->direct_read(length_enable_reg) & CH_LENGTH_ENABLE;
}

void Length::on_length_write()
{
	byte_t mask = 0;

	switch (ch) {
		case CH1:
		case CH2:
		case CH4:
			mask = 0x3F;
			break;
		case CH3:
			mask = 0xFF;
			break;
	}

	length_timer = memory->direct_read(length_data_reg) & mask;
}
