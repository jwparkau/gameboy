#include "timer.h"

#include "memory.h"

#include <iostream>

Timer::Timer(Memory *memory) :
	memory(memory)
{
}

void Timer::tick_mcycle()
{
	byte_t t;
	int tcycles = 4;
	int freq;

	div_tcycles += tcycles;

	if (div_tcycles >= 256) {
		div_tcycles -= 256;
		t = memory->direct_read(RDIV);
		if (t + 1 > 0xFF) {
			memory->direct_write(RDIV, 0);
		} else {
			memory->direct_write(RDIV, t+1);
		}
	}

	byte_t tac = memory->direct_read(RTAC);
	if (tac & 0x4) {
		tima_tcycles += tcycles;
		switch (tac & 0x3) {
			case 0x0:
				freq = 1024;
				break;
			case 0x1:
				freq = 16;
				break;
			case 0x2:
				freq = 64;
				break;
			case 0x3:
				freq = 256;
				break;
			default:
				freq = 0;
		}

		if (tima_tcycles >= freq) {
			tima_tcycles -= freq;

			t = memory->direct_read(RTIMA);
			if (t + 1 > 0xFF) {
				memory->direct_write(RTIMA, memory->direct_read(RTMA));
				byte_t int_flag = memory->direct_read(RIF) | INT_TIMER;
				memory->direct_write(RIF, int_flag);
			} else {
				memory->direct_write(RTIMA, t + 1);
			}
		}
	}
}

