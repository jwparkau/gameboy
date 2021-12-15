#ifndef GB_AUDIO_CHANNEL_H
#define GB_AUDIO_CHANNEL_H

#include "../types.h"
#include <memory>

enum sound_channel {
	CH1,
	CH2,
	CH3,
	CH4
};

class Length;
class Sweep;
class Envelope;
class Memory;

class Channel {
	public:
		uint32_t freq_timer = 0;
		uint32_t wave_pos = 0;
		uint16_t LFSR = 0xFFFF;
		enum sound_channel ch;

		Memory *memory;
		std::unique_ptr<Length> length;
		std::unique_ptr<Sweep> sweep;
		std::unique_ptr<Envelope> envelope;

		Channel(enum sound_channel ch, Memory *memory);
		~Channel();

		void tick_tcycle();

		void clock_length();
		void clock_sweep();
		void clock_envelope();

		void trigger();
		void on_length_write();
		void on_freq_write();
		void on_envelope_write();

		void set_freq_timer();
		void on_ch4_expiry();
		void set_LFSR_bit(int n, bool set);

		float get_voltage();
		bool is_enabled();
		void set_enabled(bool enabled);

};

#endif
