#include "../inc/hex.hpp"
#include <algorithm>

void hex_print(FILE *bin_out, FILE *out, hex &h) {
	sort(h.programs.begin(), h.programs.end());
	// Binary file
	write_to_binary(bin_out, h.programs.size());
	for (size_t i = 0; i < h.programs.size(); i++) {
		write_to_binary(bin_out, std::get<0>(h.programs[i]));

		size_t nbytes = std::get<1>(h.programs[i]).size();
		write_to_binary(bin_out, nbytes);
		for (size_t j = 0; j < nbytes; j++)
			write_to_binary(bin_out, std::get<1>(h.programs[i])[j]);
	}


	// Hex file
	for (auto [offset, bytes]: h.programs) {
		for (size_t i = 0; i < bytes.size(); i++) {
			if (i % 8 == 0)
				fprintf(out, "%08lX:", offset + i);
			fprintf(out, " %02X", bytes[i]);
			if (i % 8 == 7 && i != bytes.size() - 1)
				fprintf(out, "\n");
		}
		fprintf(out, "\n");
	}
}

void hex_read(std::tuple<byte*, long> data, hex &h) {
	auto [bytes, fsize] = data;
	h.programs.clear();
	size_t offset = 0;
	/* read header */
	size_t nprograms = read_from_data<size_t>(bytes, offset);
	/* read programs */
	for (size_t i = 0; i < nprograms; i++) {
		addr_t p_offset = read_from_data<addr_t>(bytes, offset);

		std::vector<byte> prog;
		size_t nbytes = read_from_data<size_t>(bytes, offset);
		for (size_t j = 0; j < nbytes; j++) {
			byte x = read_from_data<byte>(bytes, offset);
			prog.push_back(x);
		}
		h.programs.push_back({p_offset, prog});
	}
}
