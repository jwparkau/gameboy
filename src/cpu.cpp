#include "cpu.h"

#include "gameboy.h"
#include "memory.h"

#include <iostream>
#include <stdexcept>

CPU::CPU(GameBoy *gameboy) :
	gameboy(gameboy)
{
}

void CPU::dump_state()
{
}

void CPU::set_fake_boot_dmg_state()
{
}

void CPU::tick_mcycle()
{
	gameboy->tick_mcycle_no_cpu();
}

int CPU::step_instruction()
{
	int cycles = 0;
	int r1, r2, rr, r, f, n;
	byte_t opcode;

	byte_t int_enable = fast_read(RIE) & 0x1F;
	byte_t int_flag = fast_read(RIF) & 0x1F;

	if (first) {
		opcode = fast_read(PC);
		first = false;
	} else {
		opcode = read(PC);
	}

	int inter = int_enable & int_flag;

	if (halted) {
		if (IME) {
			if (!inter) {
				return 4;
			}
		} else {
			if (!inter) {
				return 4;
			} else {
			}
		}
	}

	if (inter) {
		halted = false;
	}

	if (IME && inter) {
		tick_mcycle();
		tick_mcycle();

		write(--SP, MSB(PC));
		write(--SP, LSB(PC));

		if (inter & INT_VBLANK) {
			PC = 0x40;
			int_flag &= ~(INT_VBLANK);
		} else if (inter & INT_STAT) {
			PC = 0x48;
			int_flag &= ~(INT_STAT);
		} else if (inter & INT_TIMER) {
			PC = 0x50;
			int_flag &= ~(INT_TIMER);
		} else if (inter & INT_SERIAL) {
			PC = 0x58;
			int_flag &= ~(INT_SERIAL);
		} else if (inter & INT_JOYPAD) {
			PC = 0x60;
			int_flag &= ~(INT_JOYPAD);
		} else {
			throw std::runtime_error("ERROR in handling interrupt: no interrupt to handle");
		}

		fast_write(RIF, int_flag & 0xFF);
		IME = false;
		tick_mcycle();

		opcode = read(PC);

		cycles += 20;
	}

	if (ei_delay) {
		ei_delay = false;
		IME = true;
	}

	dump_state();
	PC++;

	switch (opcode) {
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
			r1 = (opcode >> 3) & 0x7;
			r2 = opcode & 0x7;
			cycles += op_ld_rr(r1, r2);
			break;

		case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x3E:
			r = (opcode >> 3) & 0x7;
			cycles += op_ld_rn(r);
			break;

		case 0x46: case 0x4E: case 0x56: case 0x5E: case 0x66: case 0x6E: case 0x7E:
			r = (opcode >> 3) & 0x7;
			cycles += op_ld_rhl(r);
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
			r = opcode & 0x7;
			cycles += op_ld_hlr(r);
			break;

		case 0x36:
			cycles += op_ld_hln();
			break;

		case 0x0A:
			cycles += op_ld_abc();
			break;

		case 0x1A:
			cycles += op_ld_ade();
			break;

		case 0xFA:
			cycles += op_ld_ann();
			break;

		case 0x02:
			cycles += op_ld_bca();
			break;

		case 0x12:
			cycles += op_ld_dea();
			break;

		case 0xEA:
			cycles += op_ld_nna();
			break;

		case 0xF0:
			cycles += op_ld_aion();
			break;

		case 0xE0:
			cycles += op_ld_iona();
			break;

		case 0xF2:
			cycles += op_ld_aioc();
			break;

		case 0xE2:
			cycles += op_ld_ioca();
			break;

		case 0x22:
			cycles += op_ldi_hla();
			break;

		case 0x2A:
			cycles += op_ldi_ahl();
			break;

		case 0x32:
			cycles += op_ldd_hla();
			break;

		case 0x3A:
			cycles += op_ldd_ahl();
			break;

		case 0x01: case 0x11: case 0x21:
			rr = (opcode >> 4) * 2;
			cycles += op_ld_rrnn(rr);
			break;
		case 0x31:
			cycles += op_ld_spnn();
			break;

		case 0x08:
			cycles += op_ld_nnsp();
			break;

		case 0xF9:
			cycles += op_ld_sphl();
			break;

		case 0xC5: case 0xD5: case 0xE5:
			rr = ((opcode >> 4) & 0x3) * 2;
			cycles += op_push_rr(rr);
			break;
		case 0xF5:
			cycles += op_push_rr(RAF);
			break;

		case 0xC1: case 0xD1: case 0xE1:
			rr = ((opcode >> 4) & 0x3) * 2;
			cycles += op_pop_rr(rr);
			break;
		case 0xF1:
			cycles += op_pop_rr(RAF);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
			r = opcode & 0x7;
			cycles += op_add_ar(r);
			break;

		case 0xC6:
			cycles += op_add_an();
			break;

		case 0x86:
			cycles += op_add_ahl();
			break;

		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
			r = opcode & 0x7;
			cycles += op_adc_ar(r);
			break;

		case 0xCE:
			cycles += op_adc_an();
			break;
		
		case 0x8E:
			cycles += op_adc_ahl();
			break;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
			r = opcode & 0x7;
			cycles += op_sub_r(r);
			break;

		case 0xD6:
			cycles += op_sub_n();
			break;

		case 0x96:
			cycles += op_sub_hl();
			break;

		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
			r = opcode & 0x7;
			cycles += op_sbc_r(r);
			break;

		case 0xDE:
			cycles += op_sbc_n();
			break;
		
		case 0x9E:
			cycles += op_sbc_hl();
			break;

		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
			r = opcode & 0x7;
			cycles += op_and_r(r);
			break;

		case 0xE6:
			cycles += op_and_n();
			break;

		case 0xA6:
			cycles += op_and_hl();
			break;

		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
			r = opcode & 0x7;
			cycles += op_xor_r(r);
			break;

		case 0xEE:
			cycles += op_xor_n();
			break;
		
		case 0xAE:
			cycles += op_xor_hl();
			break;

		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
			r = opcode & 0x7;
			cycles += op_or_r(r);
			break;

		case 0xF6:
			cycles += op_or_n();
			break;

		case 0xB6:
			cycles += op_or_hl();
			break;

		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
			r = opcode & 0x7;
			cycles += op_cp_r(r);
			break;

		case 0xFE:
			cycles += op_cp_n();
			break;
		
		case 0xBE:
			cycles += op_cp_hl();
			break;

		case 0x04: case 0x14: case 0x24: case 0x0C: case 0x1C: case 0x2C: case 0x3C:
			r = (opcode >> 3) & 0x7;
			cycles += op_inc_r(r);
			break;
		
		case 0x34:
			cycles += op_inc_hl();
			break;

		case 0x05: case 0x15: case 0x25: case 0x0D: case 0x1D: case 0x2D: case 0x3D:
			r = (opcode >> 3) & 0x7;
			cycles += op_dec_r(r);
			break;
		
		case 0x35:
			cycles += op_dec_hl();
			break;

		case 0x27:
			cycles += op_daa();
			break;

		case 0x2F:
			cycles += op_cpl();
			break;

		case 0x09: case 0x19: case 0x29:
			rr = (opcode >> 4) * 2;
			cycles += op_add_hlrr(rr);
			break;
		case 0x39:
			cycles += op_add_hlsp();
			break;

		case 0x03: case 0x13: case 0x23:
			rr = (opcode >> 4) * 2;
			cycles += op_inc_rr(rr);
			break;
		case 0x33:
			cycles += op_inc_sp();
			break;

		case 0x0B: case 0x1B: case 0x2B:
			rr = (opcode >> 4) * 2;
			cycles += op_dec_rr(rr);
			break;
		case 0x3B:
			cycles += op_dec_sp();
			break;

		case 0xE8:
			cycles += op_add_spdd();
			break;

		case 0xF8:
			cycles += op_ld_hlspdd();
			break;

		case 0x07:
			cycles += op_rlca();
			break;

		case 0x17:
			cycles += op_rla();
			break;
		
		case 0x0F:
			cycles += op_rrca();
			break;
		
		case 0x1F:
			cycles += op_rra();
			break;

		case 0xCB:
			cycles += cb_instr();
			break;

		case 0x3F:
			cycles += op_ccf();
			break;

		case 0x37:
			cycles += op_scf();
			break;

		case 0x00:
			cycles += op_nop();
			break;

		case 0x76:
			cycles += op_halt();
			break;
		
		case 0x10:
			cycles += op_stop();
			break;

		case 0xF3:
			cycles += op_di();
			break;

		case 0xFB:
			cycles += op_ei();
			break;

		case 0xC3:
			cycles += op_jp_nn();
			break;

		case 0xE9:
			cycles += op_jp_hl();
			break;
		
		case 0xC2: case 0xCA: case 0xD2: case 0xDA:
			f = (opcode >> 3) & 0x3;
			cycles += op_jp_f_nn(f);
			break;

		case 0x18:
			cycles += op_jr_pcdd();
			break;

		case 0x20: case 0x28: case 0x30: case 0x38:
			f = (opcode >> 3) & 0x3;
			cycles += op_jr_f_pcdd(f);
			break;

		case 0xCD:
			cycles += op_call_nn();
			break;
		
		case 0xC4: case 0xCC: case 0xD4: case 0xDC:
			f = (opcode >> 3) & 0xf;
			cycles += op_call_f_nn(f);
			break;

		case 0xC9:
			cycles += op_ret();
			break;

		case 0xC0: case 0xC8: case 0xD0: case 0xD8:
			f = (opcode >> 3) & 0x3;
			cycles += op_ret_f(f);
			break;

		case 0xD9:
			cycles += op_reti();
			break;

		case 0xC7: case 0xCF: case 0xD7: case 0xDF: case 0xE7: case 0xEF: case 0xF7: case 0xFF:
			n = opcode & 0x38;
			cycles += op_rst_n(n);
			break;


	}

	return cycles;
}

