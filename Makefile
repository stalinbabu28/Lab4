all: object remove

object: main.o
	g++ main.o -o riscv_asm

main.o: main.cpp
	g++ -c main.cpp

remove:
	rm main.o
	
clean:
	rm riscv_asm