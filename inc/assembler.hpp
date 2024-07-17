#ifndef SS_ASSEMBLER_HPP
#define SS_ASSEMBLER_HPP
#include "../inc/common.hpp"
#include <stdint.h>
#include <string>
#include "../inc/reloc.hpp"



extern bool ended;
extern reloc r;
#define CURR_SEC r.sections[r.sections.size() - 1]


enum instruction_type {
	HALT_CODE, INT_CODE, IRET_CODE, CALL_CODE, RET_CODE,
	JMP_CODE, BEQ_CODE, BNE_CODE, BGT_CODE,
	PUSH_CODE, POP_CODE,
	XCHG_CODE,
	ADD_CODE, SUB_CODE, MUL_CODE, DIV_CODE,
	NOT_CODE, AND_CODE, OR_CODE, XOR_CODE, SHL_CODE, SHR_CODE,
	LD_CODE, ST_CODE, CSRRD_CODE, CSRWR_CODE
};

void free_op(operand);
void start_section(std::string);
void end_last_section();


#endif /* ifndef SS_ASSEMBLER_HPP */
