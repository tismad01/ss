#ifndef SS_HEX_HPP
#define SS_HEX_HPP
#include "../inc/common.hpp"
#include <vector>

struct hex {
	std::vector<std::tuple<addr_t, std::vector<byte>>> programs;
};


void hex_print(FILE *hex_out, FILE *out, hex &r);
void hex_read(std::tuple<byte*, long> data, hex &r);

#endif /* ifndef SS_HEX_HPP */
