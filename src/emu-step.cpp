#include "../inc/emu-step.hpp"
#include "../inc/common.hpp"
#include "../inc/emulator.hpp"

/********************************************************************/
// Aux
/********************************************************************/
void push(word_t val) {
	ctx.reg[SP_CODE] -= 4;
	mem32set(ctx.reg[SP_CODE], val);
}

word_t pop() {
	ctx.reg[SP_CODE] += 4;
	return mem32get(ctx.reg[SP_CODE]);
}

/********************************************************************/
// Instruction Handlers
/********************************************************************/
void handle_halt() {

}

void handle_int(word_t cause) {
	push(ctx.reg[PC_CODE]);
	push(ctx.csreg[STATUS_CODE]);
	ctx.csreg[CAUSE_CODE] = cause;
	ctx.csreg[STATUS_CODE] = ctx.csreg[STATUS_CODE] & (~0x1);
	ctx.reg[PC_CODE] = ctx.csreg[HANDLER_CODE];
}
void handle_call(byte mod, byte a, byte b, word_t disp) {
	if (mod == 0x0) {
		push(ctx.reg[PC_CODE]);
		ctx.reg[PC_CODE] = ctx.reg[a] + ctx.reg[b] + disp;
	}
	else if (mod == 0x1) {
		push(ctx.reg[PC_CODE]);
		ctx.reg[PC_CODE] = mem32get(ctx.reg[a] + ctx.reg[b] + disp);
	}
	else {
		illegal_instr();
	}
}
void handle_jmp(byte mod, byte a, byte b, byte c, word_t disp) {
	if (mod == 0x0) {
		ctx.reg[PC_CODE] = ctx.reg[a] + disp;
	}
	else if (mod == 0x1) {
		if (ctx.reg[b] == ctx.reg[c])
			ctx.reg[PC_CODE] = ctx.reg[a] + disp;
	}
	else if (mod == 0x2) {
		if (ctx.reg[b] != ctx.reg[c])
			ctx.reg[PC_CODE] = ctx.reg[a] + disp;
	}
	else if (mod == 0x3) {
		if ((signed)ctx.reg[b] > (signed)ctx.reg[c])
			ctx.reg[PC_CODE] = ctx.reg[a] + disp;
	}
	else if (mod == 0x4) {
		ctx.reg[PC_CODE] = mem32get(ctx.reg[a] + disp);
	}
	else if (mod == 0x5) {
		if (ctx.reg[b] == ctx.reg[c])
			ctx.reg[PC_CODE] = mem32get(ctx.reg[a] + disp);
	}
	else if (mod == 0x6) {
		if (ctx.reg[b] != ctx.reg[c])
			ctx.reg[PC_CODE] = mem32get(ctx.reg[a] + disp);
	}
	else if (mod == 0x7) {
		if ((signed)ctx.reg[b] > (signed)ctx.reg[c])
			ctx.reg[PC_CODE] = mem32get(ctx.reg[a] + disp);
	}
	else
		illegal_instr();
}

void handle_xchg(byte b, byte c) {
	word_t temp = ctx.reg[b];
	if (b != 0)
		ctx.reg[b] = ctx.reg[c];
	if (c != 0)
		ctx.reg[c] = temp;
}

void handle_maths(byte mod, byte a, byte b, byte c) {
	if (a == 0)
		return;

	if (mod == 0x0)
		ctx.reg[a] = ctx.reg[b] + ctx.reg[c];
	else if (mod == 0x1)
		ctx.reg[a] = ctx.reg[b] - ctx.reg[c];
	else if (mod == 0x2)
		ctx.reg[a] = ctx.reg[b] * ctx.reg[c];
	else if (mod == 0x3)
		ctx.reg[a] = ctx.reg[c] == 0 ? 0 : ctx.reg[b] / ctx.reg[c];
	else
		illegal_instr();
}

