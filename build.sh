#!/usr/bin/env bash

# Build the project

echo "Building stdlib..."
echo "Compiling linux.glang..."
./cmake-build-debug/glang ./stdlib/linux.glang -L --no-core
echo "Assembling linux.asm..."
nasm -felf64 -g -Fdwarf ./stdlib/linux.asm -o ./stdlib/linux.o
echo "Compiling core.glang..."
./cmake-build-debug/glang ./stdlib/core.glang -L --no-core
echo "Assembling core.asm..."
nasm -felf64 -g -Fdwarf ./stdlib/core.asm -o ./stdlib/core.o
echo "Creating libglang.a..."
ar rcs ./stdlib/libglang.a ./stdlib/linux.o ./stdlib/core.o

echo ""
echo "Building test..."
echo "Compiling test.glang..."
./cmake-build-debug/glang ./examples/test.glang
echo "Assembling test.asm..."
nasm -felf64 -g -Fdwarf ./examples/test.asm -o ./examples/test.o
echo "Linking test.out..."
ld -g -Lstdlib examples/test.o -lglang -o ./examples/test.out