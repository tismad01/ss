#ifndef SS_ASM_INSTR_HPP
#define SS_ASM_INSTR_HPP
#include "../inc/common.hpp"
#include "../inc/assembler.hpp"


void mk_halt();
void mk_int();
void mk_iret();
void mk_call(operand op);
void mk_jmp(operand op);
void mk_branch(byte mode, byte regA, byte regB, operand op);
void mk_push(byte reg);
void mk_pop(byte reg);
void mk_op(byte ocmod, byte regA, byte regB, byte regC);
void mk_ld(operand op, byte reg);
void mk_st(byte reg, operand op);
void mk_csrrd(byte creg, byte reg);
void mk_csrwr(byte reg, byte csreg);


#endif /* ifndef SS_ASM_INSTR_HPP */