void handle_logic(byte mod, byte a, byte b, byte c) {
	if (a == 0)
		return;

	if (mod == 0x0)
		ctx.reg[a] = ~ctx.reg[b];
	else if (mod == 0x1)
		ctx.reg[a] = ctx.reg[b] & ctx.reg[c];
	else if (mod == 0x2)
		ctx.reg[a] = ctx.reg[b] | ctx.reg[c];
	else if (mod == 0x3)
		ctx.reg[a] = ctx.reg[b] ^ ctx.reg[c];
	else
		illegal_instr();
}

void handle_shift(byte mod, byte a, byte b, byte c) {
	if (a == 0)
		return;

	if (mod == 0x0)
		ctx.reg[a] = ctx.reg[b] << ctx.reg[c];
	else if (mod == 0x1)
		ctx.reg[a] = ctx.reg[b] << ctx.reg[c];
	else
		illegal_instr();
}

void handle_st(byte mod, byte a, byte b, byte c, word_t disp) {
	if (mod == 0x0) {
		mem32set(ctx.reg[a] + ctx.reg[b] + disp, ctx.reg[c]);
	}
	else if (mod == 0x2) {
		mem32set(mem32get(ctx.reg[a] + ctx.reg[b] + disp), ctx.reg[c]);
	}
	else if (mod == 0x1) {
		if (a != 0)
			ctx.reg[a] = ctx.reg[a] + disp;
		mem32set(ctx.reg[a], ctx.reg[c]);
	}
	else {
		illegal_instr();
	}
}

void handle_ld(byte mod, byte a, byte b, byte c, word_t disp) {
	if (mod == 0x0) {
		if (a != 0)
			ctx.reg[a] = ctx.csreg[b];
	}
	else if (mod == 0x1) {
		if (a != 0)
			ctx.reg[a] = ctx.reg[b] + disp;
	}
	else if (mod == 0x2) {
		if (a != 0)
			ctx.reg[a] = mem32get(ctx.reg[b] + ctx.reg[c] + disp);
	}
	else if (mod == 0x3) {
		if (a != 0)
			ctx.reg[a] = mem32get(ctx.reg[b]);
		if (b != 0)
			ctx.reg[b] = ctx.reg[b] + disp;
	}
	else if (mod == 0x4) {
		ctx.csreg[a] = ctx.reg[b];
	}
	else if (mod == 0x5) {
		ctx.csreg[a] = ctx.csreg[b] | disp;
	}
	else if (mod == 0x6) {
		ctx.csreg[a] = mem32get(ctx.reg[b] + ctx.reg[c] + disp);
	}
	else if (mod == 0x7) {
		ctx.csreg[a] = mem32get(ctx.reg[b]);
		if (b != 0)
			ctx.reg[b] = ctx.reg[b] + disp;
	}
	else {
		illegal_instr();
	}
}
/********************************************************************/
// Implementation
/********************************************************************/

void illegal_instr() {
	handle_int(1);
}

bool step() {
	instr_t instr = mem32get(ctx.reg[PC_CODE]);

	byte oc = (instr >> 28);
	byte mod = (instr >> 24) & 0xf;
	byte a = (instr >> 20) & 0xf;
	byte b = (instr >> 16) & 0xf;
	byte c = (instr >> 12) & 0xf;
	word_t disp = instr & 0xfff;
	if (disp & 0x800)
		disp |= 0xfffff000;

	ctx.reg[PC_CODE] += sizeof(instr_t);

	switch ((opcode)oc) {
		case (OC_HALT):
			handle_halt();
			return false;
		case (OC_INT):
			handle_int(4);
			return true;
		case (OC_CALL):
			handle_call(mod, a, b, disp);
			return true;
		case (OC_JMP):
			handle_jmp(mod, a, b, c, disp);
			return true;
		case (OC_XCHG):
			handle_xchg(b, c);
			return true;
		case (OC_MATHS):
			handle_maths(mod, a, b, c);
			return true;
		case (OC_LOGIC):
			handle_logic(mod, a, b, c);
			return true;
		case (OC_SHIFT):
			handle_shift(mod, a, b, c);
			return true;
		case (OC_ST):
			handle_st(mod, a, b, c, disp);
			return true;
		case (OC_LD):
			handle_ld(mod, a, b, c, disp);
			return true;
	}
	illegal_instr();
	return true;
}

