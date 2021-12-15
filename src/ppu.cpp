#include "ppu.h"

#include "memory.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

static const int DOTS_PER_SCANLINE = 456;

static const uint32_t REAL_COLOURS[4] =  {
	0xFFFFFFFF,
	0xFFD3D3D3,
	0xFF404040,
	0xFF000000
};

PPU::PPU(Memory *memory) :
	memory(memory)
{
	framebuffer.fill(0xFFFFFFFF);
}

void PPU::tick_mcycle()
{
	for (int i = 0; i < 4; i++) {
		tick_tcycle();
	}
}

void PPU::tick_tcycle()
{
	if (!(LCDC() & LCD_ENABLE)) {
		dot_counter = 0;
		window_line = 0;
		pixels_pushed = 0;
		direct_write(RLY, 0);
		set_mode(HBLANK);
		return;
	}

	enum ppu_mode mode = get_mode();
	enum ppu_mode prev_mode = mode;
	byte_t ly = LY();

	switch (mode) {
		case HBLANK:
			if (dot_counter + 1 == DOTS_PER_SCANLINE) {
				if (ly < 143) {
					mode = OAM_SCAN;
				} else if (ly == 143) {
					mode = VBLANK;
				} else {
					throw std::runtime_error("how am I in HBLANK?");
				}
			}
			break;
		case VBLANK:
			if (dot_counter + 1 == DOTS_PER_SCANLINE && ly == 153) {
				mode = OAM_SCAN;
			}
			break;
		case OAM_SCAN:
			if (dot_counter == 0) {
				do_oam_scan();
			} else if (dot_counter + 1 == 80) {
				mode = PIXEL_DRAW;
			}
			break;
		case PIXEL_DRAW:
			if (ly >= 144) {
				throw std::runtime_error("ly too big in pixel draw?");
			}
			do_pixel_draw();
			mode = HBLANK;
			break;
	}

	dot_counter++;
	if (dot_counter >= DOTS_PER_SCANLINE) {
		dot_counter = 0;

		inc_LY();
		if (LY() == direct_read(RLYC)) {
			if (direct_read(RSTAT) & LYC_INT) {
				memory->request_interrupt(INT_STAT);
			}
		}
	}

	if (mode != prev_mode) {
		set_mode(mode);
		if (STAT() & OAM_INT && mode == OAM_SCAN) {
			memory->request_interrupt(INT_STAT);
		}
		if (mode == VBLANK) {
			window_line = 0;
			memory->request_interrupt(INT_VBLANK);
			if (STAT() & VBLANK_INT) {
				memory->request_interrupt(INT_STAT);
			}
		}
		if (STAT() & HBLANK_INT && mode == HBLANK) {
			memory->request_interrupt(INT_STAT);
		}

	}

	if (LY() == read(RLYC)) {
		direct_write(RSTAT, direct_read(RSTAT) | LYC_EQUAL);
	} else {
		direct_write(RSTAT, direct_read(RSTAT) & ~LYC_EQUAL);
	}
}

