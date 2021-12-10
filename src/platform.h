#ifndef GB_PLATFORM_H
#define GB_PLATFORM_H

#include <SDL2/SDL.h>

class GameBoy;

enum input_buttons : char;

class Platform {
	public:
		SDL_Window *window{};
		SDL_Renderer *renderer{};
		SDL_Texture *texture{};

		int width = 160;
		int height = 144;
		int scale_factor = 4;

		~Platform();

		void init_platform();
		void handle_input(GameBoy *gameboy);
		void handle_keypress(GameBoy *gameboy, SDL_Keycode sym, bool down);
		enum input_buttons translate_sym(SDL_Keycode sym);
		void render(uint32_t *pixels);
};

#endif
