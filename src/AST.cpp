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

void IdExpression::accept(Visitor* visitor) {
    visitor->visitIdExpression(this);
}

void BinaryExpression::accept(Visitor* visitor) {
    right->accept(visitor);
    left->accept(visitor);
    visitor->visitBinaryExpression(this);
}

void Compound::accept(Visitor* visitor) {
    visitor->visitCompound(this);
    for(Statement* stmt : statements) {
        stmt->accept(visitor);
    }
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

void VarAssignment::accept(Visitor* visitor) {
    value->accept(visitor);
    visitor->visitVarAssignment(this);
}

void VarDeclaration::accept(Visitor* visitor) {
    visitor->visitVarDeclaration(this);
}

void FunctionDefinition::accept(Visitor* visitor) {
    visitor->visitFunctionDefinition(this);
    body->accept(visitor);
}

void Program::accept(Visitor* visitor) {
    for(VarDeclaration* decl : declarations) {
        decl->accept(visitor);
    }
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

void PrintVisitor::visitCompound(Compound* stmt) {
    std::cout << "Visiting Compound..." << std::endl;
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

// ConstExprVisitor implementation

void ConstExprVisitor::visitIntLit(IntLit* expr) {
    stack.push(expr->value);
}

void ConstExprVisitor::visitStringLit(StringLit* expr) {
    stack.push({});
}

void ConstExprVisitor::visitIdExpression(IdExpression* expr) {
    stack.push({});
}

void ConstExprVisitor::visitBinaryExpression(BinaryExpression* expr) {
    int left;
    int right;
    if(stack.top().has_value()) left = stack.top().value();
    else return;
    stack.pop();
    if(stack.top().has_value()) right = stack.top().value();
    else return;
    stack.pop();

    switch(expr->op) {
        case BinaryOperator::PLUS:
            stack.push(left + right);
            break;
        case BinaryOperator::MINUS:
            stack.push(left - right);
            break;
        case BinaryOperator::MUL:
            stack.push(left * right);
            break;
        case BinaryOperator::DIV:
            stack.push(left / right);
            break;
    }
}

void ConstExprVisitor::visitCompound(Compound* stmt) {}

void ConstExprVisitor::visitIf(If* stmt) {}

void ConstExprVisitor::visitReturn(Return* stmt) {
    if(stack.top().has_value())
        stmt->value = new IntLit(stack.top().value());
}

void ConstExprVisitor::visitCallStatement(CallStatement* stmt) {
    for(int i = 0; i < stmt->arguments.size(); i++) {
        stmt->arguments.at(i)->accept(this);

        if(stack.top().has_value()) stmt->arguments.at(i) = new IntLit(stack.top().value());
        stack.pop();
    }
}

void ConstExprVisitor::visitVarAssignment(VarAssignment *stmt) {
    if(stack.top().has_value()) stmt->value = new IntLit(stack.top().value());
    stack.pop();
}

void ConstExprVisitor::visitVarDeclaration(VarDeclaration *decl) {}

void ConstExprVisitor::visitFunctionDefinition(FunctionDefinition *def) {}

void ConstExprVisitor::visitProgram(Program* prog) {}

// CountVarDeclVisitor implementation

CountVarDeclVisitor::CountVarDeclVisitor() {
    varDecls = 0;
}

void CountVarDeclVisitor::visitIntLit(IntLit* expr) {}
void CountVarDeclVisitor::visitStringLit(StringLit* expr) {}
void CountVarDeclVisitor::visitIdExpression(IdExpression* expr) {}
void CountVarDeclVisitor::visitBinaryExpression(BinaryExpression* expr) {}

void CountVarDeclVisitor::visitCompound(Compound* stmt) {}
void CountVarDeclVisitor::visitIf(If* stmt) {}
void CountVarDeclVisitor::visitReturn(Return* stmt) {}
void CountVarDeclVisitor::visitCallStatement(CallStatement* stmt) {}
void CountVarDeclVisitor::visitVarAssignment(VarAssignment* stmt) {}

void CountVarDeclVisitor::visitVarDeclaration(VarDeclaration* stmt) {
    varDecls++;
}

void CountVarDeclVisitor::visitFunctionDefinition(FunctionDefinition* def) {}
void CountVarDeclVisitor::visitProgram(Program* prog) {}

int CountVarDeclVisitor::getNumVarDecls() {
    return varDecls;
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

void CodeGenVisitor::visitIdExpression(IdExpression* expr) {
    stack.push("[" + expr->id.name + "]");
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

void CodeGenVisitor::visitCompound(Compound* stmt) {
    textSegment.push_back(new Push("rbp"));
    textSegment.push_back(new Move("rbp", "rsp"));
    CountVarDeclVisitor* visitor = new CountVarDeclVisitor();
    stmt->accept(visitor);
    textSegment.push_back(new Sub("rsp", std::to_string(visitor->getNumVarDecls() * 8)));
}

void CodeGenVisitor::visitIf(If* stmt) {}

void CodeGenVisitor::visitReturn(Return* stmt) {
    textSegment.push_back(new Move("rsp", "rbp"));
    textSegment.push_back(new Pop("rbp"));
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

void CodeGenVisitor::visitVarAssignment(VarAssignment *stmt) {
    std::string val = stack.top();
    stack.pop();

    textSegment.push_back(new Move("dword [" + stmt->id.name + "]", val));
}

void CodeGenVisitor::visitVarDeclaration(VarDeclaration* decl) {
    switch(decl->type.type) {
        case TypeIdentifierType::I8:
            dataSegment.push_back(new DefineVar(decl->id.name, "db"));
            break;
        case TypeIdentifierType::I16:
            dataSegment.push_back(new DefineVar(decl->id.name, "dw"));
            break;
        case TypeIdentifierType::I32:
            dataSegment.push_back(new DefineVar(decl->id.name, "dd"));
            break;
        case TypeIdentifierType::I64:
            dataSegment.push_back(new DefineVar(decl->id.name, "dq"));
            break;
        case TypeIdentifierType::VOID:
        case TypeIdentifierType::CHAR:
        case TypeIdentifierType::F32:
        case TypeIdentifierType::F64:
        case TypeIdentifierType::BOOL:
          break;
        }
}

void CodeGenVisitor::visitFunctionDefinition(FunctionDefinition *def) {
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
