#include "../inc/asm-directives.hpp"
#include "../inc/common.hpp"
#include "../inc/assembler.hpp"

void mk_label(char *str) {
	CURR_SEC.syms[(addr_t)CURR_SEC.data.size()] = str;
}

void mk_global(char *str) {
	for(auto glob: r.globals)
		if(strcmp(glob.c_str(), str) == 0)
			exit_with_err("duplicate global definition");

	r.globals.insert(str);
}

void mk_extern(char *str) {
	r.externs.insert(str);
}

void mk_skip(int bytes) {
	for (int i = 0; i < bytes; i++)
		CURR_SEC.data.emplace_back(0);

}
void mk_word(int num) {
	const byte* bytes = reinterpret_cast<const byte*>(&num);
	for (size_t i = 0; i < sizeof(num); ++i) {
		CURR_SEC.data.emplace_back(bytes[i]);
	}
}
void mk_ascii(char *str) {
	size_t n = strlen(str);
	for (size_t i = 1; i < n - 1; i++)
		CURR_SEC.data.emplace_back((byte)str[i]);
}
void mk_word(char *str) {
	CURR_SEC.syms[(addr_t)CURR_SEC.data.size()] = str;
	CURR_SEC.data.push_back(0);
	CURR_SEC.data.push_back(0);
	CURR_SEC.data.push_back(0);
	CURR_SEC.data.push_back(0);
}

void mk_end() {
	end_last_section();
	ended = true;
}
