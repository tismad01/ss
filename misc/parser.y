
%{
#include <stdio.h>
#include <string.h>
#include <algorithm>
int yyparse(void);
int yylex(void);
void yyerror(const char*);



%}
%code requires {
	#include "../inc/common.hpp"
	#include "../inc/assembler.hpp"
	#include "../inc/asm-instr.hpp"
	#include "../inc/asm-directives.hpp"
}
%union {
	int num;
	char *ident;
	operand oper;
	reg_t reg;
}


%token TOKEN_LPAR
%token TOKEN_RPAR
%token TOKEN_LBRACKET
%token TOKEN_RBRACKET
%token TOKEN_PLUS
%token TOKEN_MINUS
%token TOKEN_PERCENT
%token TOKEN_DOLLAR
%token TOKEN_SEMI
%token TOKEN_COLON
%token TOKEN_COMMA

%token <ident> TOKEN_GLOBAL
%token <ident> TOKEN_EXTERN
%token <ident> TOKEN_SECTION
%token <ident> TOKEN_WORD
%token <ident> TOKEN_SKIP
%token <ident> TOKEN_ASCII
%token <ident> TOKEN_END

%token <ident> TOKEN_HALT
%token <ident> TOKEN_INT
%token <ident> TOKEN_IRET
%token <ident> TOKEN_CALL
%token <ident> TOKEN_RET
%token <ident> TOKEN_JMP
%token <ident> TOKEN_BEQ
%token <ident> TOKEN_BNE
%token <ident> TOKEN_BGT
%token <ident> TOKEN_PUSH
%token <ident> TOKEN_POP
%token <ident> TOKEN_XCHG
%token <ident> TOKEN_ADD
%token <ident> TOKEN_SUB
%token <ident> TOKEN_MUL
%token <ident> TOKEN_DIV
%token <ident> TOKEN_NOT
%token <ident> TOKEN_AND
%token <ident> TOKEN_OR
%token <ident> TOKEN_XOR
%token <ident> TOKEN_SHL
%token <ident> TOKEN_SHR
%token <ident> TOKEN_LD
%token <ident> TOKEN_ST
%token <ident> TOKEN_CSRRD
%token <ident> TOKEN_CSRWR

%token <num> TOKEN_NUM
%token <ident> TOKEN_IDENT
%token <ident> TOKEN_STRING

%token <reg> TOKEN_GPR
%token <reg> TOKEN_SP
%token <reg> TOKEN_PC
%token <reg> TOKEN_STATUS
%token <reg> TOKEN_HANDLER
%token <reg> TOKEN_CAUSE


%nterm <oper> op;
%nterm <oper> spec_operand;
%nterm word;
%nterm <ident> global_list;
%nterm <ident> extern_list;
%nterm word_list;
%nterm <instr> instr;
%nterm <ident> directive;
%nterm <reg> csreg;

%define parse.error verbose

%%

prog
	: statements
	;


statements
	:
	| statements statement
	;

statement
	: directive
	| instr
	;

directive
	: TOKEN_GLOBAL global_list
	| TOKEN_EXTERN extern_list
	| TOKEN_SECTION TOKEN_IDENT  {
		if (!ended) start_section($2);
	}
	| TOKEN_WORD word_list
	| TOKEN_SKIP TOKEN_NUM {
		if (!ended) mk_skip($2);
	}
	| TOKEN_ASCII TOKEN_STRING {
		if (!ended) mk_ascii($2);
		free($2);
	}
	| TOKEN_END {
		if (!ended) mk_end();
	}
	| TOKEN_IDENT TOKEN_COLON  {
		if (!ended) mk_label($1);
		free($1);
	}
	;

global_list
	: TOKEN_IDENT {
		if (!ended) mk_global($1);
		free($1);
	}
	| global_list TOKEN_COMMA TOKEN_IDENT {
		if (!ended) mk_global($3);
		free($3);
	}
	;

extern_list
	: TOKEN_IDENT {
		if (!ended) mk_extern($1);
		free($1);
	}
	| extern_list TOKEN_COMMA TOKEN_IDENT {
		if (!ended) mk_extern($3);
		free($3);
	}
	;

word_list
	: word
	| word_list TOKEN_COMMA word
	;

word
	: TOKEN_IDENT {
		if (!ended) mk_word($1);
		free($1);
	}
	| TOKEN_NUM {
		if (!ended) mk_word($1);
	}
	;


