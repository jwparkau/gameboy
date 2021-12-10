#ifndef GB_PPU_H
#define GB_PPU_H

#include "types.h"

#include <array>
#include <cstdint>
#include <queue>
#include <vector>

class Memory;

enum lcdc_flag {
	BG_ENABLE = 0x1,
	OBJ_ENABLE = 0x2,
	OBJ_SIZE = 0x4,
	BG_MAP_AREA = 0x8,
	BG_DATA_AREA = 0x10,
	WINDOW_ENABLE = 0x20,
	WINDOW_MAP_AREA = 0x40,
	LCD_ENABLE = 0x80
};

enum stat_flags {
	LYC_EQUAL = 0x4,
	HBLANK_INT = 0x8,
	VBLANK_INT = 0x10,
	OAM_INT = 0x20,
	LYC_INT = 0x40
};

enum ppu_mode {
	HBLANK,
	VBLANK,
	OAM_SCAN,
	PIXEL_DRAW
};

enum sprite_flags {
	PALETTE_NUMBER = 0x10,
	FLIP_X = 0x20,
	FLIP_Y = 0x40,
	BG_PRIORITY = 0x80
};

struct Sprite {
	byte_t y;
	byte_t x;
	byte_t tile_index;
	byte_t flags;
};

class PPU {
	public:
		int dot_counter = 0;
		int window_line = 0;
		int pixels_pushed = 0;
		bool lcd_was_enabled = true;
		std::array<std::uint32_t, 160*144> framebuffer{};

		Memory *memory{};
		std::vector<Sprite> scanline_sprites;

		PPU(Memory *memory);

		void tick_tcycle();
		void tick_mcycle();

		byte_t read(addr_t addr);
		void write(addr_t addr, byte_t data);
		byte_t direct_read(addr_t addr);
		void direct_write(addr_t addr, byte_t data);

		enum ppu_mode get_mode();
		void set_mode(enum ppu_mode mode);
		void do_oam_scan();
		void do_pixel_draw();
		void set_pixel(byte_t x, byte_t y, byte_t colour, byte_t palette);
		int get_colour_at(addr_t tiledata, byte_t x, byte_t y);

		byte_t LY();
		void inc_LY();
		byte_t LCDC();
		byte_t STAT();

		bool get_lcdc_flag(enum lcdc_flag);
};

#endif
