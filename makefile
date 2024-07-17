CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic
ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator
NIVO_A=tests/nivo-a
NIVO_B=tests/nivo-b

nivo-a: all
	@mkdir -p out/
	./${ASSEMBLER} -o out/main.o $(NIVO_A)/main.s
	./${ASSEMBLER} -o out/math.o $(NIVO_A)/math.s
	./${ASSEMBLER} -o out/handler.o $(NIVO_A)/handler.s
	./${ASSEMBLER} -o out/isr_timer.o $(NIVO_A)/isr_timer.s
	./${ASSEMBLER} -o out/isr_terminal.o $(NIVO_A)/isr_terminal.s
	./${ASSEMBLER} -o out/isr_software.o $(NIVO_A)/isr_software.s
	./${LINKER} -hex \
	  -place=my_code@0x40000000 -place=math@0xF0000000 \
	  -o out/program.hex \
	  out/handler.o out/math.o out/main.o out/isr_terminal.o out/isr_timer.o out/isr_software.o
	./${EMULATOR} out/program.hex.out

nivo-b: all
	@mkdir -p out/
	./${ASSEMBLER} -o out/main.o $(NIVO_B)/main.s
	./${ASSEMBLER} -o out/handler.o $(NIVO_B)/handler.s
	./${ASSEMBLER} -o out/isr_terminal.o $(NIVO_B)/isr_terminal.s
	./${ASSEMBLER} -o out/isr_timer.o $(NIVO_B)/isr_timer.s
	./${LINKER} -hex \
	  -place=my_code@0x40000000 \
	  -o out/program.hex \
	  out/main.o out/isr_terminal.o out/isr_timer.o out/handler.o
	./${EMULATOR} out/program.hex.out


all: $(ASSEMBLER) $(LINKER) $(EMULATOR)


misc/lexer.cpp: misc/lexer.l
	flex --header-file=misc/lexer.hpp --outfile=misc/lexer.cpp $^

misc/parser.cpp: misc/parser.y
	bison --header=misc/parser.hpp --output=misc/parser.cpp $^

clean:
	rm -rf misc/lexer.hpp misc/parser.hpp
	rm -rf misc/lexer.cpp misc/parser.cpp assembler linker emulator
	rm -rf out/


$(ASSEMBLER): src/assembler.cpp src/common.cpp src/reloc.cpp misc/lexer.cpp misc/parser.cpp src/asm-instr.cpp src/asm-directives.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

$(LINKER): src/linker.cpp src/common.cpp src/reloc.cpp src/hex.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

$(EMULATOR): src/emulator.cpp src/common.cpp src/hex.cpp src/emu-step.cpp src/term.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^


