#include "AST.hpp"

#include <iostream>

void IntLit::accept(Visitor* visitor) {
    visitor->visitIntLit(this);
}

void StringLit::accept(Visitor* visitor)  {
    visitor->visitStringLit(this);
}

void Scope::accept(Visitor* visitor) {
    visitor->visitScope(this);
    for(Statement* stmt : statements) {
        stmt->accept(visitor);
    }
}

void Return::accept(Visitor* visitor) {
    visitor->visitReturn(this);
    value->accept(visitor);
}

void CallStatement::accept(Visitor* visitor) {
    visitor->visitCallStatement(this);
    for(Expression* expr : arguments) {
        expr->accept(visitor);
    }
}

void FunctionDefinition::accept(Visitor* visitor) {
    visitor->visitFunctionDefinition(this);
    body->accept(visitor);
}

void Program::accept(Visitor* visitor) {
    visitor->visitProgram(this);
    main.accept(visitor);
}

// PrintVisitor implementation

void PrintVisitor::visitIntLit(IntLit* expr) {
    std::cout << "Visiting IntLit..." << std::endl;
}

void PrintVisitor::visitStringLit(StringLit* expr) {
    std::cout << "Visiting StringLit..." << std::endl;
}

void PrintVisitor::visitScope(Scope* stmt) {
    std::cout << "Visiting Scope..." << std::endl;
}

void PrintVisitor::visitCallStatement(CallStatement* stmt) {
    std::cout << "Visiting CallStatement..." << std::endl;
}

void PrintVisitor::visitFunctionDefinition(FunctionDefinition* def) {
    std::cout << "Visiting FunctionDefinition..." << std::endl;
}

void PrintVisitor::visitIf(If* stmt) {
    std::cout << "Visiting If..." << std::endl;
}

void PrintVisitor::visitProgram(Program* prog) {
    std::cout << "Visiting Program..." << std::endl;
}

void PrintVisitor::visitReturn(Return* stmt) {
    std::cout << "Visiting Return..." << std::endl;
}