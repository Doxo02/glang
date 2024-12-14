#include <iostream>
#include <fstream>
#include <string>

#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

void printParseTree(Program* program);

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage: glang <source_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string fileName = argv[1];

    std::ifstream srcFile(fileName);

    std::string line;
    Lexer lexer;
    unsigned int number = 1;
    while(std::getline(srcFile, line)) {
        lexer.passLine(line, number++);
    }
    srcFile.close();

    std::string outFileName = fileName.replace(fileName.find(".glang"), 6, ".asm");

    Parser parser(lexer.getTokens());

    Program* program = parser.parse();

    //ConstExprVisitor cVisitor;
    //program->accept(&cVisitor);

    //printParseTree(program);

    CodeGenVisitor visitor;

    program->accept(&visitor);

    auto data = visitor.getDataSegment();
    auto text = visitor.getTextSegment();

    std::ofstream outFile(outFileName);

    outFile << "section .text" << std::endl;
    outFile << "global _start" << std::endl;
    outFile << "_start:" << std::endl;
    outFile << "\tmov rdi, [rsp]" << std::endl;
    outFile << "\tlea rsi, [rsp + 8]" << std::endl;
    outFile << "\tcall main" << std::endl;
    outFile << "\tmov rdi, rax" << std::endl;
    outFile << "\tmov rax, 60" << std::endl;
    outFile << "\tsyscall" << std::endl;

    for(auto t: text) {
        outFile << t->genNasm() << std::endl;
    }

    outFile << "section .data" << std::endl;

    for(auto d : data) {
        outFile << d->genNasm() << std::endl;
    }

    return 0;
}

void printParseTree(Program* program) {
    for (FunctionDefinition* def : program->functions) {
        std::cout << def->toString(0) << std::endl;
    }
}