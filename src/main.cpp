#include <iostream>
#include <fstream>

#include "Lexer.hpp"
#include "Parser.hpp"

void printParseTree(Program program);

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage: rlang <source_file>" << std::endl;
        return 1;
    }

    std::ifstream srcFile(argv[1]);

    std::string line;
    Lexer lexer;
    unsigned int number = 0;
    while(std::getline(srcFile, line)) {
        lexer.passLine(line, number++);
    }
    srcFile.close();

    Parser parser(lexer.getTokens());

    Program program = parser.parse();

    printParseTree(program);

    return 0;
}

void printParseTree(Program program) {
    std::cout << program.main.toString(0) << std::endl;
    
}