int CPU::cb_instr()
{
	int cycles = 0;
	int r, n;

	byte_t opcode = read_next();

	switch (opcode) {
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07:
			r = opcode & 0x7;
			cycles += op_rlc_r(r);
			break;
		
		case 0x06:
			cycles += op_rlc_hl();
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17:
			r = opcode & 0x7;
			cycles += op_rl_r(r);
			break;
		
		case 0x16:
			cycles += op_rl_hl();
			break;

		case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0F:
			r = opcode & 0x7;
			cycles += op_rrc_r(r);
			break;

		case 0x0E:
			cycles += op_rrc_hl();
			break;

		case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1F:
			r = opcode & 0x7;
			cycles += op_rr_r(r);
			break;

		case 0x1E:
			cycles += op_rr_hl();
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x27:
			r = opcode & 0x7;
			cycles += op_sla_r(r);
			break;
		
		case 0x26:
			cycles += op_sla_hl();
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x37:
			r = opcode & 0x7;
			cycles += op_swap_r(r);
			break;
		
		case 0x36:
			cycles += op_swap_hl();
			break;

		case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2F:
			r = opcode & 0x7;
			cycles += op_sra_r(r);
			break;

		case 0x2E:
			cycles += op_sra_hl();
			break;

		case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3F:
			r = opcode & 0x7;
			cycles += op_srl_r(r);
			break;

		case 0x3E:
			cycles += op_srl_hl();
			break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
			n = (opcode >> 3) & 0x7;
			r = opcode & 0x7;
			cycles += op_bit_n_r(n, r);
			break;
		case 0x46: case 0x56: case 0x66: case 0x76: case 0x4E: case 0x5E: case 0x6E: case 0x7E:
			n = (opcode >> 3) & 0x7;
			cycles += op_bit_n_hl(n);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
			n = (opcode >> 3) & 0x7;
			r = opcode & 0x7;
			cycles += op_reset_n_r(n, r);
			break;
		case 0x86: case 0x96: case 0xA6: case 0xB6: case 0x8E: case 0x9E: case 0xAE: case 0xBE:
			n = (opcode >> 3) & 0x7;
			cycles += op_reset_n_hl(n);
			break;

		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCF:
		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD7:
		case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDF:
		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEF:
		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFF:
			n = (opcode >> 3) & 0x7;
			r = opcode & 0x7;
			cycles += op_set_n_r(n, r);
			break;
		case 0xC6: case 0xD6: case 0xE6: case 0xF6: case 0xCE: case 0xDE: case 0xEE: case 0xFE:
			n = (opcode >> 3) & 0x7;
			cycles += op_set_n_hl(n);
			break;
	}

	return cycles;
}

