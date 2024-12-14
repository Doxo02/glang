default_target: run

run: test.out
	./examples/test.out

test.out: test.o
	ld examples/test.o -o examples/test.out

test.o: test.asm
	nasm -felf64 examples/test.asm -o examples/test.o

test.asm: build-compiler
	./build/glang examples/test.glang

build-compiler:
	(cd build && make)