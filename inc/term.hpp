#ifndef SS_TERM_HPP
#define SS_TERM_HPP

#include "common.hpp"

extern addr_t term_in;
extern addr_t term_out;

void term_init();
void term_close();

int kbhit();


#endif /* ifndef SS_TERM_HPP */