int CPU::op_ld_rr(int r1, int r2)
{
	REGISTER[r1] = REGISTER[r2];
	return 4;
}

int CPU::op_ld_rn(int r)
{
	REGISTER[r] = read_next();
	return 8;
}

int CPU::op_ld_rhl(int r)
{
	REGISTER[r] = read(reg_read16(RHL));
	return 8;
}

int CPU::op_ld_hlr(int r)
{
	write(reg_read16(RHL), REGISTER[r]);
	return 8;
}

int CPU::op_ld_hln()
{
	write(reg_read16(RHL), read_next());
	return 12;
}

int CPU::op_ld_abc()
{
	REGISTER[RA] = read(reg_read16(RBC));
	return 8;
}

int CPU::op_ld_ade()
{
	REGISTER[RA] = read(reg_read16(RDE));
	return 8;
}

int CPU::op_ld_ann()
{
	REGISTER[RA] = read(read16_next());
	return 16;
}

int CPU::op_ld_bca()
{
	write(reg_read16(RBC), REGISTER[RA]);
	return 8;
}

int CPU::op_ld_dea()
{
	write(reg_read16(RDE), REGISTER[RA]);
	return 8;
}

int CPU::op_ld_nna()
{
	write(read16_next(), REGISTER[RA]);
	return 16;
}