instr
	: TOKEN_HALT
		{ if (!ended) mk_halt(); }
	| TOKEN_INT
		{ if (!ended) mk_int(); }
	| TOKEN_IRET
		{ if (!ended) mk_iret(); }
	| TOKEN_CALL spec_operand
		{ if (!ended) mk_call($2); free_op($2);}
	| TOKEN_RET
		{ if (!ended) mk_pop(PC_CODE); }
	| TOKEN_JMP spec_operand
		{ if (!ended) mk_jmp($2); free_op($2); }
	| TOKEN_BEQ TOKEN_GPR TOKEN_COMMA TOKEN_GPR TOKEN_COMMA spec_operand
		{ if (!ended) mk_branch(0x31, $2, $4, $6); free_op($6); }
	| TOKEN_BNE TOKEN_GPR TOKEN_COMMA TOKEN_GPR TOKEN_COMMA spec_operand
		{ if (!ended) mk_branch(0x32, $2, $4, $6); free_op($6); }
	| TOKEN_BGT TOKEN_GPR TOKEN_COMMA TOKEN_GPR TOKEN_COMMA spec_operand
		{ if (!ended) mk_branch(0x33, $2, $4, $6); free_op($6); }
	| TOKEN_PUSH TOKEN_GPR
		{ if (!ended) mk_push($2); }
	| TOKEN_POP TOKEN_GPR
		{ if (!ended) mk_pop($2); }
	| TOKEN_XCHG TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x40, 0, $2, $4); }
	| TOKEN_ADD TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x50, $4, $4, $2); }
	| TOKEN_SUB TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x51, $4, $4, $2); }
	| TOKEN_MUL TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x52, $4, $4, $2); }
	| TOKEN_DIV TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x53, $4, $4, $2); }
	| TOKEN_NOT TOKEN_GPR
		{ if (!ended) mk_op(0x60, $2, $2, 0); }
	| TOKEN_AND TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x61, $4, $4, $2);}
	| TOKEN_OR TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x62, $4, $4, $2);}
	| TOKEN_XOR TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x63, $4, $4, $2);}
	| TOKEN_SHL TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x70, $4, $4, $2);}
	| TOKEN_SHR TOKEN_GPR TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_op(0x71, $4, $4, $2);}
	| TOKEN_LD op TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_ld($2, $4); free_op($2); }
	| TOKEN_ST TOKEN_GPR TOKEN_COMMA op
		{ if (!ended) mk_st($2, $4); free_op($4); }
	| TOKEN_CSRRD csreg TOKEN_COMMA TOKEN_GPR
		{ if (!ended) mk_csrrd($2, $4); }
	| TOKEN_CSRWR TOKEN_GPR TOKEN_COMMA csreg
		{ if (!ended) mk_csrwr($2, $4); }
	;

op
	: TOKEN_NUM {
		$$ = operand(MEM_LIT, $1, 0, 0);
	}
	| TOKEN_IDENT {
		$$ = operand(MEM_SYM, 0, $1, 0);
	}
	| TOKEN_DOLLAR TOKEN_NUM{
		$$ = operand(IMM_LIT, $2, 0, 0);
	}
	| TOKEN_DOLLAR TOKEN_IDENT{
		$$ = operand(IMM_SYM, 0, $2, 0);
	}
	| TOKEN_GPR {
		$$ = operand(IMM_REG, 0, 0, $1);
	}
	| TOKEN_LBRACKET TOKEN_GPR TOKEN_RBRACKET {
		$$ = operand(MEM_REG, 0, 0, $2);
	}
	| TOKEN_LBRACKET TOKEN_GPR TOKEN_PLUS TOKEN_NUM TOKEN_RBRACKET {
		$$ = operand(MEM_REG_LIT, $4, 0, $2);
	}
	| TOKEN_LBRACKET TOKEN_GPR TOKEN_PLUS TOKEN_IDENT TOKEN_RBRACKET {
		$$ = operand(MEM_REG_SYM, 0, $4, $2);
	}
	;

spec_operand
	: TOKEN_NUM {
		$$ = operand(IMM_LIT, $1, 0, 0);}
	| TOKEN_IDENT {
		$$ = operand(IMM_SYM, 0, $1, 0);
	}
	;

csreg
	: TOKEN_STATUS  { $$ = 0; }
	| TOKEN_HANDLER { $$ = 1; }
	| TOKEN_CAUSE   { $$ = 2; }
	;



%%
