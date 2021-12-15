#ifndef GB_AUDIO_SWEEP_H
#define GB_AUDIO_SWEEP_H

#include "../types.h"

class Memory;
class Channel;

class Sweep {
	public:
		uint32_t sweep_timer = 0;
		bool sweep_enabled = false;
		uint32_t shadow_freq = 0;
		addr_t sweep_reg;

		Memory *memory{};
		Channel *channel{};

		Sweep(Memory *memory, Channel *channel);

		void on_clock();
		void on_trigger();

		byte_t get_period();
		byte_t get_direction();
		byte_t get_shift();
		uint32_t calculate_new_freq();

		void disable_ch1();
		void set_ch1_freq(uint32_t freq);
		uint32_t get_ch1_freq();
};

#endif
