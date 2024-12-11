#include "AST.hpp"
#include "OpCode.hpp"

#include <cstddef>
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

void CallExpression::accept(Visitor* visitor) {
    for(Expression* expr : args) expr->accept(visitor);
    visitor->visitCallExpression(this);
}

void Compound::accept(Visitor* visitor) {
    visitor->visitCompound(this);
    for(Statement* stmt : statements) {
        stmt->accept(visitor);
    }
}

void EndCompound::accept(Visitor* visitor) {
    visitor->visitEndCompound(this);
}

void Return::accept(Visitor* visitor) {
    if(value != nullptr) value->accept(visitor);
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

void VarDeclAssign::accept(Visitor* visitor) {
    value->accept(visitor);
    visitor->visitVarDeclAssign(this);
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
void ConstExprVisitor::visitEndCompound(EndCompound* stmt) {}
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

void ConstExprVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    if(stack.top().has_value()) stmt->value = new IntLit(stack.top().value());
    stack.pop();
}

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
void CountVarDeclVisitor::visitEndCompound(EndCompound* stmt) {}
void CountVarDeclVisitor::visitIf(If* stmt) {}
void CountVarDeclVisitor::visitReturn(Return* stmt) {}
void CountVarDeclVisitor::visitCallStatement(CallStatement* stmt) {}
void CountVarDeclVisitor::visitVarAssignment(VarAssignment* stmt) {}

void CountVarDeclVisitor::visitVarDeclaration(VarDeclaration* stmt) {
    varDecls++;
}

void CountVarDeclVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    varDecls++;
}

void CountVarDeclVisitor::visitFunctionDefinition(FunctionDefinition* def) {}
void CountVarDeclVisitor::visitProgram(Program* prog) {}

int CountVarDeclVisitor::getNumVarDecls() {
    return varDecls;
}

// CodeGenVisitor implementation

CodeGenVisitor::CodeGenVisitor() {
    root = new Scope(nullptr);
    current = root;
}

void CodeGenVisitor::visitIntLit(IntLit* expr) {
    push("qword " + std::to_string(expr->value), 8);
}

void CodeGenVisitor::visitStringLit(StringLit* expr) {
    DefineString* code = new DefineString(expr->value, stringIndex++);

    dataSegment.push_back(code);
    push(code->getId(), 8);
}

void CodeGenVisitor::visitIdExpression(IdExpression* expr) {
    if(parameters.find(expr->id.name) != parameters.cend()) {
        auto data = parameters.find(expr->id.name)->second;
        int off = parameters.size() - data.index;

        if(expr->derefDepth == 0) {
            push("qword [rbp + " + std::to_string(off*8+8) + "]", 8);
            return;
        }

        textSegment.push_back(new Move("rax", "qword [rbp + " + std::to_string(off*8+8) + "]"));
        int i = 0;
        if(expr->id.name == "argv") i = 1;
        for(; i < expr->derefDepth; i++) {
            textSegment.push_back(new Move("rax", "[rax]"));
        }
        push("rax", 8);
    } else {
        Var var = current->getVar(expr->id);

        std::cout << expr->id.name << ": " << var.type.ptrDepth << std::endl;

        std::string toPush = "qword ";
        for(int i = 0; i < expr->derefDepth; i++) toPush.append("[");
        toPush.append("[rsp + ");
        toPush.append(std::to_string(offset - var.offset));
        toPush.append("]");
        for(int i = 0; i < expr->derefDepth; i++) toPush.append("]");

        push(toPush, 8);
    }
}

void CodeGenVisitor::visitBinaryExpression(BinaryExpression* expr) {
    pop("rax", 8);
    pop("rbx", 8);
    if(expr->op == BinaryOperator::PLUS) {
        textSegment.push_back(new Add("rax", "rbx"));
    } else if(expr->op == BinaryOperator::MINUS) {
        textSegment.push_back(new Sub("rax", "rbx"));
    } else if(expr->op == BinaryOperator::MUL) {
        textSegment.push_back(new Multiply("rax", "rbx"));
    } else if(expr->op == BinaryOperator::DIV) {
        textSegment.push_back(new Div("rax", "rbx"));
    }
    std::string toPush = "";
    for(int i = 0; i < expr->derefDepth; i++) toPush.append("[");
    toPush.append("rax");
    for(int i = 0; i < expr->derefDepth; i++) toPush.append("]");
    push("rax", 8);
}

