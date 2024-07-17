#include "../inc/common.hpp"
#include <stdio.h>

std::tuple<byte*, long> read_file(char *path) {
	FILE *file = fopen(path, "rb");
	if (!file)
		exit_with_err("couldn't open file %s", path);

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	byte* data = (byte*) malloc(fsize + 1);
	fread(data, fsize, 1, file);

	fclose(file);
	return {data, fsize};
}

