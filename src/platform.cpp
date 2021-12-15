#include "platform.h"

#include "gameboy.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <stdexcept>
#include <string>

static const std::string tag = "#Platform: ";

void Platform::init_platform()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
		throw std::runtime_error("SDL init failed");
	}

	window = SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width*scale_factor, height*scale_factor, SDL_WINDOW_SHOWN);
	if (!window) {
		throw std::runtime_error("SDL window could not be created");
	}

	SDL_SetWindowResizable(window, SDL_FALSE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		throw std::runtime_error("SDL renderer could not be created");
	}

	if (SDL_RenderSetLogicalSize(renderer, width, height) < 0) {
		throw std::runtime_error("SDL logical size could not be set");
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!texture) {
		throw std::runtime_error("SDL texture could not be created");
	}

	audio_spec_want.freq = 48000;
	audio_spec_want.format = AUDIO_F32SYS;
	audio_spec_want.channels = 2;
	audio_spec_want.samples = 4096;
	audio_spec_want.callback = nullptr;

	/*
	const int count = SDL_GetNumAudioDevices(0);
	for (int i = 0; i < count; i++) {
		printf("Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}
	*/

	audio_device = SDL_OpenAudioDevice(nullptr, false, &audio_spec_want, &audio_spec_have, 0);
	if (!audio_device) {
		throw std::runtime_error("SDL audio error");
	}
}

Platform::~Platform()
{
	if (texture) {
		SDL_DestroyTexture(texture);
	}
	texture = nullptr;

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	renderer = nullptr;

	if (window) {
		SDL_DestroyWindow(window);
	}
	window = nullptr;

	if (audio_device) {
		SDL_CloseAudioDevice(audio_device);
	}
	audio_device = 0;

	SDL_Quit();
}

void Platform::handle_input(GameBoy *gameboy)
{
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			gameboy->running = false;
			break;
		}

		switch (e.type) {
			case SDL_KEYDOWN:
				handle_keypress(gameboy, e.key.keysym.sym, true);
				break;
			case SDL_KEYUP:
				handle_keypress(gameboy, e.key.keysym.sym, false);
				break;
		}
	}
}

void Platform::handle_keypress(GameBoy *gameboy, SDL_Keycode sym, bool down)
{
	enum input_buttons button = translate_sym(sym);
	if (button != NOT_MAPPED) {
		gameboy->set_button(button, down);
	}
}

enum input_buttons Platform::translate_sym(SDL_Keycode sym)
{
	switch (sym) {
		case SDLK_RIGHT:
			return B_RIGHT;
		case SDLK_LEFT:
			return B_LEFT;
		case SDLK_UP:
			return B_UP;
		case SDLK_DOWN:
			return B_DOWN;
		case SDLK_x:
			return B_A;
		case SDLK_z:
			return B_B;
		case SDLK_1:
			return B_START;
		case SDLK_2:
			return B_SELECT;
		default:
			return NOT_MAPPED;
	}
}

void Platform::render(uint32_t *pixels)
{
	SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_Rect r = {0, 0, width, height};
	SDL_RenderFillRect(renderer, &r);

	SDL_UpdateTexture(texture, NULL, pixels, width * sizeof *pixels);

	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}
