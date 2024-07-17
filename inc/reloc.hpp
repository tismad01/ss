#ifndef SS_RELOCATABLE_HPP
#define SS_RELOCATABLE_HPP
#include <vector>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include "../inc/common.hpp"

struct section {
	std::string name;
	std::vector<byte> data;
	
	std::vector<std::tuple<addr_t, word_t>> literals;
	std::vector<std::tuple<addr_t, std::string>> extra_syms;
	std::unordered_map<addr_t, std::string> syms;
	std::unordered_map<addr_t, std::string> rels; //32bit
	std::unordered_map<addr_t, std::string> local_rels; //12bit
};

struct resolved_rel {
	std::string sym_sec, rel_sec;
	addr_t sym_offset, rel_offset;
	bool operator <(const resolved_rel &r) const {
		return std::tie(sym_sec, rel_sec, sym_offset, rel_offset)
			< std::tie(r.sym_sec, r.rel_sec, r.sym_offset, r.rel_offset);
	}
};

struct reloc {
	std::vector<section> sections;
	std::unordered_map<std::string, std::pair<addr_t, std::string>> symtab;
	std::set<std::pair<std::string, std::pair<addr_t, std::string>>> reltab;
	std::set<resolved_rel> resolvedtab;


	// only used in assembler
	std::unordered_set<std::string> globals;
	std::unordered_set<std::string> externs;
};


struct symbol
{
	char name[MAX_STR_LEN];
	char section[MAX_STR_LEN];
	int offset;
	symbol(const char *_name, const char *_section, int _offset) {
		memset(name, 0, sizeof(name));
		memset(section, 0, sizeof(section));
		strcpy(name, _name);
		strcpy(section, _section);
		offset = _offset;
	}
};


void reloc_print(FILE *f, reloc &r);
void reloc_read(std::tuple<byte*, long> data, reloc &r);
void reloc_merge(reloc &r1, reloc &r2);
void reloc_resolve(reloc &r1);



#endif /* ifndef SS_RELOCATABLE_HPP */
