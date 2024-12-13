cmake_minimum_required(VERSION 3.5.0)
project(glang VERSION 1.0 LANGUAGES CXX)

add_executable(glang src/main.cpp src/Lexer.cpp src/Parser.cpp src/AST.cpp
        src/ScratchAllocator.cpp
        src/ScratchAllocator.h)
