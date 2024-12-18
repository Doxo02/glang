#include <iostream>
#include <fstream>
#include <string>

#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

void printParseTree(const Program* program);

int main(int argc, char** argv) {
    bool asLib = false;
    bool core = true;

    if(argc < 2) {
        std::cout << "Usage: glang <source_file>" << std::endl;
        return EXIT_FAILURE;
    }
    if(argc > 2) {
        for(int i = 2; i < argc; i++) {
            if(std::string(argv[i]) == "-L") {
                asLib = true;
            }
            if(std::string(argv[i]) == "--no-core") {
                core = false;
            }
        }
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

    Parser parser(lexer.getTokens(), fileName, core);
    
    std::string outFileName = fileName.replace(fileName.find(".glang"), 6, ".asm");

    Program* program = parser.parse();

    //ConstExprVisitor cVisitor;
    //program->accept(&cVisitor);

    //printParseTree(program);

    TypeChecker typeChecker;
    CodeGenVisitor visitor;

    program->accept(&typeChecker);
    program->accept(&visitor);

    auto data = visitor.getDataSegment();
    auto text = visitor.getTextSegment();
    auto bss = visitor.getBssSegment();
    auto ro = visitor.getROSegment();
    auto globals = visitor.getGlobals();
    auto externs = program->getExterns();

    std::ofstream outFile(outFileName);

    outFile << "section .text" << std::endl;
    if(!asLib) {
        outFile << "global _start" << std::endl;
        outFile << "_start:" << std::endl;
        outFile << "\tmov rdi, [rsp]" << std::endl;
        outFile << "\tlea rsi, [rsp + 8]" << std::endl;
        outFile << "\tcall main" << std::endl;
        outFile << "\tmov rdi, rax" << std::endl;
        outFile << "\tmov rax, 60" << std::endl;
        outFile << "\tsyscall" << std::endl;
    }

    for(const std::string& label : globals) {
        outFile << "global " << label << std::endl;
    }
    for(const std::string& label : externs) {
        outFile << "extern " << label << std::endl;
    }

    for(auto t: text) {
        outFile << t->genNasm() << std::endl;
    }

    outFile << std::endl << "section .data" << std::endl;

    for(auto d : data) {
        outFile << d->genNasm() << std::endl;
    }

    outFile << std::endl << "section .bss" << std::endl;

    for(auto d : bss) {
        outFile << d->genNasm() << std::endl;
    }

    outFile << std::endl << "section .rodata" << std::endl;
    for(auto d : ro) {
        outFile << d->genNasm() << std::endl;
    }

    return 0;
}

void printParseTree(const Program* program) {
    for (const FunctionDefinition* def : program->functions) {
        std::cout << def->toString(0) << std::endl;
    }
}