int CPU::op_ld_aion()
{
	REGISTER[RA] = read(0xFF00 + read_next());
	return 12;
}

int CPU::op_ld_iona()
{
	write(0xFF00 + read_next(), REGISTER[RA]);
	return 12;
}

int CPU::op_ld_aioc()
{
	REGISTER[RA] = read(0xFF00 + REGISTER[RC]);
	return 8;
}

int CPU::op_ld_ioca()
{
	write(0xFF00 + REGISTER[RC], REGISTER[RA]);
	return 8;
}

int CPU::op_ldi_hla()
{
	write(reg_read16(RHL), REGISTER[RA]);
	reg_write16(RHL, reg_read16(RHL)+1);
	return 8;
}

int CPU::op_ldi_ahl()
{
	REGISTER[RA] = read(reg_read16(RHL));
	reg_write16(RHL, reg_read16(RHL)+1);
	return 8;
}

int CPU::op_ldd_hla()
{
	write(reg_read16(RHL), REGISTER[RA]);
	reg_write16(RHL, reg_read16(RHL)-1);
	return 8;
}

int CPU::op_ldd_ahl()
{
	REGISTER[RA] = read(reg_read16(RHL));
	reg_write16(RHL, reg_read16(RHL)-1);
	return 8;
}

int CPU::op_ld_rrnn(int rr)
{
	reg_write16(rr, read16_next());
	return 12;
}

int CPU::op_ld_spnn()
{
	SP = read16_next();
	return 12;
}

int CPU::op_ld_nnsp()
{
	write16(read16_next(), SP);
	return 20;
}

int CPU::op_ld_sphl()
{
	SP = reg_read16(RHL);
	tick_mcycle();
	return 8;
}

int CPU::op_push_rr(int rr)
{
	tick_mcycle();
	write(--SP, MSB(reg_read16(rr)));
	write(--SP, LSB(reg_read16(rr)));
	return 16;
}

int CPU::op_pop_rr(int rr)
{
	int lo = read(SP++);
	int hi = read(SP++);
	uint16_t result = (hi << 8) + lo;
	reg_write16(rr, result);
	return 12;
}

void CPU::op_add(int y)
{
	int x = REGISTER[RA];
	int result = x + y;
	int half = (x & 0xF) + (y & 0xF);
	REGISTER[RA] = LSB(result);

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(false);
	set_flag_h(half & 0x10);
	set_flag_c(result & 0x100);
}

int CPU::op_add_ar(int r)
{
	op_add(REGISTER[r]);
	return 4;
}

int CPU::op_add_an()
{
	op_add(read_next());
	return 8;
}

