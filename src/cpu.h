#ifndef GB_CPU_H
#define GB_CPU_H

#include <cstdint>
#include <array>

#include "types.h"

enum cpu_reg {
	RB = 0,
	RC = 1,
	RD = 2,
	RE = 3,
	RH = 4,
	RL = 5,
	RF = 6,
	RA = 7,
	
	RBC = RB,
	RDE = RD,
	RHL = RH,
	RAF = RF
};

class GameBoy;

class CPU {
	public:
		void dump_state();

		std::array<byte_t, 8> REGISTER {};
		std::uint16_t SP = 0;
		std::uint16_t PC = 0;
		bool IME = true;
		bool ei_delay = false;
		bool halted = false;
		bool first = true;

		GameBoy *gameboy = nullptr;

		CPU(GameBoy *gameboy);

		void set_fake_boot_dmg_state();

		int step_instruction();
		int cb_instr();
		void tick_mcycle();

		int to_16reg(int r);
		uint16_t reg_read16(int rr);
		void reg_write16(int rr, uint16_t data);

		byte_t fast_read(addr_t addr);
		void fast_write(addr_t addr, byte_t data);
		byte_t read(addr_t addr);
		uint16_t read16(addr_t addr);
		void write(addr_t addr, byte_t data);
		void write16(addr_t addr, uint16_t data);
		byte_t read_next();
		uint16_t read16_next();

		byte_t MSB(uint16_t data);
		byte_t LSB(uint16_t data);

		void set_flag_z(bool x);
		void set_flag_n(bool x);
		void set_flag_h(bool x);
		void set_flag_c(bool x);
		void set_flag(bool x, int bit);

		bool fz();
		bool fn();
		bool fh();
		bool cy();

		int op_ld_rr(int r1, int r2);
		int op_ld_rn(int r);
		int op_ld_rhl(int r);
		int op_ld_hlr(int r);
		int op_ld_hln();
		int op_ld_abc();
		int op_ld_ade();
		int op_ld_ann();
		int op_ld_bca();
		int op_ld_dea();
		int op_ld_nna();
		int op_ld_aion();
		int op_ld_iona();
		int op_ld_aioc();
		int op_ld_ioca();
		int op_ldi_hla();
		int op_ldi_ahl();
		int op_ldd_hla();
		int op_ldd_ahl();

		int op_ld_rrnn(int rr);
		int op_ld_spnn();
		int op_ld_nnsp();
		int op_ld_sphl();
		int op_push_rr(int rr);
		int op_pop_rr(int rr);

		void op_add(int y);
		int op_add_ar(int r);
		int op_add_an();
		int op_add_ahl();
		void op_adc(int y);
		int op_adc_ar(int r);
		int op_adc_an();
		int op_adc_ahl();
		void op_sub(int y);
		int op_sub_r(int r);
		int op_sub_n();
		int op_sub_hl();
		void op_sbc(int y);
		int op_sbc_r(int r);
		int op_sbc_n();
		int op_sbc_hl();
		void op_and(int y);
		int op_and_r(int r);
		int op_and_n();
		int op_and_hl();
		void op_xor(int y);
		int op_xor_r(int r);
		int op_xor_n();
		int op_xor_hl();
		void op_or(int y);
		int op_or_r(int r);
		int op_or_n();
		int op_or_hl();
		void op_cp(int y);
		int op_cp_r(int r);
		int op_cp_n();
		int op_cp_hl();
		int op_inc_r(int r);
		int op_inc_hl();
		int op_dec_r(int r);
		int op_dec_hl();
		int op_daa();
		int op_cpl();

		void op_add_hl(int y);
		int op_add_hlrr(int rr);
		int op_add_hlsp();
		int op_inc_rr(int rr);
		int op_inc_sp();
		int op_dec_rr(int rr);
		int op_dec_sp();
		int op_add_spdd();
		int op_ld_hlspdd();

		byte_t op_rlc(byte_t x);
		byte_t op_rl(byte_t x);
		byte_t op_rrc(byte_t x);
		byte_t op_rr(byte_t x);
		int op_rlca();
		int op_rla();
		int op_rrca();
		int op_rra();
		int op_rlc_r(int r);
		int op_rlc_hl();
		int op_rl_r(int r);
		int op_rl_hl();
		int op_rrc_r(int r);
		int op_rrc_hl();
		int op_rr_r(int r);
		int op_rr_hl();
		byte_t op_sla(byte_t x);
		int op_sla_r(int r);
		int op_sla_hl();
		byte_t op_swap(byte_t x);
		int op_swap_r(int r);
		int op_swap_hl();
		byte_t op_sra(byte_t x);
		int op_sra_r(int r);
		int op_sra_hl();
		byte_t op_srl(byte_t x);
		int op_srl_r(int r);
		int op_srl_hl();

		void op_bit(int n, byte_t x);
		int op_bit_n_r(int n, int r);
		int op_bit_n_hl(int n);
		byte_t op_setreset(int n, byte_t x, bool set);
		int op_set_n_r(int n, int r);
		int op_set_n_hl(int n);
		int op_reset_n_r(int n, int r);
		int op_reset_n_hl(int n);

		int op_ccf();
		int op_scf();
		int op_nop();
		int op_halt();
		int op_stop();
		int op_di();
		int op_ei();

		bool fset(int f);
		int op_jp_nn();
		int op_jp_hl();
		int op_jp_f_nn(int f);
		int op_jr_pcdd();
		int op_jr_f_pcdd(int f);
		int op_call_nn();
		int op_call_f_nn(int f);
		int op_ret();
		int op_ret_f(int f);
		int op_reti();
		int op_rst_n(int n);
};

#endif
