#include "AST.hpp"
#include "OpCode.hpp"

#include <cstddef>
#include <stack>
#include <string>
#include <vector>

void IntLit::accept(Visitor* visitor, std::string reg) {
    visitor->visitIntLit(this, reg);
}

void StringLit::accept(Visitor* visitor, std::string reg)  {
    visitor->visitStringLit(this, reg);
}

void IdExpression::accept(Visitor* visitor, std::string reg) {
    visitor->visitIdExpression(this, reg);
}

void BinaryExpression::accept(Visitor* visitor, std::string reg) {
    visitor->visitBinaryExpression(this, reg);
}

void CallExpression::accept(Visitor* visitor, std::string reg) {
    visitor->visitCallExpression(this, reg);
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
    visitor->visitReturn(this);
}

void CallStatement::accept(Visitor* visitor) {
    visitor->visitCallStatement(this);
}

void VarAssignment::accept(Visitor* visitor) {
    visitor->visitVarAssignment(this);
}

void VarDeclaration::accept(Visitor* visitor) {
    visitor->visitVarDeclaration(this);
}

void VarDeclAssign::accept(Visitor* visitor) {
    visitor->visitVarDeclAssign(this);
}

void While::accept(Visitor* visitor)
{
    visitor->visitWhile(this);
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

void ConstExprVisitor::visitIntLit(IntLit* expr, std::string reg) {
    stack.emplace(expr->value);
}

void ConstExprVisitor::visitStringLit(StringLit* expr, std::string reg) {
    stack.emplace();
}

void ConstExprVisitor::visitIdExpression(IdExpression* expr, std::string reg) {
    stack.emplace();
}

void ConstExprVisitor::visitBinaryExpression(BinaryExpression* expr, std::string reg) {
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
            stack.emplace(left + right);
            break;
        case BinaryOperator::MINUS:
            stack.emplace(left - right);
            break;
        case BinaryOperator::MUL:
            stack.emplace(left * right);
            break;
        case BinaryOperator::DIV:
            stack.emplace(left / right);
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
    for(auto & argument : stmt->arguments) {
        //argument->accept(this, TODO);

        if(stack.top().has_value()) argument = new IntLit(stack.top().value());
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

void CountVarDeclVisitor::visitIntLit(IntLit* expr, std::string reg) {}
void CountVarDeclVisitor::visitStringLit(StringLit* expr, std::string reg) {}
void CountVarDeclVisitor::visitIdExpression(IdExpression* expr, std::string reg) {}
void CountVarDeclVisitor::visitBinaryExpression(BinaryExpression* expr, std::string reg) {}

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

int CountVarDeclVisitor::getNumVarDecls() const {
    return varDecls;
}

// CodeGenVisitor implementation

void CodeGenVisitor::push(const std::string& what, const size_t bytes = 8) {
    textSegment.push_back(new Push(what));
    offset += bytes;
}

void CodeGenVisitor::pop(const std::string& where, const size_t bytes = 8) {
    textSegment.push_back(new Pop(where));
    offset -= bytes;
}

CodeGenVisitor::CodeGenVisitor() {
    root = new Scope(nullptr);
    current = root;
}

void CodeGenVisitor::visitIntLit(IntLit* expr, const std::string reg) {
    textSegment.push_back(new Move(reg, std::to_string(expr->value)));
}

void CodeGenVisitor::visitStringLit(StringLit* expr, const std::string reg) {
    auto* code = new DefineString(expr->value, stringIndex++);

    dataSegment.push_back(code);
    textSegment.push_back(new Move(reg, code->getId()));
}

void CodeGenVisitor::visitIdExpression(IdExpression* expr, const std::string reg) {
    if(parameters.find(expr->id.name) != parameters.cend()) {
        auto [type, index] = parameters.find(expr->id.name)->second;
        const int off = static_cast<int>(parameters.size()) - index;

        textSegment.push_back(new Move(reg, "qword [rbp + " + std::to_string(off*8+8) + "]"));
        int i = 0;
        if(expr->id.name == "argv") i = 1;
        deref(expr->derefDepth - i, reg);

        if (expr->derefDepth == type.ptrDepth) {
            makeType(type.type);
        }
    } else {
        Var var = current->getVar(expr->id);
        const int off = offset - var.offset;

        textSegment.push_back(new Move(reg, "qword [rsp + " + std::to_string(off) + "]"));
        deref(expr->derefDepth, reg);

        if (expr->derefDepth == var.type.ptrDepth) {
            makeType(var.type.type);
        }
    }
}

void CodeGenVisitor::visitBinaryExpression(BinaryExpression* expr, const std::string reg) {
    expr->left->accept(this, reg);
    expr->right->accept(this, "r11");
    if(expr->op == BinaryOperator::PLUS) {
        textSegment.push_back(new Add(reg, "r11"));
    }
    else if(expr->op == BinaryOperator::MINUS) {
        textSegment.push_back(new Sub(reg, "r11"));
    }
    else if(expr->op == BinaryOperator::MUL) {
        textSegment.push_back(new Multiply(reg, "r11"));
    }
    else if(expr->op == BinaryOperator::DIV) {
        textSegment.push_back(new Div(reg, "r11"));
    }
    else if (expr->op == BinaryOperator::NEQUALS) {
        textSegment.push_back(new NotEqual(reg, "r11"));
    }

    deref(expr->derefDepth, reg);
}

void CodeGenVisitor::visitCallExpression(CallExpression* expr, std::string reg) {
    if(expr->id.name == "syscall") {
        expr->args.at(0)->accept(this, "rax");
        expr->args.at(1)->accept(this, "rdi");
        expr->args.at(2)->accept(this, "rsi");
        expr->args.at(3)->accept(this, "rdx");
        expr->args.at(4)->accept(this, "r10");
        expr->args.at(5)->accept(this, "r8");
        expr->args.at(6)->accept(this, "r9");

        textSegment.push_back(new Syscall());
    } else {
        for (Expression* expr : expr->args)
        {
            expr->accept(this, "r11");
            push("r11");
        }
        textSegment.push_back(new Call(expr->id.name));
        for (int i = 0; i < expr->args.size(); i++)
        {
            pop("r11");
        }

        if (reg != "rax")
            textSegment.push_back(new Move(reg, "rax"));
    }
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
    if(func.top()->returnType.type != TypeIdentifierType::VOID) {
        stmt->value->accept(this, "rax");
    }
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
    func.pop();
}

void CodeGenVisitor::visitCallStatement(CallStatement* stmt) {
    if(stmt->id.name == "syscall") {
        stmt->arguments.at(0)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(1)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(2)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(3)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(4)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(6)->accept(this, "rax");
        push("rax");
        stmt->arguments.at(5)->accept(this, "rax");
        push("rax");

        pop("r9");
        pop("r8");
        pop("rcx");
        pop("rdx");
        pop("rsi");
        pop("rdi");
        pop("rax");

        textSegment.push_back(new Syscall());
    } else {
        for (Expression* expr : stmt->arguments)
        {
            expr->accept(this, "r11");
            push("r11");
        }
        textSegment.push_back(new Call(stmt->id.name));

        for (int i = 0; i < stmt->arguments.size(); i++)
        {
            pop("r11");
        }
    }

    //parameterStack.push(parameters);
}

void CodeGenVisitor::visitVarAssignment(VarAssignment *stmt) {
    Var var = current->getVar(stmt->id);
    stmt->value->accept(this, "rax");
    if (var.type.ptrDepth == 0)
    {
        makeType(var.type.type);
    }
    textSegment.push_back(new Move("[rsp + " + std::to_string(offset - var.offset) + "]", "rax"));
}

void CodeGenVisitor::visitVarDeclaration(VarDeclaration* stmt) {
    push("qword 0");
    current->addVar(stmt->id, Var{offset, stmt->type});
}

void CodeGenVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    stmt->value->accept(this, "rax");
    push("rax");
    current->addVar(stmt->id, Var{offset, stmt->type});
}

void CodeGenVisitor::visitWhile(While* stmt)
{
    int i = whileIndex++;
    textSegment.push_back(new Label("while" + std::to_string(i) + "_start"));
    stmt->condition->accept(this, "rax");
    textSegment.push_back(new Compare("rax", "0"));
    textSegment.push_back(new Jump("je", "while" + std::to_string(i) + "_end"));
    stmt->body->accept(this);
    textSegment.push_back(new Jump("jmp", "while" + std::to_string(i) + "_start"));
    textSegment.push_back(new Label("while" + std::to_string(i) + "_end"));
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

void CodeGenVisitor::deref(const int depth, const std::string& reg)
{
    for (int i = 0 ; i < depth; i++)
    {
        textSegment.push_back(new Move(reg, "[" + reg + "]"));
    }
}

void CodeGenVisitor::makeType(const TypeIdentifierType type)
{
    switch (type)
    {
    case TypeIdentifierType::I64:
    case TypeIdentifierType::F64:
        break;
    case TypeIdentifierType::I8:
    case TypeIdentifierType::CHAR:
        textSegment.push_back(new XOR("rbx", "rbx"));
        textSegment.push_back(new Move("bl", "al"));
        textSegment.push_back(new Move("rax", "rbx"));
        break;
    case TypeIdentifierType::I16:
        textSegment.push_back(new XOR("rbx", "rbx"));
        textSegment.push_back(new Move("bx", "ax"));
        textSegment.push_back(new Move("rax", "rbx"));
        break;
    case TypeIdentifierType::I32:
    case TypeIdentifierType::F32:
        textSegment.push_back(new XOR("rbx", "rbx"));
        textSegment.push_back(new Move("ebx", "eax"));
        textSegment.push_back(new Move("rax", "rbx"));
        break;
    case TypeIdentifierType::VOID:
    case TypeIdentifierType::BOOL:
        std::cerr << "Void and Bool types are unsupported right now" << std::endl;
        exit(EXIT_FAILURE);
    }
}



std::stack<size_t> CodeGenVisitor::getStack() {
    return offsetStack;
}

std::vector<OpCode*> CodeGenVisitor::getDataSegment() {
    return dataSegment;
}

std::vector<OpCode*> CodeGenVisitor::getTextSegment() {
    return textSegment;
}