int CPU::op_add_ahl()
{
	op_add(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_adc(int y)
{
	int x = REGISTER[RA];
	int result = x + y + cy();
	int half = (x & 0xF) + (y & 0xF) + cy();
	REGISTER[RA] = LSB(result);

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(false);
	set_flag_h(half & 0x10);
	set_flag_c(result & 0x100);
}

int CPU::op_adc_ar(int r)
{
	op_adc(REGISTER[r]);
	return 4;
}

int CPU::op_adc_an()
{
	op_adc(read_next());
	return 8;
}

int CPU::op_adc_ahl()
{
	op_adc(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_sub(int y)
{
	int x = REGISTER[RA];
	int result = unsign(x - y);
	REGISTER[RA] = result & 0xFF;

	set_flag_z(x == y);
	set_flag_n(true);
	set_flag_h((x & 0xF) < (y & 0xF));
	set_flag_c(x < y);
}

int CPU::op_sub_r(int r)
{
	op_sub(REGISTER[r]);
	return 4;
}

int CPU::op_sub_n()
{
	op_sub(read_next());
	return 8;
}

int CPU::op_sub_hl()
{
	op_sub(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_sbc(int y)
{
	int x = REGISTER[RA];
	int result = unsign(x - y - cy());
	REGISTER[RA] = result & 0xFF;

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(true);
	set_flag_h((x & 0xF) - (y & 0xF) - cy() < 0);
	set_flag_c(x - y - cy() < 0);
}

int CPU::op_sbc_r(int r)
{
	op_sbc(REGISTER[r]);
	return 4;
}

int CPU::op_sbc_n()
{
	op_sbc(read_next());
	return 8;
}

int CPU::op_sbc_hl()
{
	op_sbc(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_and(int y)
{
	REGISTER[RA] &= y;

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(false);
	set_flag_h(true);
	set_flag_c(false);
}

int CPU::op_and_r(int r)
{
	op_and(REGISTER[r]);
	return 4;
}

int CPU::op_and_n()
{
	op_and(read_next());
	return 8;
}

int CPU::op_and_hl()
{
	op_and(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_xor(int y)
{
	REGISTER[RA] ^= y;

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(false);
}

int CPU::op_xor_r(int r)
{
	op_xor(REGISTER[r]);
	return 4;
}

int CPU::op_xor_n()
{
	op_xor(read_next());
	return 8;
}

int CPU::op_xor_hl()
{
	op_xor(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_or(int y)
{
	REGISTER[RA] |= y;

	set_flag_z(REGISTER[RA] == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(false);
}

int CPU::op_or_r(int r)
{
	op_or(REGISTER[r]);
	return 4;
}

int CPU::op_or_n()
{
	op_or(read_next());
	return 8;
}

int CPU::op_or_hl()
{
	op_or(read(reg_read16(RHL)));
	return 8;
}

void CPU::op_cp(int y)
{
	int x = REGISTER[RA];

	set_flag_z(x == y);
	set_flag_n(true);
	set_flag_h((x & 0xF) < (y & 0xF));
	set_flag_c(x < y);
}

int CPU::op_cp_r(int r)
{
	op_cp(REGISTER[r]);
	return 4;
}

int CPU::op_cp_n()
{
	op_cp(read_next());
	return 8;
}

int CPU::op_cp_hl()
{
	op_cp(read(reg_read16(RHL)));
	return 8;
}

int CPU::op_inc_r(int r)
{
	int x = REGISTER[r];
	int y = 1;

	int result = x + y;
	int half = (x & 0xF) + y;
	REGISTER[r] = result & 0xFF;

	set_flag_z(REGISTER[r] == 0);
	set_flag_n(false);
	set_flag_h(half & 0x10);

	return 4;
}

int CPU::op_inc_hl()
{
	addr_t addr = reg_read16(RHL);
	int x = read(addr);
	int y = 1;

	int result_byte = (x + y) & 0xFF;
	int half = (x & 0xF) + y;

	write(addr, result_byte);

	set_flag_z(result_byte == 0);
	set_flag_n(false);
	set_flag_h(half & 0x10);

	return 12;
}

int CPU::op_dec_r(int r)
{
	int x = REGISTER[r];
	int y = 1;
	int result = unsign(x - y);
	REGISTER[r] = result;

	set_flag_z(x == y);
	set_flag_n(true);
	set_flag_h((x & 0xF) < y);

	return 4;
}

int CPU::op_dec_hl()
{
	addr_t addr = reg_read16(RHL);
	int x = read(addr);
	int y = 1;

	int result = unsign(x - y);

	write(addr, result);

	set_flag_z(x == y);
	set_flag_n(true);
	set_flag_h((x & 0xF) < y);

	return 12;
}

int CPU::op_daa()
{
	int a = REGISTER[RA];

	if (!fn()) {
		if (fh() || (a & 0xF) > 0x9) {
			a += 0x6;
		}
		if (cy() || a > 0x9F) {
			a += 0x60;
		}
	} else {
		if (fh()) {
			a -= 0x6;
			if (!cy()) {
				a = unsign(a);
			}
		}
		if (cy()) {
			a -= 0x60;
		}
	}

	if (a & 0x100) {
		set_flag_c(true);
	}

	REGISTER[RA] = unsign(a);
	set_flag_z(REGISTER[RA] == 0);
	set_flag_h(false);

	return 4;
}

int CPU::op_cpl()
{
	REGISTER[RA] ^= 0xFF;

	set_flag_n(true);
	set_flag_h(true);

	return 4;
}

void CPU::op_add_hl(int y)
{
	int x = reg_read16(RHL);

	int result = x + y;
	int half = (x & 0xFFF) + (y & 0xFFF);

	reg_write16(RHL, result & 0xFFFF);

	set_flag_n(false);
	set_flag_h(half & 0x1000);
	set_flag_c(result & 0x10000);
}

int CPU::op_add_hlrr(int rr)
{
	op_add_hl(reg_read16(rr));
	tick_mcycle();
	return 8;
}

int CPU::op_add_hlsp()
{
	op_add_hl(SP);
	tick_mcycle();
	return 8;
}

int CPU::op_inc_rr(int rr)
{
	reg_write16(rr, reg_read16(rr) + 1);
	tick_mcycle();
	return 8;
}

int CPU::op_inc_sp()
{
	SP++;
	tick_mcycle();
	return 8;
}

int CPU::op_dec_rr(int rr)
{
	reg_write16(rr, reg_read16(rr) - 1);
	tick_mcycle();
	return 8;
}

int CPU::op_dec_sp()
{
	SP--;
	tick_mcycle();
	return 8;
}

int CPU::op_add_spdd()
{
	int x = SP;
	int y = unsign16(sign(read_next()));

	int result = x + y;
	int carry = (x & 0xFF) + (y & 0xFF);
	int half = (x & 0xF) + (y & 0xF);

	SP = result & 0xFFFF;

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(half & 0x10);
	set_flag_c(carry & 0x100);

	tick_mcycle();
	tick_mcycle();

	return 16;
}

int CPU::op_ld_hlspdd()
{
	int x = SP;
	int y = unsign16(sign(read_next()));

	int result = x + y;
	int carry = (x & 0xFF) + (y & 0xFF);
	int half = (x & 0xF) + (y & 0xF);

	reg_write16(RHL, result & 0xFFFF);

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(half & 0x10);
	set_flag_c(carry & 0x100);

	tick_mcycle();

	return 12;
}

byte_t CPU::op_rlc(byte_t x)
{
	bool discarded = (x & 0x80);

	x <<= 1;
	if (discarded) {
		x |= 0x1;
	}

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

byte_t CPU::op_rl(byte_t x)
{
	bool discarded = (x & 0x80);

	x <<= 1;
	if (cy()) {
		x |= 0x1;
	}

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

byte_t CPU::op_rrc(byte_t x)
{
	bool discarded = (x & 0x1);

	x >>= 1;
	if (discarded) {
		x |= 0x80;
	}

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

byte_t CPU::op_rr(byte_t x)
{
	bool discarded = (x & 0x1);

	x >>= 1;
	if (cy()) {
		x |= 0x80;
	}

	set_flag_z(false);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

int CPU::op_rlca()
{
	REGISTER[RA] = op_rlc(REGISTER[RA]);
	return 4;
}

int CPU::op_rla()
{
	REGISTER[RA] = op_rl(REGISTER[RA]);
	return 4;
}

int CPU::op_rrca()
{
	REGISTER[RA] = op_rrc(REGISTER[RA]);
	return 4;
}

int CPU::op_rra()
{
	REGISTER[RA] = op_rr(REGISTER[RA]);
	return 4;
}

int CPU::op_rlc_r(int r)
{
	REGISTER[r] = op_rlc(REGISTER[r]);
	set_flag_z(REGISTER[r] == 0);
	return 8;
}

int CPU::op_rlc_hl()
{
	addr_t addr = reg_read16(RHL);
	byte_t result = op_rlc(read(addr));
	write(addr, result);
	set_flag_z(result == 0);
	return 16;
}

int CPU::op_rl_r(int r)
{
	REGISTER[r] = op_rl(REGISTER[r]);
	set_flag_z(REGISTER[r] == 0);
	return 8;
}

int CPU::op_rl_hl()
{
	addr_t addr = reg_read16(RHL);
	byte_t result = op_rl(read(addr));
	write(addr, result);
	set_flag_z(result == 0);
	return 16;
}

int CPU::op_rrc_r(int r)
{
	REGISTER[r] = op_rrc(REGISTER[r]);
	set_flag_z(REGISTER[r] == 0);
	return 8;
}

int CPU::op_rrc_hl()
{
	addr_t addr = reg_read16(RHL);
	byte_t result = op_rrc(read(addr));
	write(addr, result);
	set_flag_z(result == 0);
	return 16;
}

int CPU::op_rr_r(int r)
{
	REGISTER[r] = op_rr(REGISTER[r]);
	set_flag_z(REGISTER[r] == 0);
	return 8;
}

int CPU::op_rr_hl()
{
	addr_t addr = reg_read16(RHL);
	byte_t result = op_rr(read(addr));
	write(addr, result);
	set_flag_z(result == 0);
	return 16;
}

byte_t CPU::op_sla(byte_t x)
{
	bool discarded = x & 0x80;

	x <<= 1;

	set_flag_z(x == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

int CPU::op_sla_r(int r)
{
	REGISTER[r] = op_sla(REGISTER[r]);
	return 8;
}

int CPU::op_sla_hl()
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_sla(read(addr)));
	return 16;
}

byte_t CPU::op_swap(byte_t x)
{
	int hi = x >> 4;
	int lo = x & 0xF;

	x = (lo << 4) + hi;

	set_flag_z(x == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(false);

	return x;
}

int CPU::op_swap_r(int r)
{
	REGISTER[r] = op_swap(REGISTER[r]);
	return 8;
}

int CPU::op_swap_hl()
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_swap(read(addr)));
	return 16;
}

byte_t CPU::op_sra(byte_t x)
{
	bool discarded = x & 0x1;
	bool b7 = x & 0x80;

	x >>= 1;

	if (b7) {
		x |= 0x80;
	}

	set_flag_z(x == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

int CPU::op_sra_r(int r)
{
	REGISTER[r] = op_sra(REGISTER[r]);
	return 8;
}

int CPU::op_sra_hl()
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_sra(read(addr)));
	return 16;
}

byte_t CPU::op_srl(byte_t x)
{
	bool discarded = x & 0x1;

	x >>= 1;

	set_flag_z(x == 0);
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(discarded);

	return x;
}

int CPU::op_srl_r(int r)
{
	REGISTER[r] = op_srl(REGISTER[r]);
	return 8;
}

int CPU::op_srl_hl()
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_srl(read(addr)));
	return 16;
}

void CPU::op_bit(int n, byte_t x)
{
	bool set = x & (1 << n);

	set_flag_z(!set);
	set_flag_n(false);
	set_flag_h(true);
}

int CPU::op_bit_n_r(int n, int r)
{
	op_bit(n, REGISTER[r]);
	return 8;
}

int CPU::op_bit_n_hl(int n)
{
	op_bit(n, read(reg_read16(RHL)));
	return 12;
}

byte_t CPU::op_setreset(int n, byte_t x, bool set)
{
	if (set) {
		x |= (1 << n);
	} else {
		x &= ~(1 << n);
	}

	return x;
}

int CPU::op_set_n_r(int n, int r)
{
	REGISTER[r] = op_setreset(n, REGISTER[r], true);
	return 8;
}

int CPU::op_set_n_hl(int n)
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_setreset(n, read(addr), true));
	return 16;
}

int CPU::op_reset_n_r(int n, int r)
{
	REGISTER[r] = op_setreset(n, REGISTER[r], false);
	return 8;
}

int CPU::op_reset_n_hl(int n)
{
	addr_t addr = reg_read16(RHL);
	write(addr, op_setreset(n, read(addr), false));
	return 16;
}

int CPU::op_ccf()
{
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(!cy());
	return 4;
}

int CPU::op_scf()
{
	set_flag_n(false);
	set_flag_h(false);
	set_flag_c(true);
	return 4;
}

int CPU::op_nop()
{
	return 4;
}

int CPU::op_halt()
{
	halted = true;
	return 4;
}

int CPU::op_stop()
{
	return 4;
}

int CPU::op_di()
{
	IME = false;
	return 4;
}

int CPU::op_ei()
{
	ei_delay = true;
	return 4;
}

int CPU::op_jp_nn()
{
	addr_t nn = read16_next();
	PC = nn;
	tick_mcycle();
	return 16;
}

int CPU::op_jp_hl()
{
	PC = reg_read16(RHL);
	return 4;
}

int CPU::op_jp_f_nn(int f)
{
	addr_t nn = read16_next();
	if (fset(f)) {
		PC = nn;
		tick_mcycle();
		return 16;
	} else {
		return 12;
	}
}

int CPU::op_jr_pcdd()
{
	int offset = unsign16(sign(read_next()));
	PC += offset;
	tick_mcycle();
	return 12;
}

int CPU::op_jr_f_pcdd(int f)
{
	int offset = unsign16(sign(read_next()));
	if (fset(f)) {
		PC += offset;
		tick_mcycle();
		return 12;
	} else {
		return 8;
	}
}

int CPU::op_call_nn()
{
	addr_t nn = read16_next();
	tick_mcycle();
	write(--SP, MSB(PC));
	write(--SP, LSB(PC));
	PC = nn;
	return 24;
}

int CPU::op_call_f_nn(int f)
{
	addr_t nn = read16_next();
	if (fset(f)) {
		tick_mcycle();
		write(--SP, MSB(PC));
		write(--SP, LSB(PC));
		PC = nn;
		return 24;
	} else {
		return 12;
	}
}

int CPU::op_ret()
{
	addr_t addr = read(SP++);
	PC = (read(SP++) << 8) + addr;
	tick_mcycle();
	return 16;
}

int CPU::op_ret_f(int f)
{
	bool set = fset(f);

	if (set) {
		tick_mcycle();
		addr_t addr = read(SP++);
		PC = (read(SP++) << 8) + addr;
	}

	tick_mcycle();
	return set ? 20 : 8;
}

int CPU::op_reti()
{
	IME = true;
	return op_ret();
}

int CPU::op_rst_n(int n)
{
	tick_mcycle();
	write(--SP, MSB(PC));
	write(--SP, LSB(PC));
	PC = n & 0xFF;
	return 16;
}

int CPU::to_16reg(int r)
{
	int reg;
	switch (r) {
		case RA: case RF:
			reg = RF;
			break;
		case RB: case RC:
			reg = RB;
			break;
		case RD: case RE:
			reg = RD;
			break;
		case RH: case RL:
			reg = RH;
			break;
		default:
			throw std::runtime_error("invalid register to_16reg");
	}

	return reg;
}

uint16_t CPU::reg_read16(int rr)
{
	//r = to_16reg(r);
	switch (rr) {
		case RAF:
			return (REGISTER[RA] << 8) + REGISTER[RF];
		case RBC: case RDE: case RHL:
			return (REGISTER[rr] << 8) + REGISTER[rr+1];
		default:
			throw std::runtime_error("invalid register reg_read16");
	}
}

void CPU::reg_write16(int rr, uint16_t data)
{
	switch (rr) {
		case RAF:
			REGISTER[RA] = MSB(data);
			REGISTER[RF] = LSB(data) & 0xF0;
			break;
		case RBC: case RDE: case RHL:
			REGISTER[rr] = MSB(data);
			REGISTER[rr+1] = LSB(data);
			break;
		default:
			throw std::runtime_error("invalid register reg_write16");
	}
}

byte_t CPU::fast_read(addr_t addr)
{
	return gameboy->memory->read(addr);
}

void CPU::fast_write(addr_t addr, byte_t data)
{
	gameboy->memory->write(addr, data);
}

byte_t CPU::read(addr_t addr)
{
	tick_mcycle();
	return fast_read(addr);
}

uint16_t CPU::read16(addr_t addr)
{
	return (read(addr+1) << 8) + read(addr);
}

byte_t CPU::read_next()
{
	return read(PC++);
}

uint16_t CPU::read16_next()
{
	byte_t result = read_next();
	return (read_next() << 8) + result;
}

void CPU::write(addr_t addr, byte_t data)
{
	tick_mcycle();
	fast_write(addr, data);
}

void CPU::write16(addr_t addr, uint16_t data)
{
	write(addr, LSB(data));
	write(addr+1, MSB(data));
}


byte_t CPU::MSB(uint16_t data)
{
	return data >> 8;
}

byte_t CPU::LSB(uint16_t data)
{
	return data & 0xFF;
}

void CPU::set_flag_z(bool x)
{
	set_flag(x, 7);
}

void CPU::set_flag_n(bool x)
{
	set_flag(x, 6);
}

void CPU::set_flag_h(bool x)
{
	set_flag(x, 5);
}

void CPU::set_flag_c(bool x)
{
	set_flag(x, 4);
}

void CPU::set_flag(bool x, int bit)
{
	if (x) {
		REGISTER[RF] |= 1 << bit;
	} else {
		REGISTER[RF] &= ~(1 << bit);
	}
}

bool CPU::fz()
{
	return ((REGISTER[RF] & 0x80) != 0);
}

bool CPU::fn()
{
	return ((REGISTER[RF] & 0x40) != 0);
}

bool CPU::fh()
{
	return ((REGISTER[RF] & 0x20) != 0);
}

bool CPU::cy()
{
	return ((REGISTER[RF] & 0x10) != 0);
}

bool CPU::fset(int f)
{
	f &= 0x3;

	switch (f) {
		case 0:
			return !fz();
		case 1:
			return fz();
		case 2:
			return !cy();
		case 3:
			return cy();
		default:
			throw std::runtime_error("fset error somehow");
	}
}
