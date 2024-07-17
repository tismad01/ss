#include "../inc/asm-instr.hpp"
#include "../inc/assembler.hpp"

/********************************************************************/
// Aux
/********************************************************************/

addr_t garbage = 0x777;
void append_instr(instr_t ocmod, instr_t regA, instr_t regB, instr_t regC, instr_t disp) {
	instr_t new_inst
		= (ocmod << 24)
		| ((regA & 0xf) << 20)
		| ((regB & 0xf) << 16)
		| ((regC & 0xf) << 12)
		| (disp & 0xfff);
	byte* bytes = reinterpret_cast<byte*>(&new_inst);
	for (size_t i = 0; i < sizeof(new_inst); ++i) {
		CURR_SEC.data.emplace_back(bytes[i]);
	}
}
void append_sym_pool(char *sym) {
	CURR_SEC.extra_syms.push_back({CURR_SEC.data.size(), sym});
}

void append_lit_pool(word_t val) {
	CURR_SEC.literals.push_back({CURR_SEC.data.size(), val});
}
void append_reloc_local(char *sym) {
	CURR_SEC.local_rels[(addr_t)CURR_SEC.data.size()] = sym;
}



/********************************************************************/
// Implementation
/********************************************************************/
void mk_halt() {
	append_instr(0x00, 0, 0, 0, 0);
}
void mk_int() {
	append_instr(0x10, 0, 0, 0, 0);
}
void mk_iret() {
	append_instr(0x97, STATUS_CODE, SP_CODE, 0, 4);
	mk_pop(PC_CODE);
}
void mk_call(operand op) {
	if (op.optype == IMM_LIT)
		append_lit_pool(op.literal);
	else // if (op.optype == IMM_SYM)
		append_sym_pool(op.symbol);
	append_instr(0x21, PC_CODE, ZERO_CODE, 0, garbage);
}

void mk_jmp(operand op) {
	if (op.optype == IMM_LIT) {
		append_instr(0x30, PC_CODE, 0, 0, op.literal);
	}
	else {// if (op.optype == IMM_SYM)
		append_reloc_local(op.symbol);
		append_instr(0x30, PC_CODE, 0, 0, garbage);
	}
	
}

void mk_branch(byte ocmod, byte regA, byte regB, operand op) {
	if (op.optype == IMM_LIT) {
		append_instr(ocmod, PC_CODE, regA, regB, op.literal);
	}
	else { // if (op.optype == IMM_SYM)
		append_reloc_local(op.symbol);
		append_instr(ocmod, PC_CODE, regA, regB, garbage);
	}

}
void mk_push(byte reg) {
	append_instr(0x81, SP_CODE, 0, reg, -4);

}
void mk_pop(byte reg) {
	append_instr(0x93, reg, SP_CODE, 0, 4);
}
void mk_op(byte ocmod, byte regA, byte regB, byte regC) {
	append_instr(ocmod, regA, regB, regC, 0);
}
void mk_ld(operand op, byte reg) {
	switch (op.optype) {
	case(IMM_LIT):
		append_lit_pool(op.literal);
		append_instr(0x92, reg, PC_CODE, ZERO_CODE, garbage);
		break;
	case (IMM_SYM):
		append_sym_pool(op.symbol);
		append_instr(0x92, reg, PC_CODE, ZERO_CODE, garbage);
		break;
	case (MEM_LIT):
		append_lit_pool(op.literal);
		append_instr(0x92, reg, PC_CODE, ZERO_CODE, garbage);
		append_instr(0x92, reg, reg, ZERO_CODE, ZERO_CODE);
		break;
	case (MEM_SYM):
		append_sym_pool(op.symbol);
		append_instr(0x92, reg, PC_CODE, ZERO_CODE, garbage);
		append_instr(0x92, reg, reg, ZERO_CODE, ZERO_CODE);
		break;
	case (IMM_REG):
		// reg = op.reg & op.reg
		append_instr(0x61, reg, op.reg, op.reg, 0);
		break;
	case (MEM_REG):
		append_instr(0x92, reg, op.reg, ZERO_CODE, 0);
		break;
	case (MEM_REG_LIT):
		if (op.literal >= (1U << 12))
			exit_with_err("found [%%r%u + %u], but %u can't fit into 12b", reg, op.literal, op.literal);
		append_instr(0x92, reg, op.reg, ZERO_CODE, op.literal);
		break;
	case (MEM_REG_SYM):
		append_reloc_local(op.symbol);
		append_instr(0x92, reg, op.reg, ZERO_CODE, garbage);
		break;
	}

}
void mk_st(byte reg, operand op) {
	switch (op.optype) {
	case (IMM_LIT):
		exit_with_err("instruction st with an immediate literal");
		break;
	case (IMM_SYM):
		exit_with_err("instruction st with an immediate symbol");
		break;
	case (MEM_LIT):
		append_lit_pool(op.literal);
		append_instr(0x82, PC_CODE, ZERO_CODE, reg, garbage);
		break;
	case (MEM_SYM):
		append_sym_pool(op.symbol);
		append_instr(0x82, PC_CODE, ZERO_CODE, reg, garbage);
		break;
	case (IMM_REG):
		// op.reg = reg & reg
		append_instr(0x61, op.reg, reg, reg, 0);
		break;
	case (MEM_REG):
		append_instr(0x80, op.reg, ZERO_CODE, reg, 0);
		break;
	case (MEM_REG_LIT):
		if (op.literal >= (1U << 12))
			exit_with_err("found [%%r%u + %u], but %u can't fit into 12b", op.reg, op.literal, op.literal);
		append_instr(0x80, op.reg, ZERO_CODE, reg, op.literal);
		break;
	case (MEM_REG_SYM):
		append_reloc_local(op.symbol);
		append_instr(0x80, op.reg, ZERO_CODE, reg, garbage);
		break;
	}

}
void mk_csrrd(byte csreg, byte reg) {
	append_instr(0x90, reg, csreg, 0, 0);
}
void mk_csrwr(byte reg, byte csreg) {
	append_instr(0x94, csreg, reg, 0, 0);
}
