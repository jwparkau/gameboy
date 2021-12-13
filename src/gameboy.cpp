#include "gameboy.h"

#include "cartridge.h"
#include "cpu.h"
#include "memory.h"
#include "platform.h"
#include "ppu.h"
#include "timer.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <memory>
#include <stdexcept>


static const int CYCLES_PER_SECOND = 4194304;
static const int CYCLES_PER_FRAME = 70224;
static const double FRAMES_PER_SECOND = CYCLES_PER_SECOND / (double)CYCLES_PER_FRAME;

GameBoy::GameBoy(Platform *platform) :
	platform(platform),
	cartridge(std::make_unique<Cartridge>()),
	memory(std::make_unique<Memory>(cartridge.get())),
	timer(std::make_unique<Timer>(memory.get())),
	ppu(std::make_unique<PPU>(memory.get())),
	cpu(std::make_unique<CPU>(this))
{
}

GameBoy::~GameBoy() = default;

void GameBoy::load_cartridge(std::string filename)
{
	cartridge->load_cartridge_from_file(filename);
}

void GameBoy::map_bootrom(std::string filename)
{
	memory->map_bootrom(filename);
}

void GameBoy::start_emulation()
{
	running = true;
	int cycles = 0;

	uint64_t frame_start = 0;

	uint64_t freq = SDL_GetPerformanceFrequency();
	uint64_t frame_duration = freq / FRAMES_PER_SECOND;

	if (frame_duration == 0) {
		throw std::runtime_error("clock freq too small");
	}

	cpu->set_fake_boot_dmg_state();
	platform->init_platform();

	while (true) {
		frame_start = SDL_GetPerformanceCounter();

		cycles = 0;

		if (!running) {
			on_quit();
			break;
		}

		platform->handle_input(this);

		while (cycles < CYCLES_PER_FRAME) {
			cycles += cpu->step_instruction();
		}

		platform->render(ppu->framebuffer.data());

		uint64_t frame_time = SDL_GetPerformanceCounter() - frame_start;
		//std::cerr << "instantaneous fps: " << (double) freq / frame_time << "\n";
		int64_t ticks_sleep = frame_duration - frame_time;
		

		if (ticks_sleep <= 0) {
			std::cerr << "too slow!! - over by " << (-ticks_sleep) / (freq / 1000.0) << "ms\n";
			continue;
		}
		SDL_Delay(ticks_sleep / (freq / 1000.0) * 0.95);

		// Busy wait
		while (SDL_GetPerformanceCounter() < frame_start + frame_duration) {

		}
	}
}

void GameBoy::on_quit()
{
	cartridge->on_quit();
}

void GameBoy::tick_mcycle_no_cpu()
{
	timer->tick_mcycle();
	ppu->tick_mcycle();
}

void GameBoy::set_button(enum input_buttons button, bool down)
{
	if (button < 4) {
		if (down) {
			memory->direction_buttons &= ~(1 << button);
		} else {
			memory->direction_buttons |= (1 << button);
		}
	} else {
		if (down) {
			memory->action_buttons &= ~(1 << (button - 4));
		} else {
			memory->action_buttons |= (1 << (button - 4));
		}
	}

	if (down) {
		memory->request_interrupt(INT_JOYPAD);
	}
}
