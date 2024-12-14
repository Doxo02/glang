cmake_minimum_required(VERSION 3.31.1)
project(glang VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cpp src/Lexer.cpp src/Parser.cpp src/AST.cpp)
