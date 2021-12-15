#include "envelope.h"

#include "../memory.h"

#include <iostream>
#include <stdexcept>

Envelope::Envelope(enum sound_channel ch, Memory *memory) :
	memory(memory)
{
	switch (ch) {
		case CH1:
			envelope_reg = NR12;
			break;
		case CH2:
			envelope_reg = NR22;
			break;
		case CH4:
			envelope_reg = NR42;
			break;
		default:
			throw std::runtime_error("invalid channel for envelope");
	}
}

void Envelope::on_trigger()
{
	period_timer = get_period();
	current_volume = get_initial_volume();
}

void Envelope::on_envelope_write()
{
	period_timer = get_period();
	current_volume = get_initial_volume();
}

void Envelope::on_clock()
{
	byte_t period = get_period();
	if (period == 0) {
		return;
	}

	if (period_timer > 0) {
		period_timer--;

		if (period_timer == 0) {
			period_timer = period;

			byte_t direction = get_direction();
			if (direction == 1 && current_volume < 0xF) {
				current_volume++;
			} else if (direction == 0 && current_volume > 0) {
				current_volume--;
			}
		}
	}

}

byte_t Envelope::get_initial_volume()
{
	return memory->direct_read(envelope_reg) >> 4 & 0xF;
}

byte_t Envelope::get_direction()
{
	return memory->direct_read(envelope_reg) >> 3 & 0x1;
}

byte_t Envelope::get_period()
{
	return memory->direct_read(envelope_reg) & 0x7;
}
