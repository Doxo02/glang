#include "AST.hpp"
#include "OpCode.hpp"

#include <algorithm>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

void IntLit::accept(Visitor* visitor) {
    visitor->visitIntLit(this);
}

void StringLit::accept(Visitor* visitor)  {
    visitor->visitStringLit(this);
}

void BinaryExpression::accept(Visitor* visitor) {
    right->accept(visitor);
    left->accept(visitor);
    visitor->visitBinaryExpression(this);
}

void Scope::accept(Visitor* visitor) {
    for(Statement* stmt : statements) {
        stmt->accept(visitor);
    }
    visitor->visitScope(this);
}

void Return::accept(Visitor* visitor) {
    value->accept(visitor);
    visitor->visitReturn(this);
}

void CallStatement::accept(Visitor* visitor) {
    for(Expression* expr : arguments) {
        expr->accept(visitor);
    }
    visitor->visitCallStatement(this);
}

void FunctionDefinition::accept(Visitor* visitor) {
    visitor->visitFunctionDefinition(this);
    body->accept(visitor);
}

void Program::accept(Visitor* visitor) {
    for(FunctionDefinition* def : functions) {
        def->accept(visitor);
    }
    visitor->visitProgram(this);
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

// CodeGenVisitor implementation

void CodeGenVisitor::visitIntLit(IntLit* expr) {
    stack.push(std::to_string(expr->value));
}

void CodeGenVisitor::visitStringLit(StringLit* expr) {
    DefineString* code = new DefineString(expr->value, stringIndex++);

    dataSegment.push_back(code);
    stack.push(code->getId());
}

void CodeGenVisitor::visitBinaryExpression(BinaryExpression* expr) {
    textSegment.push_back(new Move("r11", stack.top()));
    stack.pop();
    if(expr->op == BinaryOperator::PLUS) {
        textSegment.push_back(new Add("r11", stack.top()));
    } else if(expr->op == BinaryOperator::MINUS) {
        textSegment.push_back(new Sub("r11", stack.top()));
    } else if(expr->op == BinaryOperator::MUL) {
        textSegment.push_back(new Multiply("r11", stack.top()));
    } else if(expr->op == BinaryOperator::DIV) {
        textSegment.push_back(new Div("r11", stack.top()));
    }
    stack.pop();

    stack.push("r11");
}

void CodeGenVisitor::visitScope(Scope* stmt) {}

void CodeGenVisitor::visitIf(If* stmt) {}

void CodeGenVisitor::visitReturn(Return* stmt) {
    textSegment.push_back(new Move("rax", stack.top()));
    textSegment.push_back(new ReturnOp());
    stack.pop();
}

void CodeGenVisitor::visitCallStatement(CallStatement* stmt) {
    std::vector<std::string> args;

    for(int i = 0; i < stmt->arguments.size(); i++) {
        args.push_back(stack.top());
        stack.pop();
    }

    std::reverse(args.begin(), args.end());

    if(stmt->id.name == "syscall") {
        textSegment.push_back(new Move("rax", args.at(0)));
        textSegment.push_back(new Move("rdi", args.at(1)));
        textSegment.push_back(new Move("rsi", args.at(2)));
        textSegment.push_back(new Move("rdx", args.at(3)));
        textSegment.push_back(new Move("r10", args.at(4)));
        textSegment.push_back(new Move("r8", args.at(5)));
        textSegment.push_back(new Move("r9", args.at(6)));

        textSegment.push_back(new Syscall());
    }
}

void CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* def) {
    textSegment.push_back(new Label(def->id.name));
}

void CodeGenVisitor::visitProgram(Program* prog) {}

std::stack<std::string> CodeGenVisitor::getStack() {
    return stack;
}

std::vector<OpCode*> CodeGenVisitor::getDataSegment() {
    return dataSegment;
}

std::vector<OpCode*> CodeGenVisitor::getTextSegment() {
    return textSegment;
}