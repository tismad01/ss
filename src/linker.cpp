
#include "../inc/linker.hpp"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <set>
#include "../inc/common.hpp"
#include "../inc/hex.hpp"
#include "../inc/reloc.hpp"
#include <string>
#include <stdlib.h>
#include <map>


std::vector<std::tuple<addr_t, std::string>> placed_sections;



reloc r1, r2;
hex h;


std::map<std::string, addr_t> line_up_sections(){
	std::map<std::string, addr_t> ret;
	//delovi memorije koji nisu okupirani sekcijom, pocetna adresa, krajnja adresa + 1
	std::set<std::pair<size_t, size_t>> free_space;
	free_space.insert({0, 1LL << 32});

	for (auto [addr, name]: placed_sections) {
		if (ret.find(name) != ret.end())
			exit_with_err("multiple -place arguments for section %s", name.c_str());
		size_t ind = r1.sections.size();
		for (size_t i = 0; i < r1.sections.size(); i++) {
			if (r1.sections[i].name == name) {
				ind = i;
				break;
			}
		}
		if (ind == r1.sections.size())
			exit_with_err("no section named %s, needed by -place", name.c_str());

		for (auto [start, end]: free_space) {
			if (end >= addr) {
				size_t size = (addr_t)r1.sections[ind].data.size();
				if (addr + size > end)
					exit_with_err("couldn't fit section %s due to overlapping -place", name.c_str());

				free_space.erase({start, end});
				if (start < addr)
					free_space.insert({start, addr});
				if (addr + size < end)
					free_space.insert({addr + size, end});
				ret[name] = (addr_t)addr;
				break;
			}
		}
	}


	for (auto sec: r1.sections) {
		if (ret.find(sec.name) == ret.end()) {
			for (auto [start, end]: free_space) {
				size_t size = (addr_t)sec.data.size();
				if (start + size <= end) {
					free_space.erase({start, end});
					if (start + size < end)
						free_space.insert({start + size, end});
					ret[sec.name] = (addr_t)start;
					break;
				}
			}
		}
	}
	return ret;
}

static void patch_addr(section &sec, addr_t addr, word_t val) {
	for(int i = 0; i < 4; i++){
		sec.data[addr + i] = (val & 0xFF);
		val >>= 8;
	}
}

void build_hex(std::map<std::string, addr_t> placements) {
	for (auto reloc: r1.resolvedtab)
		for (auto &ssec: r1.sections)
			if (ssec.name == reloc.sym_sec)
				for (auto &rsec: r1.sections)
					if (rsec.name == reloc.rel_sec)
						patch_addr(rsec, reloc.rel_offset, placements[ssec.name] + reloc.sym_offset);

	h.programs.clear();
	for (auto sec: r1.sections) {
		h.programs.push_back({placements[sec.name], sec.data});
	}
}

const char *hex_aux_path(const char *out_path) {
	char *hex_out_path;
	size_t len = strlen(out_path);
	hex_out_path = (char*)malloc(len + 5);
	strcpy(hex_out_path, out_path);
	strcpy(hex_out_path + len, ".out");

	return hex_out_path;
}

int main(int argc, char **argv){

	bool need_hex = false, need_reloc = false;
	bool first_reloc = true;

	const char* out_path = "a.hex";

	for(int i = 1; i < argc; i++){
		std::string arg = argv[i];
		if(arg == "-o" && (i < argc - 1)) {
			out_path = argv[++i];
		}
		else if(arg == "-hex"){
			need_hex = true;
		}
		else if (arg == "-reloc") {
			need_reloc = true;
		}
		else if(arg.substr(0, 6) == "-place"){
			arg = arg.substr(7);
			std::size_t pos = arg.find("@");
			if(pos == arg.npos)
				exit_with_err("bad argument %s", argv[i]);
			std::string num = arg.substr(pos + 1);
			addr_t address;
			if (num[0] == '0' && num[1] == 'x')
				address = (addr_t)strtol(num.substr(2).c_str(), NULL, 16);
			else
				address = (addr_t)strtol(num.c_str(), NULL, 10);
			std::string section_name = arg.substr(0, pos);
			placed_sections.push_back({address, section_name});
		}
		else {
			auto data = read_file(argv[i]);

			if (first_reloc) {
				first_reloc = false;
				reloc_read(data, r1);
			}
			else {
				reloc_read(data, r2);
				reloc_merge(r1, r2);
			}
		}
	}


	FILE *out = fopen(out_path, "w");
	if (!out)
		exit_with_err("coudln't open file %s", out_path);


	reloc_resolve(r1);


	if (need_reloc) {
		reloc_print(out, r1);
	}
	if(need_hex) {
		if (r1.reltab.size() > 0)
			exit_with_err("not all relocations could be resolved");
		const char *hex_out_path = hex_aux_path(out_path);
		FILE *hex_out = fopen(hex_out_path, "w");

		auto placements = line_up_sections();
		build_hex(placements);
		hex_print(hex_out, out, h);

		fclose(hex_out);
	}



	return 0;
}
