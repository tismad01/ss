#ifndef SS_COMMON_HPP
#define SS_COMMON_HPP
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#include <unistd.h>

typedef uint8_t byte;
typedef uint8_t reg_t;
typedef uint32_t instr_t;
typedef uint32_t addr_t;
typedef uint32_t word_t;
#define MAX_STR_LEN 100

struct name {
	char str[MAX_STR_LEN];
	name(const char *_name) {
		memset(str, 'n', sizeof(name));
		strcpy(str, _name);
	}
};



enum reg_type: reg_t {
	ZERO_CODE = 0,
	SP_CODE = 14,
	PC_CODE = 15,
	STATUS_CODE = 0,
	HANDLER_CODE = 1,
	CAUSE_CODE = 2
};

enum operand_type {
	IMM_LIT,
	IMM_SYM,
	MEM_LIT,
	MEM_SYM,
	IMM_REG,
	MEM_REG,
	MEM_REG_LIT,
	MEM_REG_SYM
};

struct operand {
	operand_type optype;
	word_t literal;
	char *symbol;
	reg_t reg;

	operand() = default;
	operand(operand_type op, addr_t l, char *s, reg_t r)
		: optype(op), literal(l), symbol(s), reg(r) {  }
};



#define exit_with_err(...) \
	do { \
		fprintf(stderr, "error %s:%d: ", __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while (false)


template<typename T>
static inline void write_to_binary(FILE *out, T x) {
	fwrite(&x, sizeof(x), 1, out);
}
template<typename T>
static inline T read_from_data(byte *data, size_t &offset) {
	T *x = (T*)&data[offset];
	offset += sizeof(T);
	return *x;
}

std::tuple<byte*, long> read_file(char *path);



#endif /* ifndef SS_COMMON_HPP */
