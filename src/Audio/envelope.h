#ifndef GB_AUDIO_ENVELOPE_H
#define GB_AUDIO_ENVELOPE_H

#include "../types.h"

#include "channel.h"

class Memory;

class Envelope {
	public:
		uint32_t period_timer = 0;
		addr_t envelope_reg = 0;
		byte_t current_volume;

		Memory *memory{};

		Envelope(enum sound_channel ch, Memory *memory);

		void on_clock();
		void on_trigger();
		void on_envelope_write();

		byte_t get_initial_volume();
		byte_t get_direction();
		byte_t get_period();
};

#endif
