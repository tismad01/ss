#include "../misc/parser.hpp"
#include "../inc/common.hpp"
#include "../inc/reloc.hpp"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <set>
#include <vector>
#include <unordered_set>
#include "../inc/asm-instr.hpp"


bool ended = false;
reloc r;



void free_op(operand op){
	if(op.optype == IMM_SYM || op.optype == MEM_REG_SYM || op.optype == MEM_SYM){
		free(op.symbol);
	}
}
addr_t get_curr_addr() {
	return (addr_t)CURR_SEC.data.size();
}

void patch_addr_12(int addr, int lit){
	CURR_SEC.data[addr + 1] = (CURR_SEC.data[addr + 1] & 0xF0) | ((lit >> 8) & 0xF);
	CURR_SEC.data[addr + 0] = (lit & 0xFF);
}

void end_last_section(){
	/* literal pool */
	for (auto [offset, value]:CURR_SEC.literals) {
		patch_addr_12(offset, (addr_t)CURR_SEC.data.size() - offset - 4);  //patchujemo adrese iz bazena

		const byte* bytes2 = reinterpret_cast<const byte*>(&value); //dodajemo bazen literala na kraj sekcije
		for (size_t i = 0; i < sizeof(value); ++i)
			CURR_SEC.data.emplace_back(bytes2[i]);

	}

	/* symbol pool */
	for (auto [offset, name]:CURR_SEC.extra_syms) {
		patch_addr_12(offset, (addr_t)CURR_SEC.data.size() - offset - 4);  //patchujemo adrese iz bazena

		CURR_SEC.rels[(addr_t)CURR_SEC.data.size()] = name;

		word_t value = 0x55555555;
		const byte* bytes2 = reinterpret_cast<const byte*>(&value); //dodajemo bazen literala na kraj sekcije
		for (size_t i = 0; i < sizeof(value); ++i) {
			CURR_SEC.data.emplace_back(bytes2[i]);
		}

	}

	for (auto [offset, sym_name]:CURR_SEC.syms) {//ubacimo sve simbole iz lokalne u globalnu TS
		r.symtab[sym_name] = {offset, CURR_SEC.name};
	}


	/* local symbols (12b) relocations */
	auto temp1 = CURR_SEC.local_rels;
	for(auto [roffset, rname]: temp1){
		bool found = false;
		for(auto [soffset, sname]: CURR_SEC.syms){
			if(rname == sname){ //nasli smo simbol u nasoj sekciji, patchujemo
				patch_addr_12(roffset, soffset - roffset - 4);
				CURR_SEC.rels.erase(roffset);
				found = true;
				break;
			}
		}
		if(!found) exit_with_err("couldn't resolve local symbol %s", rname.c_str());
	}


	//ubacimo preostale lokalne relokacije u globalnu reltab
	for(auto [roffset, rname]:CURR_SEC.rels) //ubacimo sve simbole iz lokalne u globalnu TS
		r.reltab.insert({rname, {roffset, CURR_SEC.name}});

}

void start_section(std::string name)
{
	if(!r.sections.empty()) end_last_section();

	struct section s;

	s.data = std::vector<byte>();
	s.literals = std::vector<std::tuple<addr_t, addr_t>>();
	s.syms = std::unordered_map<addr_t, std::string>();
	s.rels = std::unordered_map<addr_t, std::string>();
	s.name = name;


	r.sections.push_back(s);
}




int usage(const char *prog_name) {
	printf("usage: %s [-o <output>] <input>", prog_name);
	return 1;
}

int main(int argc, char **argv) {
	extern FILE *yyin, *yyout;
	FILE *out;
	const char *in_path, *out_path = "a.o";


	if (argc == 4) {
		if (strcmp("-o", argv[1]) != 0)
			return usage(argv[0]);
		in_path = argv[3];
		out_path = argv[2];
	}
	else if (argc == 2) {
		in_path = argv[1];
	}
	else {
		return usage(argv[0]);
	}
	out = fopen(out_path, "wb");
	yyin = fopen(in_path, "r");
	yyout = fopen("/dev/null", "w");

	if (!yyin)
		exit_with_err("couldn't open %s", in_path);
	if (!yyout)
		exit_with_err("couldn't open /dev/null");
	if (!out)
		exit_with_err("couldn't open %s", out_path);

	if (yyparse() != 0)
		exit_with_err("parsing failed");


	reloc_resolve(r);
	for (auto [name, loc]: r.reltab)
		if (r.externs.find(name) == r.externs.end()) {
			exit_with_err("symbol %s wasn't declared extern", name.c_str());
		}

	size_t found_globals = 0;
	auto temp = r.symtab;
	for (auto [name, loc]: temp)
		if (r.globals.find(name) == r.globals.end()) {
			r.symtab.erase(name);
		}
		else
			found_globals++;
	if (found_globals != r.globals.size())
		exit_with_err("didn't define all globals");

	reloc_print(out, r);



	fclose(yyin);
	fclose(yyout);
	fclose(out);



	return 0;
}
