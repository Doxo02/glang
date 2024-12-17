default_target: run

run: test.out
	rm examples/*.o
	./examples/test.out

test.out: test.o
	ld -Lstdlib/ examples/test.o -lstd -o examples/test.out

test.o: test.asm
	nasm -felf64 -g -Fdwarf examples/test.asm -o examples/test.o

test.asm: build-compiler
	./build/glang examples/test.glang

stdlib: core.o linux.o
	ar rcs stdlib/libstd.a stdlib/core.o stdlib/linux.o
	rm -rf stdlib/*.o

core.o: core.asm
	nasm -felf64 -g -Fdwarf stdlib/core.asm -o stdlib/core.o

linux.o: linux.asm
	nasm -felf64 -g -Fdwarf stdlib/linux.asm -o stdlib/linux.o

core.asm: build-compiler
	./build/glang stdlib/core.glang -L --no-core

linux.asm: build-compiler
	./build/glang stdlib/linux.glang -L --no-core

build-compiler:
	(cd build && make)