void PPU::do_pixel_draw()
{
	byte_t y = LY();
	byte_t lcdc = LCDC();
	bool window_drawn = false;
	for (byte_t x = 0; x < 160; x++) {
		int bg_colour = -1;
		int sprite_colour = -1;
		bool bg_priority = false;
		bool palette_number = 0;

		if (lcdc & BG_ENABLE) {
			bool draw_window = false;
			byte_t wx = direct_read(RWX);
			byte_t wy = direct_read(RWY);
			byte_t scx = direct_read(RSCX);
			byte_t scy = direct_read(RSCY);


			if (lcdc & WINDOW_ENABLE) {
				if (x >= wx - 7 && y >= wy) {
					draw_window = true;
				}
			}

			byte_t tile_x, tile_y;
			addr_t tilemap;
			if (draw_window) {
				tile_x = x - wx + 7;
				tile_y = window_line;
				if (lcdc & WINDOW_MAP_AREA) {
					tilemap = 0x9C00;
				} else {
					tilemap = 0x9800;
				}
			} else {
				tile_x = x + scx;
				tile_y = y + scy;
				if (lcdc & BG_MAP_AREA) {
					tilemap = 0x9C00;
				} else {
					tilemap = 0x9800;
				}
			}

			addr_t tiledata;
			byte_t tile_index = read(tilemap + 32*(tile_y/8) + (tile_x/8));
			if (lcdc & BG_DATA_AREA) {
				tiledata = 0x8000 + 16 * tile_index;
			} else {
				tiledata = 0x9000 + 16 * sign(tile_index);
			}

			bg_colour = get_colour_at(tiledata, tile_x % 8, tile_y % 8);

			if (draw_window) {
				window_drawn = true;
			}
		}

		if (lcdc & OBJ_ENABLE) {
			for (Sprite sprite : scanline_sprites) {
				if (x < sprite.x && sprite.x <= x+8) {
					byte_t size = (lcdc & OBJ_SIZE) ? 16 : 8;
					
					byte_t tx = (x + 8 - sprite.x);
					byte_t ty = (y + 16 - sprite.y);

					if (sprite.flags & FLIP_X) {
						tx = 7 - tx;
					}
					if (sprite.flags & FLIP_Y) {
						ty = size - 1 - ty;
					}

					addr_t tiledata;
					if (size == 8) {
						tiledata = 0x8000 + 16 * sprite.tile_index;
					} else if (ty < 8) {
						tiledata = 0x8000 + 16 * (sprite.tile_index & 0xFE);
					} else {
						ty -= 8;
						tiledata = 0x8000 + 16 * (sprite.tile_index | 0x01);
					}

					bg_priority = sprite.flags & BG_PRIORITY;
					palette_number = sprite.flags & PALETTE_NUMBER;
					sprite_colour = get_colour_at(tiledata, tx, ty);

					break;
				}
			}	
		}

		if (bg_colour < 0 && sprite_colour < 0) {
			continue;	
		}

		if (sprite_colour > 0 && (!bg_priority || bg_colour == 0)) {
			set_pixel(x, y, sprite_colour, (palette_number) ? direct_read(ROBP1) : direct_read(ROBP0));
		} else {
			set_pixel(x, y, bg_colour, direct_read(RBGP));
		}

	}
	if (window_drawn) {
		window_line++;
	}
}

int PPU::get_colour_at(addr_t tiledata, byte_t x, byte_t y)
{
	byte_t low = read(tiledata + 2*y);
	byte_t high = read(tiledata + 2*y + 1);

	return (high >> (7-x) & 1)*2 + (low >> (7-x) & 1);
}

void PPU::set_pixel(byte_t x, byte_t y, byte_t colour_index, byte_t palette)
{
	int colour = palette >> (2*colour_index) & 0x3;
	framebuffer.at(y*160 + x) = REAL_COLOURS[colour];
}


void PPU::do_oam_scan()
{
	scanline_sprites.clear();

	for (addr_t i = 0xFE00; i < 0xFE00 + 160; i += 4) {
		if (scanline_sprites.size() >= 10) {
			break;
		}

		byte_t y = read(i);
		byte_t x = read(i+1);
		byte_t tile_index = read(i+2);
		byte_t flags = read(i+3);

		int sprite_size;
		if (get_lcdc_flag(OBJ_SIZE)) {
			sprite_size = 16;
		} else {
			sprite_size = 8;
		}
		int ly = LY();

		if (y - 16 <= ly && ly < y - 16 + sprite_size && x > 0) {
			Sprite sprite = {y, x, tile_index, flags};
			scanline_sprites.push_back(sprite);
		}
	}

	std::stable_sort(scanline_sprites.begin(), scanline_sprites.end(), [](const Sprite &a, const Sprite &b) -> bool {
			return a.x < b.x;
			});
}

enum ppu_mode PPU::get_mode()
{
	return static_cast<enum ppu_mode>(direct_read(RSTAT) & 0x3);
}

void PPU::set_mode(enum ppu_mode mode)
{
	direct_write(RSTAT, (direct_read(RSTAT) & ~0x3) + mode);
}

byte_t PPU::LY()
{
	return direct_read(RLY);
}

void PPU::inc_LY()
{
	byte_t x = LY() + 1;
	if (x > 153) {
		x = 0;
	}
	direct_write(RLY, x);
}

byte_t PPU::LCDC()
{
	return direct_read(RLCDC);
}

byte_t PPU::STAT()
{
	return direct_read(RSTAT);
}

bool PPU::get_lcdc_flag(enum lcdc_flag flag)
{
	return LCDC() & flag;
}

byte_t PPU::read(addr_t addr)
{
	return memory->read(addr);
}

void PPU::write(addr_t addr, byte_t data)
{
	memory->write(addr, data);
}

byte_t PPU::direct_read(addr_t addr)
{
	return memory->direct_read(addr);
}

void PPU::direct_write(addr_t addr, byte_t data)
{
	memory->direct_write(addr, data);
}
