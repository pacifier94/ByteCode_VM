CC = gcc
CXX = g++
CFLAGS = 
CXXFLAGS = -O3


all: asm vm

asm: asm.c
	$(CC) $(CFLAGS) asm.c -o asm

vm: vm.cpp
	$(CXX) $(CXXFLAGS) vm.cpp -o vm

clean:
	rm -f asm vm program.bin