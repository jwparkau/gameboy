#ifndef GB_AUDIO_LENGTH_H
#define GB_AUDIO_LENGTH_H

#include "../types.h"
#include "channel.h"

class Memory;

class Length {
	public:
		addr_t length_enable_reg = 0;
		addr_t length_data_reg = 0;
		uint32_t length_timer = 0;
		enum sound_channel ch;

		Memory *memory{};

		Length(enum sound_channel, Memory *memory);

		void on_clock();
		void on_trigger();
		void disable_ch();

		void on_length_write();

		bool get_length_enable();
};

#endif
