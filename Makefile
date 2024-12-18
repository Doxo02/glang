default_target: run

clean:
	rm -f examples/test.o examples/test.asm examples/test.out
	rm -f stdlib/core.o stdlib/core.asm stdlib/linux.o stdlib/linux.asm stdlib/libstd.a

run: test.out
	./examples/test.out

test.out: test.o
	ld -Lstdlib/ examples/test.o -lstd -o examples/test.out

test.o: test.asm
	nasm -felf64 -g -Fdwarf examples/test.asm -o examples/test.o

test.asm:
	./cmake-build-debug/glang examples/test.glang

stdlib: core.o linux.o
	ar rcs -g stdlib/libstd.a stdlib/core.o stdlib/linux.o

core.o: core.asm
	nasm -felf64 -g -Fdwarf stdlib/core.asm -o stdlib/core.o

linux.o: linux.asm
	nasm -felf64 -g -Fdwarf stdlib/linux.asm -o stdlib/linux.o

core.asm:
	./cmake-build-debug/glang stdlib/core.glang -L --no-core

linux.asm:
	./cmake-build-debug/glang stdlib/linux.glang -L --no-core