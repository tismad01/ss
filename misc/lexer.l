
%{
#include <stdio.h>
#include <string.h>
#include "../misc/parser.hpp"
#include "../inc/common.hpp"

int line_number = 1;
reg_t getNums(char *);

%}




/*  %option warn nodefault */


%option noyywrap

%x COMMENT


LPAR \(
RPAR \)
LBRACKET \[
RBRACKET \]
PLUS \+
MINUS -
PERCENT %
DOLLAR \$
SEMI ;
COLON :
COMMA ,

GLOBAL ^\.global
EXTERN ^\.extern
SECTION ^\.section
WORD ^\.word
SKIP ^\.skip
ASCII ^\.ascii
END	^\.end

HALT halt
INT int
IRET iret
CALL call
RET ret
JMP jmp
BEQ beq
BNE bne
BGT bgt
PUSH push
POP pop
XCHG xchg
ADD add
SUB  sub
MUL mul
DIV div
NOT not
AND and
OR or
XOR xor
SHL shl
SHR shr
LD ld
ST st
CSRRD csrrd
CSRWR csrwr


GPR (%r([0-9]|1[0-3]))
SP (%sp|%r14)
PC (%pc|%r15)
STATUS (%status)
HANDLER (%handler)
CAUSE (%cause)


STRING \"[a-zA-Z0-9_ ]*\"
NUM [0-9]+
HEX   0[Xx][0-9A-Fa-f]+
IDENT [a-zA-Z_][a-zA-Z0-9_]*
WS    \s*
NEWLINE \n
COMMENT #.*$




%%

{GLOBAL} { return (TOKEN_GLOBAL); }
{EXTERN} { return (TOKEN_EXTERN); }
{SECTION} { return (TOKEN_SECTION); }
{WORD} { return (TOKEN_WORD); }
{SKIP} { return (TOKEN_SKIP); }
{ASCII} { return (TOKEN_ASCII); }
{END}	{ return (TOKEN_END); }

{HALT} {yylval.num = HALT_CODE; return (TOKEN_HALT); }
{INT}  {yylval.num = INT_CODE; return (TOKEN_INT); }
{IRET} {yylval.num = IRET_CODE; return (TOKEN_IRET); }
{CALL} {yylval.num = CALL_CODE; return (TOKEN_CALL); }
{RET} {yylval.num = RET_CODE; return (TOKEN_RET); }
{JMP} {yylval.num = JMP_CODE; return (TOKEN_JMP); }
{BEQ} {yylval.num = BEQ_CODE; return (TOKEN_BEQ); }
{BNE} {yylval.num = BNE_CODE; return (TOKEN_BNE); }
{BGT} {yylval.num = BGT_CODE; return (TOKEN_BGT); }
{PUSH} {yylval.num = PUSH_CODE; return (TOKEN_PUSH); }
{POP} {yylval.num = POP_CODE; return (TOKEN_POP); }
{XCHG} {yylval.num = XCHG_CODE; return (TOKEN_XCHG); }
{ADD} {yylval.num = ADD_CODE; return (TOKEN_ADD); }
{SUB}  {yylval.num = SUB_CODE; return (TOKEN_SUB); }
{MUL} {yylval.num = MUL_CODE; return (TOKEN_MUL); }
{DIV} {yylval.num = DIV_CODE; return (TOKEN_DIV); }
{NOT} {yylval.num = NOT_CODE; return (TOKEN_NOT); }
{AND} {yylval.num = AND_CODE; return (TOKEN_AND); }
{OR} {yylval.num = OR_CODE; return (TOKEN_OR); }
{XOR} {yylval.num = XOR_CODE; return (TOKEN_XOR); }
{SHL} {yylval.num = SHL_CODE; return (TOKEN_SHL); }
{SHR} {yylval.num = SHR_CODE; return (TOKEN_SHR); }
{LD} {yylval.num = LD_CODE; return (TOKEN_LD); }
{ST} {yylval.num = ST_CODE; return (TOKEN_ST); }
{CSRRD} {yylval.num = CSRRD_CODE; return (TOKEN_CSRRD); }
{CSRWR} {yylval.num = CSRWR_CODE; return (TOKEN_CSRWR); }

       
{GPR} {yylval.reg = getNums(yytext); return (TOKEN_GPR); }
{SP} {yylval.reg = SP_CODE; return (TOKEN_GPR); }
{PC} {yylval.reg = PC_CODE; return (TOKEN_GPR); }
{STATUS} {yylval.reg = STATUS_CODE; return (TOKEN_STATUS); }
{HANDLER} {yylval.reg = HANDLER_CODE; return (TOKEN_HANDLER); }
{CAUSE} {yylval.reg = CAUSE_CODE; return (TOKEN_CAUSE); }

{LPAR}      { return (TOKEN_LPAR); }
{RPAR}      { return (TOKEN_RPAR); }
{LBRACKET} { return (TOKEN_LBRACKET); }
{RBRACKET} { return (TOKEN_RBRACKET); }
{PLUS}      { return (TOKEN_PLUS); }
{MINUS}  { return (TOKEN_MINUS); }
{PERCENT} { return (TOKEN_PERCENT); }
{DOLLAR} { return (TOKEN_DOLLAR); }
{SEMI}      { return (TOKEN_SEMI); }
{COLON} { return (TOKEN_COLON); }
{COMMA}     { return (TOKEN_COMMA); }

{COMMENT} { }
{WS}        {  }
{NEWLINE} { line_number++; }


{STRING} { yylval.ident = strdup(yytext); return TOKEN_STRING; }
{IDENT} { yylval.ident = strdup(yytext); return TOKEN_IDENT; }
{NUM} { yylval.num = atoi(yytext); return TOKEN_NUM; }
{HEX} { yylval.num = strtoq(yytext, NULL, 16); return TOKEN_NUM; }


%%

int
yyerror(const char* msg) {
    fprintf(stderr, "Error at line %d: %s\n", line_number, msg);
    return 0;
}

reg_t getNums(char *str)
{

    const char* p = str + 2;

    uint8_t num = (uint8_t) atoi(p);

    return num;
}



