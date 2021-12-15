#ifndef GB_APU_H
#define GB_APU_H

#include "types.h"

#include <SDL2/SDL.h>

#include <array>
#include <memory>


class Channel;

class Memory;

class APU {
	public:
		std::array<float, 48000*2> buffer{};
		uint32_t buffer_index = 0;
		
		Memory *memory{};

		std::unique_ptr<Channel> ch1;	
		std::unique_ptr<Channel> ch2;	
		std::unique_ptr<Channel> ch3;	
		std::unique_ptr<Channel> ch4;	

		SDL_AudioDeviceID audio_device{};

		APU(Memory *memory);
		~APU();

		uint32_t sample_timer;
		uint32_t tcycles = 0;
		uint32_t frame_sequencer = 0;

		void tick_tcycle();
		void tick_mcycle();
		void trigger(addr_t addr);

		void clock_length();
		void clock_envelope();
		void clock_sweep();

		void on_length_write(addr_t addr);
		void on_freq_write(addr_t addr);
		void on_envelope_write(addr_t addr);

		void push_samples(float left, float right);
		void queue_audio();
};

#endif
