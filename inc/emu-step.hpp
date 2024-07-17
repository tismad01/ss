#ifndef SS_EMU_STEP_HPP
#define SS_EMU_STEP_HPP
#include "../inc/common.hpp"

void illegal_instr();
void handle_int(word_t cause);
bool step();

#endif /* ifndef SS_EMU_STEP_HPP */
