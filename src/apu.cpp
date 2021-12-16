#include "apu.h"

#include "Audio/channel.h"

#include "memory.h"

#include <iostream>
#include <stdexcept>

static const int FS_TICK_PERIOD = 8192;
static const int FS_NUM_STEPS = 8;
static const int TCYCLES_PER_SAMPLE = 87;
static const int DELAY_AMOUNT = 624;

APU::APU(Memory *memory) :
	memory(memory),
	ch1(std::make_unique<Channel>(CH1, memory)),
	ch2(std::make_unique<Channel>(CH2, memory)),
	ch3(std::make_unique<Channel>(CH3, memory)),
	ch4(std::make_unique<Channel>(CH4, memory)),
	sample_timer(TCYCLES_PER_SAMPLE),
	delay(DELAY_AMOUNT)
{
}

APU::~APU() = default;

void APU::tick_mcycle()
{
	for (int i = 0; i < 4; i++) {
		tick_tcycle();
	}
}

void APU::tick_tcycle()
{
	if (!(memory->direct_read(NR52) & SOUND_ON)) {
		return;
	}

	tcycles++;
	if (tcycles == FS_TICK_PERIOD) {
		frame_sequencer++;

		frame_sequencer %= FS_NUM_STEPS;

		switch (frame_sequencer) {
			case 0:
			case 4:
				clock_length();
				break;
			case 1:
			case 3:
			case 5:
				break;
			case 2:
			case 6:
				clock_length();
				clock_sweep();
				break;
			case 7:
				clock_envelope();
				break;
			default:
				throw std::runtime_error("invalid value for frame seq step");
		}
	}

	tcycles %= FS_TICK_PERIOD;

	ch1->tick_tcycle();
	ch2->tick_tcycle();
	ch3->tick_tcycle();
	ch4->tick_tcycle();

	float left = 0;
	float right = 0;
	int left_n = 0;
	int right_n = 0;

	byte_t pan = memory->direct_read(NR51);
	float v;

	if (ch1->is_enabled()) {
		v = ch1->get_voltage();
		if (pan & CH1_LEFT) {
			left += v;
			left_n++;
		}
		if (pan & CH1_RIGHT) {
			right += v;
			right_n++;
		}
	}
	if (ch2->is_enabled()) {
		v = ch2->get_voltage();
		if (pan & CH2_LEFT) {
			left += v;
			left_n++;
		}
		if (pan & CH2_RIGHT) {
			right += v;
			right_n++;
		}
	}
	if (ch3->is_enabled()) {
		v = ch3->get_voltage();
		if (pan & CH3_LEFT) {
			left += v;
			left_n++;
		}
		if (pan & CH3_RIGHT) {
			right += v;
			right_n++;
		}
	}
	if (ch4->is_enabled()) {
		v = ch4->get_voltage();
		if (pan & CH4_LEFT) {
			left += v;
			left_n++;
		}
		if (pan & CH4_RIGHT) {
			right += v;
			right_n++;
		}
	}

	if (left_n != 0) {
		//left = left / (float)left_n;
	}
	if (right_n != 0) {
		//right = right / (float)right_n;
	}

	sample_timer--;
	if (sample_timer == 0) {
		sample_timer = TCYCLES_PER_SAMPLE;
		push_samples(left, right);
		if (delay > 0) {
			delay--;
			sample_timer++;
		}
	}

}

void APU::clock_length()
{
	ch1->clock_length();
	ch2->clock_length();
	ch3->clock_length();
	ch4->clock_length();
}

void APU::clock_envelope()
{
	ch1->clock_envelope();
	ch2->clock_envelope();
	ch4->clock_envelope();
}

void APU::clock_sweep()
{
	ch1->clock_sweep();
}

void APU::trigger(addr_t addr)
{
	switch (addr) {
		case NR14:
			ch1->trigger();
			break;
		case NR24:
			ch2->trigger();
			break;
		case NR34:
			ch3->trigger();
			break;
		case NR44:
			ch4->trigger();
			break;
	}
}

void APU::on_length_write(addr_t addr)
{
	switch (addr) {
		case NR11:
			ch1->on_length_write();
			break;
		case NR21:
			ch2->on_length_write();
			break;
		case NR31:
			ch3->on_length_write();
			break;
		case NR41:
			ch4->on_length_write();
			break;
	}
}

void APU::on_freq_write(addr_t addr)
{
	switch (addr) {
		case NR13:
		case NR14:
			ch1->on_freq_write();
			break;
		case NR23:
		case NR24:
			ch2->on_freq_write();
			break;
		case NR33:
		case NR34:
			ch3->on_freq_write();
			break;
		case NR43:
			ch4->on_freq_write();
			break;
	}
}

void APU::on_envelope_write(addr_t addr)
{
	switch (addr) {
		case NR12:
			ch1->on_envelope_write();
			break;
		case NR22:
			ch2->on_envelope_write();
			break;
		case NR42:
			ch4->on_envelope_write();
			break;
	}
}

void APU::push_samples(float left, float right)
{
	float scale_factor = 0.02;
	byte_t volume = memory->direct_read(NR50);
	left *= (volume >> 4 & 0x7) * scale_factor;
	right *= (volume & 0x7) * scale_factor;

	//printf("%f %f\n", left, right);

	buffer[buffer_index++] = left;
	buffer[buffer_index++] = right;
}

void APU::queue_audio()
{
	if (!audio_device) {
		return;
	}

	SDL_QueueAudio(audio_device, buffer.data(), 1600 * sizeof(decltype(buffer)::value_type));
	buffer_index = 0;
	delay = DELAY_AMOUNT;
}
