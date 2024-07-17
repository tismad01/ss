#include "../inc/emulator.hpp"
#include "../inc/common.hpp"
#include "../inc/hex.hpp"
#include "../inc/emu-step.hpp"
#include "../inc/term.hpp"
#include <map>

context ctx;
hex h;

int usage(const char *prog_name) {
	printf("usage: %s <input>\n", prog_name);
	return 1;
}

word_t mem32get(addr_t addr) {
	word_t b0 = ctx.mem[addr + 3];
	word_t b1 = ctx.mem[addr + 2];
	word_t b2 = ctx.mem[addr + 1];
	word_t b3 = ctx.mem[addr + 0];

	return (((((b0 << 8) | b1) << 8) | b2) << 8) | b3;
}

void mem32set(addr_t addr, word_t val) {
	if(addr >= 0xFFFFFF00LL - 3){

		if(addr == term_out){
			printf("%c", val & 0xff);
		}
		else if (addr == term_in) {
			for (int i = 0; i < 4; i++) {
				ctx.mem[addr + i] = val & 0xff;
				val >>= 8;
			}
		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			ctx.mem[addr + i] = val & 0xff;
			val >>= 8;
		}
	}
}
void init_ctx() {
	ctx.reg[ZERO_CODE] = 0;
	ctx.reg[PC_CODE] = 0x40000000;
	ctx.mem.clear();
	for (auto [offset, prog]: h.programs)
		for (size_t i = 0; i < prog.size(); i++)
			ctx.mem[offset + (addr_t)i] = prog[i];
}

void print_result() {
	for (int i = 0; i < 65; i++)
		printf("-");
	printf("\nEmulated processor executed halt instruction:");
	printf("\nEmulated processor state:\n");
	for (int i = 0; i < 16; i++) {
		if (i < 10)
			printf(" ");
		printf("r%d=0x%08x   ", i, ctx.reg[i]);
		if (i % 4 == 3)
			printf("\n");
	}
}

int main(int argc, char **argv){
	char *in_path;
	if (argc != 2)
		return usage(argv[0]);
	in_path = argv[1];
	auto data = read_file(in_path);


	hex_read(data, h);
	term_init();
	init_ctx();

	while (step()) {
		if (kbhit()) {
			int c = getchar();
			mem32set(term_in, c);
			if((ctx.csreg[STATUS_CODE] & 0x3) == 0) {
				handle_int(3);
			}
		}
	}

	print_result();

	term_close();

	return 0;
}
