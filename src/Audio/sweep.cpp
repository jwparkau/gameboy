#include "sweep.h"
#include "channel.h"

#include "../memory.h"

Sweep::Sweep(Memory *memory, Channel *channel) :
	sweep_reg(NR10),
	memory(memory),
	channel(channel)
{
}

void Sweep::on_trigger()
{
	shadow_freq = channel->freq_timer;

	byte_t period = get_period();
	byte_t shift = get_shift();

	sweep_timer = (period != 0) ? period : 8;
	sweep_enabled = (period != 0 || shift != 0);

	if (shift != 0) {
		calculate_new_freq();
	}
}

void Sweep::on_clock()
{
	if (sweep_timer > 0) {
		sweep_timer--;

		if (sweep_timer == 0) {
			byte_t period = get_period();
			sweep_timer = (period != 0) ? period : 8;

			if (sweep_enabled && period > 0) {
				uint32_t new_freq = calculate_new_freq();

				if (new_freq < 2048 && get_shift() > 0) {
					shadow_freq = new_freq;
					set_ch1_freq(new_freq);

					calculate_new_freq();
				}
			}

		}
	}

}

uint32_t Sweep::calculate_new_freq()
{
	byte_t shift = get_shift();
	byte_t direction = get_direction();

	uint32_t new_freq = shadow_freq >> shift;

	if (direction == 0) {
		new_freq = shadow_freq + new_freq;
	} else if (direction == 1) {
		new_freq = shadow_freq - new_freq;
	}

	if (new_freq >= 2048) {
		disable_ch1();
	}

	return new_freq;
}

byte_t Sweep::get_period()
{
	return memory->direct_read(sweep_reg) >> 4 & 0xF;
}

byte_t Sweep::get_direction()
{
	return memory->direct_read(sweep_reg) >> 3 & 0x1;
}

byte_t Sweep::get_shift()
{
	return memory->direct_read(sweep_reg) & 0x7;
}

void Sweep::disable_ch1()
{
	// lucky only channel has sweep, so we can get away with just disabling it directly
	byte_t status = memory->direct_read(NR52);
	status &= ~CH1_ON;
	memory->direct_write(NR52, status);
}

void Sweep::set_ch1_freq(uint32_t freq)
{
	/*
	byte_t freq_low = freq & 0xFF;
	memory->direct_write(NR13, freq_low);

	byte_t freq_high = memory->direct_read(NR14);
	freq_high &= ~0x7;
	freq_high += freq >> 8 & 0x7;
	memory->direct_write(NR14, freq_high);
	*/
	channel->freq_timer = freq;

}

uint32_t Sweep::get_ch1_freq()
{
	byte_t freq_low = memory->direct_read(NR13);
	byte_t freq_high = memory->direct_read(NR14) & 0x7;

	return (freq_high << 8) + freq_low;
}
