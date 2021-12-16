#include "channel.h"

#include "length.h"
#include "sweep.h"
#include "envelope.h"

#include "../memory.h"

#include <iostream>
#include <stdexcept>

static const int WAVE_DUTY_TABLE[4][8] = {
	{0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 0, 0, 0, 0, 1, 1},
	{0, 0, 0, 0, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 0, 0}
};

static const int VOLUME_SHIFT_TABLE[4] = {
	4, 0, 1, 2
};

Channel::Channel(enum sound_channel ch, Memory *memory) :
	ch(ch),
	memory(memory)
{
	length = std::make_unique<Length>(ch, memory);
	if (ch == CH1) {
		sweep = std::make_unique<Sweep>(memory, this);
	}
	if (ch == CH1 || ch == CH2 || ch == CH4) {
		envelope = std::make_unique<Envelope>(ch, memory);
	}
}

Channel::~Channel() = default;

void Channel::tick_tcycle()
{
	if (!is_enabled()) {
		return;
	}

	if (freq_timer > 0) {
		freq_timer--;

		if (freq_timer == 0) {
			set_freq_timer();
			wave_pos++;

			if (ch == CH4) {
				on_ch4_expiry();
			}
		}
	}
}

float Channel::get_voltage()
{
	uint16_t dac_input = 0;
	byte_t data;

	switch (ch) {
		case CH1:
			dac_input = envelope->current_volume * WAVE_DUTY_TABLE[memory->direct_read(NR11) >> 6 & 0x3][wave_pos %= 8];
			break;
		case CH2:
			dac_input = envelope->current_volume * WAVE_DUTY_TABLE[memory->direct_read(NR21) >> 6 & 0x3][wave_pos %= 8];
			break;
		case CH3:
			if (memory->direct_read(NR30) & 0x80) {
				wave_pos %= 32;
				data = memory->direct_read(WAVE_PATTERN_RAM_START + wave_pos / 2);
				if (wave_pos % 2 == 0) {
					dac_input = data & 0xF;
				} else {
					dac_input = data >> 4 & 0xF;
				}

				dac_input >>= VOLUME_SHIFT_TABLE[memory->direct_read(NR32) >> 5 & 0x3];
			} else {
				dac_input = 0;
			}
			break;
		case CH4:
			dac_input = envelope->current_volume * (1 - (LFSR & 0x1));
			break;
	}

	return dac_input / 7.5 - 1.0;
}

void Channel::clock_length()
{
	if (!is_enabled()) {
		return;
	}

	length->on_clock();
}

void Channel::clock_sweep()
{
	if (!is_enabled()) {
		return;
	}

	if (sweep) {
		sweep->on_clock();
	}
}

void Channel::clock_envelope()
{
	if (!is_enabled()) {
		return;
	}

	if (envelope) {
		envelope->on_clock();
	}
}

void Channel::trigger()
{
	set_enabled(true);
	length->on_trigger();
	if (sweep) {
		sweep->on_trigger();
	}
	if (envelope) {
		envelope->on_trigger();
	}
	LFSR = 0xFFFF;
	set_freq_timer();
	wave_pos = 0;
}

void Channel::on_length_write()
{
	length->on_length_write();
}

void Channel::on_freq_write()
{
	set_freq_timer();
}

void Channel::on_envelope_write()
{
	envelope->on_envelope_write();
}

void Channel::set_freq_timer()
{
	if (ch == CH1 || ch == CH2 || ch == CH3) {
		addr_t freq_low_addr;
		addr_t freq_high_addr;

		switch (ch) {
			case CH1:
				freq_low_addr = NR13;
				freq_high_addr = NR14;
				break;
			case CH2:
				freq_low_addr = NR23;
				freq_high_addr = NR24;
				break;
			case CH3:
				freq_low_addr = NR33;
				freq_high_addr = NR34;
				break;
			default:
				throw std::runtime_error("unreachable: ch must be CH1 CH2 CH3");
		}

		uint32_t freq = ((memory->direct_read(freq_high_addr) & 0x7) << 8) + memory->direct_read(freq_low_addr);
		freq_timer = (2048 - freq) * 4;
	} else if (ch == CH4) {
		byte_t poly_reg = memory->direct_read(NR43);
		byte_t divisor_code = (poly_reg & 0x7);
		byte_t shift_amount = (poly_reg >> 4) & 0xF;

		byte_t divisor = (divisor_code != 0) ? 16 * divisor_code : 8;
		freq_timer = divisor << shift_amount;
	}
}

void Channel::on_ch4_expiry()
{
	byte_t xor_result = (LFSR & 0x1) ^ (LFSR >> 1 & 0x1);
	LFSR >>= 1;
	set_LFSR_bit(14, xor_result);
	if (memory->direct_read(NR43) & 0x8) {
		set_LFSR_bit(6, xor_result);
	}
}

void Channel::set_LFSR_bit(int n, bool set)
{
	if (set) {
		LFSR |= (1 << n);
	} else {
		LFSR &= ~(1 << n);
	}
}

bool Channel::is_enabled()
{
	return memory->direct_read(NR52) & (1 << ch);
}

void Channel::set_enabled(bool enable)
{	
	byte_t status = memory->direct_read(NR52);
	if (enable) {
		status |= (1 << ch);
	} else {
		status &= ~(1 << ch);
	}
	memory->direct_write(NR52, status);
}