void CodeGenVisitor::visitCallExpression(CallExpression* expr) {
    if(expr->id.name == "syscall") {
        pop("r9", 8);
        pop("r8", 8);
        pop("r10", 8);
        pop("rdx", 8);
        pop("rsi", 8);
        pop("rdi", 8);
        pop("rax", 8);

        textSegment.push_back(new Syscall());
    } else {
        textSegment.push_back(new Call(expr->id.name));
    }

    push("rax", 8);
}

void CodeGenVisitor::visitCompound(Compound* stmt) {
    // CountVarDeclVisitor* visitor = new CountVarDeclVisitor();
    // stmt->accept(visitor);
    // textSegment.push_back(new Sub("rsp", std::to_string(visitor->getNumVarDecls() * 8)));
    current = new Scope(current);
}

void CodeGenVisitor::visitEndCompound(EndCompound* stmt) {
    current = current->getParent();
}

void CodeGenVisitor::visitIf(If* stmt) {}

void CodeGenVisitor::visitReturn(Return* stmt) {
    if(func.top()->returnType.type != TypeIdentifierType::VOID) pop("rax", 8);
    textSegment.push_back(new Move("rsp", "rbp"));
    textSegment.push_back(new Pop("rbp"));
    offset = 0;
    textSegment.push_back(new ReturnOp());

    offset = offsetStack.top();
    offsetStack.pop();

    if(!parameterStack.empty()) {
        parameters = parameterStack.top();
        parameterStack.pop();
    }
}

void CodeGenVisitor::visitCallStatement(CallStatement* stmt) {
    if(stmt->id.name == "syscall") {
        pop("r9", 8);
        pop("r8", 8);
        pop("r10", 8);
        pop("rdx", 8);
        pop("rsi", 8);
        pop("rdi", 8);
        pop("rax", 8);

        textSegment.push_back(new Syscall());
    } else {
        textSegment.push_back(new Call(stmt->id.name));
    }

    //parameterStack.push(parameters);
}

void CodeGenVisitor::visitVarAssignment(VarAssignment *stmt) {
    Var var = current->getVar(stmt->id);
    pop("rax", 8);
    textSegment.push_back(new Move("[rsp + " + std::to_string(offset - var.offset) + "]", "rax"));
}

void CodeGenVisitor::visitVarDeclaration(VarDeclaration* decl) {
    push("qword 0", 8);
    current->addVar(decl->id, Var{offset, decl->type});
}

void CodeGenVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    current->addVar(stmt->id, Var{offset, stmt->type});
}

void CodeGenVisitor::visitFunctionDefinition(FunctionDefinition *def) {
    textSegment.push_back(new Label(def->id.name));
    textSegment.push_back(new Push("rbp"));
    textSegment.push_back(new Move("rbp", "rsp"));

    offsetStack.push(offset);
    offset = 0;

    parameters = def->args;

    func.push(def);
}

void CodeGenVisitor::visitProgram(Program* prog) {}

std::stack<size_t> CodeGenVisitor::getStack() {
    return offsetStack;
}

std::vector<OpCode*> CodeGenVisitor::getDataSegment() {
    return dataSegment;
}

std::vector<OpCode*> CodeGenVisitor::getTextSegment() {
    return textSegment;
}

void CodeGenVisitor::push(std::string what, size_t bytes) {
    textSegment.push_back(new Push(what));
    offset += bytes;
}

void CodeGenVisitor::pop(std::string where, size_t bytes) {
    textSegment.push_back(new Pop(where));
    offset -= bytes;
}