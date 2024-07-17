#ifndef SS_EMULATOR_HPP
#define SS_EMULATOR_HPP


#include "../inc/common.hpp"
#include <unordered_map>
#include <map>

struct context{
	std::unordered_map<addr_t, byte> mem;
	word_t reg[16];
	word_t csreg[3];
};

extern context ctx;

enum opcode: byte {
	OC_HALT = 0x0,
	OC_INT = 0x1,
	OC_CALL = 0x2,
	OC_JMP = 0x3,
	OC_XCHG = 0x4,
	OC_MATHS = 0x5,
	OC_LOGIC = 0x6,
	OC_SHIFT = 0x7,
	OC_ST = 0x8,
	OC_LD = 0x9
};

word_t mem32get(addr_t);
void mem32set(addr_t, word_t);


void print_result();


#endif /* ifndef SS_EMULATOR_HPP */
