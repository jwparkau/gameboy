#ifndef GB_TIMER_H
#define GB_TIMER_H

#include "types.h"

#include <cstdint>

class Memory;

class Timer {
	public:
		int div_tcycles = 0;
		int tima_tcycles = 0;

		Memory *memory = nullptr;

		Timer(Memory *memory);
		~Timer();
		void tick_mcycle();
		void inc(addr_t r);
};

#endif
