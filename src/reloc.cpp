#include "../inc/reloc.hpp"
#include "../inc/common.hpp"

void reloc_print(FILE *out, reloc &r) {
	/* write header */
	size_t nsections = r.sections.size() + 3;
	write_to_binary(out, nsections);
	/* write symtab */
	write_to_binary(out, r.symtab.size());
	for (auto [name, S]: r.symtab) {
		auto [offset, section] = S;
		symbol s(name.c_str(), section.c_str(), offset);
		write_to_binary(out, s);
	}
	/* write reltab */
	write_to_binary(out, r.reltab.size());

	for(auto [name, S]: r.reltab){
		auto [offset, section] = S;
		symbol s(name.c_str(), section.c_str(), offset);
		write_to_binary(out, s);
	}
	/* write resolvedtab */
	write_to_binary(out, r.resolvedtab.size());
	for (auto resolved_reloc: r.resolvedtab) {
		name sname(resolved_reloc.sym_sec.c_str());
		name rname(resolved_reloc.rel_sec.c_str());
		write_to_binary(out, sname);
		write_to_binary(out, resolved_reloc.sym_offset);
		write_to_binary(out, rname);
		write_to_binary(out, resolved_reloc.rel_offset);
	}
	/* write text sections */
	for (auto sec: r.sections) {
		write_to_binary(out, sec.data.size());
		name n(sec.name.c_str());
		write_to_binary(out, n);
		for (auto b: sec.data)
			write_to_binary(out, b);
	}
}

void reloc_read(std::tuple<byte*, long> data, reloc &r) {
	auto [bytes, fsize] = data;
	size_t offset = 0;
	r.sections.clear();
	r.symtab.clear();
	r.reltab.clear();
	r.globals.clear();
	r.resolvedtab.clear();
	/* read header */
	size_t nsections = read_from_data<size_t>(bytes, offset);
	/* read symtab */
	size_t nsyms = read_from_data<size_t>(bytes, offset);
	for (size_t i = 0; i < nsyms; i++) {
		symbol s = read_from_data<symbol>(bytes, offset);
		r.symtab[s.name] = {s.offset, s.section};
	}
	/* read reltab */
	nsyms = read_from_data<size_t>(bytes, offset);
	for (size_t i = 0; i < nsyms; i++) {
		symbol s = read_from_data<symbol>(bytes, offset);
		r.reltab.insert({s.name, {s.offset, s.section}});
	}
	/* read resolvedtab */
	nsyms = read_from_data<size_t>(bytes, offset);
	for (size_t i = 0; i < nsyms; i++) {
		std::string sname = read_from_data<name>(bytes, offset).str;
		addr_t soffset = read_from_data<addr_t>(bytes, offset);
		std::string rname = read_from_data<name>(bytes, offset).str;
		addr_t roffset = read_from_data<addr_t>(bytes, offset);
		r.resolvedtab.insert({sname, rname, soffset, roffset});
	}
	/* read text sections */
	for (size_t i = 0; i < nsections - 3; i++) {
		r.sections.push_back({});
		r.sections[i].data.clear();
		r.sections[i].literals.clear();
		r.sections[i].syms.clear();
		r.sections[i].rels.clear();

		size_t nbytes = read_from_data<size_t>(bytes, offset);
		name n = read_from_data<name>(bytes, offset);
		r.sections[i].name = n.str;

		for (size_t j = 0; j < nbytes; j++) {
			byte x = read_from_data<byte>(bytes, offset);
			r.sections[i].data.emplace_back(x);
		}
	}
}

static void merge_sections(section &s1, reloc &r2, section &s2) {
	/* resolvedtab */
	std::vector<resolved_rel> resolvedtab;
	for (auto x: r2.resolvedtab)
		resolvedtab.push_back(x);
	for (size_t i = 0; i < resolvedtab.size(); i++) {
		auto &reloc = resolvedtab[i];
		if (reloc.rel_sec == s2.name) {
			reloc.rel_offset += s1.data.size();

		}
		if (reloc.sym_sec == s2.name) {
			reloc.sym_offset += s1.data.size();
		}
	}
	r2.resolvedtab.clear();
	for (auto x: resolvedtab)
		r2.resolvedtab.insert(x);
	/* symtab */
	for (auto &[sym, loc]: r2.symtab) {
		auto &[offset, sec] = loc;
		if (sec == s2.name) {
			offset += s1.data.size();
		}
	}
	/* reltab */
	auto temp_reltab = r2.reltab;
	r2.reltab.clear();
	for (auto [name, loc]: temp_reltab) {
		auto [offset, sec] = loc;
		addr_t extra_offset = 0;
		if (sec == s1.name)
			extra_offset = (addr_t)s1.data.size();
		r2.reltab.insert({name, {offset + extra_offset, sec}});
	}
	/* finally merge sections */
	for (auto x: s2.data)
		s1.data.push_back(x);
}

void reloc_merge(reloc &r1, reloc &r2) {
	for (auto sec2: r2.sections) {
		bool merged = false;
		for (auto &sec1: r1.sections) {
			if (sec1.name == sec2.name) {
				merged = true;
				merge_sections(sec1, r2, sec2);
				break;
			}
		}
		if (!merged)
			r1.sections.push_back(sec2);
	}
	for (auto x: r2.resolvedtab)
		r1.resolvedtab.insert(x);
	for (auto [k, v]: r2.symtab) {
		if (r1.symtab.find(k) != r1.symtab.end())
			exit_with_err("conflicting global %s found", k.c_str());
		r1.symtab[k] = v;
	}
	for (auto x: r2.reltab)
		r1.reltab.insert(x);
}

void reloc_resolve(reloc &r) {
	auto temp_reltab = r.reltab;
	for (auto [rname, rel]: temp_reltab) {
		auto [roffset, rsec] = rel;
		for (auto [sname, sym]: r.symtab) {
			auto [soffset, ssec] = sym;
			if (sname == rname) {
				r.resolvedtab.insert({ssec, rsec, soffset, roffset});
				r.reltab.erase({rname, rel});
				break;
			}
		}
	}
}
