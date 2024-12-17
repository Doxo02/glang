#include "AST.hpp"
#include "OpCode.hpp"
#include "ScratchAllocator.h"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <ios>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>

const std::string GPREGS[] =    {"rbx", "r10",  "r11",  "r12",  "r13",  "r14",  "r15", 
                                 "rax", "rdi",  "rsi", "rdx", "rcx", "r8",  "r9"};

const std::string GPREGS8[] =   {"bl",  "r10b", "r11b", "r12b", "r13b", "r14b", "r15b", 
                                 "al",  "dil",  "sil", "dl",  "cl",  "r8b", "r9b"};

const std::string GPREGS16[] =  {"bx",  "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", 
                                 "ax",  "di",   "si",  "dx",  "cx",  "r8w", "r9w"};

const std::string GPREGS32[] =  {"ebx", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", 
                                 "eax", "edi",  "esi", "edx", "ecx", "r8d", "r9d"};

const int FIRST_ARG = 7;

std::array<bool, 7> usedRegs = {false, false, false, false, false, false, false};

void IntLit::accept(Visitor* visitor, int reg) {
    visitor->visitIntLit(this, reg);
}

void StringLit::accept(Visitor* visitor, int reg)  {
    visitor->visitStringLit(this, reg);
}

void CharLit::accept(Visitor* visitor, int reg) {
    visitor->visitCharLit(this, reg);
}

void IdExpression::accept(Visitor* visitor, int reg) {
    visitor->visitIdExpression(this, reg);
}

void BinaryExpression::accept(Visitor* visitor, int reg) {
    visitor->visitBinaryExpression(this, reg);
}

void CallExpression::accept(Visitor* visitor, int reg) {
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

void If::accept(Visitor* visitor) {
    visitor->visitIf(this);
}

void IfElse::accept(Visitor* visitor) {
    visitor->visitIfElse(this);
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
}

void Program::accept(Visitor* visitor) {
    visitor->visitProgram(this);
    for(VarDeclaration* decl : declarations) {
        decl->accept(visitor);
    }
    for(VarDeclAssign* decl : declAssigns) {
        decl->accept(visitor);
    }
    for(FunctionDefinition* def : functions) {
        def->accept(visitor);
    }
}

// ConstExprVisitor implementation

void ConstExprVisitor::visitIntLit(IntLit* expr, int reg) {
    stack.emplace(expr->value);
}

void ConstExprVisitor::visitStringLit(StringLit* expr, int reg) {
    stack.emplace();
}

void ConstExprVisitor::visitCharLit(CharLit* expr, int reg) {}

void ConstExprVisitor::visitIdExpression(IdExpression* expr, int reg) {
    stack.emplace();
}

void ConstExprVisitor::visitBinaryExpression(BinaryExpression* expr, int reg) {
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
        case BinaryOperator::MOD:
            stack.emplace(left % right);
            break;
        case BinaryOperator::EQUALS:
        case BinaryOperator::NEQUALS:
        case BinaryOperator::LESS:
        case BinaryOperator::GREATER:
        case BinaryOperator::LEQUALS:
        case BinaryOperator::GEQUALS:
        case BinaryOperator::BIT_AND:
        case BinaryOperator::BIT_OR:
            break;
        }
}

void ConstExprVisitor::visitCompound(Compound* stmt) {}
void ConstExprVisitor::visitEndCompound(EndCompound* stmt) {}
void ConstExprVisitor::visitIf(If* stmt) {}
void ConstExprVisitor::visitIfElse(IfElse* stmt) {}

void ConstExprVisitor::visitReturn(Return* stmt) {
    if(stack.top().has_value())
        stmt->value = new IntLit(stack.top().value());
}

void ConstExprVisitor::visitCallStatement(CallStatement* stmt) {
    for(auto & argument : stmt->arguments) {
        //argument->accept(this, );

        if(stack.top().has_value()) argument = new IntLit(stack.top().value());
        stack.pop();
    }
}

void ConstExprVisitor::visitVarAssignment(VarAssignment *stmt) {}

void ConstExprVisitor::visitVarDeclaration(VarDeclaration *decl) {}

void ConstExprVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    if(stack.top().has_value()) stmt->value = new IntLit(stack.top().value());
    stack.pop();
}

void ConstExprVisitor::visitFunctionDefinition(FunctionDefinition *def) {}

void ConstExprVisitor::visitProgram(Program* prog) {}

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
    allocator = ScratchAllocator();
    func.push(nullptr);
}

void CodeGenVisitor::visitIntLit(IntLit* expr, const int reg) {
    textSegment.push_back(new Move(GPREGS[reg], std::to_string(expr->value)));
    expr->type = TypeIdentifierType::I64;
}

void CodeGenVisitor::visitStringLit(StringLit* expr, const int reg) {
    auto* code = new DefineString(expr->value, stringIndex++);

    dataSegment.push_back(code);
    textSegment.push_back(new Move(GPREGS[reg], code->getId()));
    expr->type = TypeIdentifierType::U64;
}

void CodeGenVisitor::visitCharLit(CharLit* expr, const int reg) {
    textSegment.push_back(new XOR(GPREGS[reg], GPREGS[reg]));
    std::stringstream data;
    data << "0x" << std::hex << (int)expr->value;
    textSegment.push_back(new Move(GPREGS8[reg], data.str()));
    expr->type = TypeIdentifierType::CHAR;
}

void CodeGenVisitor::visitIdExpression(IdExpression* expr, const int reg) {
    const bool indexExpr = expr->index != nullptr;
    bool global = false;
    TypeIdentifier type;
    std::string right = "[";

    if(current->getVar(expr->id) != nullptr) {
        const Var* var = current->getVar(expr->id);
        const int off = offset - var->offset;

        type = var->type;
        
        right.append("rsp + ");
        right.append(std::to_string(off));
    } else if(globalVars.find(expr->id.name) != globalVars.cend()) {
        type = globalVars.find(expr->id.name)->second;

        right.append(expr->id.name);
        global = true;
    } else {
        throw std::runtime_error("can't resolve symbol: \"" + expr->id.name + "\"");
    }

    right.append("]");

    if(global || loadAddress) textSegment.push_back(new LoadEffectiveAddr(GPREGS[reg], right));
    else textSegment.push_back(new Move(GPREGS[reg], right));

    if(indexExpr) {
        int indexReg = allocator.allocate();
        expr->index->accept(this, indexReg);
        textSegment.push_back(new Add(GPREGS[reg], GPREGS[indexReg]));
        if(!loadAddress) textSegment.push_back(new Move(GPREGS[reg], "[" + GPREGS[reg] + "]"));
        allocator.free(indexReg);
    }

    deref(expr->derefDepth, GPREGS[reg]);

    expr->type = TypeIdentifierType::U64;
    if (expr->derefDepth == type.ptrDepth && !loadAddress) {
        makeType(type.type, reg);
        expr->type = type.type;
    }
}

void CodeGenVisitor::visitBinaryExpression(BinaryExpression* expr, const int reg) {
    int r = allocator.allocate();
    int lr = reg;

    if((expr->op == BinaryOperator::DIV || expr->op == BinaryOperator::MOD) && reg != 7) {
        lr = 7;
        if(usedRegs[0]) push("rax");
    }

    expr->left->accept(this, lr);
    expr->right->accept(this, r);

    std::string lReg, rReg;
    auto type = expr->right->type;
    expr->type = type;
    bool sign = false;

    switch (type) {
    case TypeIdentifierType::I8:
        lReg = GPREGS8[lr];
        rReg = GPREGS8[r];
        sign = true;
        break;
    case TypeIdentifierType::I16:
        lReg = GPREGS16[lr];
        rReg = GPREGS16[r];
        sign = true;
        break;
    case TypeIdentifierType::I32:
        lReg = GPREGS32[lr];
        rReg = GPREGS32[r];
        sign = true;
        break;
    case TypeIdentifierType::I64:
        lReg = GPREGS[lr];
        rReg = GPREGS[r];
        sign = true;
        break;
    case TypeIdentifierType::U8:
        lReg = GPREGS8[lr];
        rReg = GPREGS8[r];
        break;
    case TypeIdentifierType::U16:
        lReg = GPREGS16[lr];
        rReg = GPREGS16[r];
        break;
    case TypeIdentifierType::U32:
        lReg = GPREGS32[lr];
        rReg = GPREGS32[r];
        break;
    case TypeIdentifierType::U64:
        lReg = GPREGS[lr];
        rReg = GPREGS[r];
        break;
    case TypeIdentifierType::CHAR:
        lReg = GPREGS8[lr];
        rReg = GPREGS8[r];
        break;
    case TypeIdentifierType::BOOL:
        lReg = GPREGS8[lr];
        rReg = GPREGS8[r];
        break;
    case TypeIdentifierType::F32:
    case TypeIdentifierType::F64:
    case TypeIdentifierType::VOID:
    default:
        throw std::runtime_error("Floating point and void types are unsupported");
    }

    if(expr->op == BinaryOperator::PLUS) {
        textSegment.push_back(new Add(lReg, rReg, sign));
    }
    else if(expr->op == BinaryOperator::MINUS) {
        textSegment.push_back(new Sub(lReg, rReg, sign));
    }
    else if(expr->op == BinaryOperator::MUL) {
        textSegment.push_back(new Multiply(lReg, rReg, sign));
    }
    else if(expr->op == BinaryOperator::DIV) {
        if(usedRegs[3]) push("rdx");
        textSegment.push_back(new XOR("rdx", "rdx"));
        textSegment.push_back(new Div(rReg, sign));
        if(usedRegs[3]) pop("rdx");
    }
    else if(expr->op == BinaryOperator::MOD) {
        if(usedRegs[3]) push("rdx");
        textSegment.push_back(new XOR("rdx", "rdx"));
        textSegment.push_back(new Div(rReg, sign));
        textSegment.push_back(new Move(lReg, "rdx"));
        if(usedRegs[3]) pop("rdx");
    }
    else if(expr->op == BinaryOperator::BIT_OR) {
        textSegment.push_back(new OR(lReg, rReg));
    }
    else if(expr->op == BinaryOperator::BIT_AND) {
        textSegment.push_back(new AND(lReg, rReg));
    }
    else {
        textSegment.push_back(new Comparison(lReg, rReg, expr->op));
    }
    allocator.free(r);

    deref(expr->derefDepth, lReg);

    if(lr == 7 && reg != 7) {
        textSegment.push_back(new Move(GPREGS[reg], lReg));
        if(usedRegs[0]) pop("rax");
    }
}

void CodeGenVisitor::visitCallExpression(CallExpression* expr, int reg) {
    for(int i = 0; i < std::size(usedRegs); i++) {
        if(usedRegs[i]) push(GPREGS[i+FIRST_ARG]);
    }
    usedRegStack.push(usedRegs);
    for(bool& b : usedRegs) b = false;

    if(expr->id.name == "syscall") {
        for (int i = 0; i < expr->args.size(); i++)
        {
            expr->args.at(i)->accept(this, i+FIRST_ARG);
            usedRegs[i] = true;
        }

        textSegment.push_back(new Syscall());
    } else {
        for (int i = 0; i < expr->args.size(); i++)
        {
            auto arg = expr->args.at(i);
            arg->accept(this, i+FIRST_ARG+1);
            usedRegs[i+1] = true;
        }
        textSegment.push_back(new Call(expr->id.name));
    }

    if (reg != 7)
        textSegment.push_back(new Move(GPREGS[reg], "rax"));

    std::swap(usedRegs, usedRegStack.top());
    usedRegStack.pop();
    for(int i = std::size(usedRegs)-1; i >= 0; i--) {
        if(usedRegs[i]) {
            pop(GPREGS[i+FIRST_ARG]);
        }
    }
}

void CodeGenVisitor::visitCompound(Compound* stmt) {
    current = new Scope(current);
}

void CodeGenVisitor::visitEndCompound(EndCompound* stmt) {
    int r = allocator.allocate();
    for(int i = 0; i < current->getNumVars(); i++) {
        pop(GPREGS[r]);
    }
    current = current->getParent();
}

void CodeGenVisitor::visitIf(If* stmt) {
    int reg = allocator.allocate();
    int index = ifIndex++;

    stmt->condition->accept(this, reg);
    textSegment.push_back(new Compare(GPREGS[reg], "0"));
    textSegment.push_back(new Jump("je", func.top()->id.name + ".If" + std::to_string(index) + "_End"));
    stmt->body->accept(this);
    textSegment.push_back(new Label(".If" + std::to_string(index) + "_End"));
}

void CodeGenVisitor::visitIfElse(IfElse* stmt) {
    int reg = allocator.allocate();
    int index = ifIndex++;

    stmt->condition->accept(this, reg);
    textSegment.push_back(new Compare(GPREGS[reg], "0"));
    textSegment.push_back(new Jump("je", func.top()->id.name + ".If" + std::to_string(index) + "_Else"));
    stmt->ifBody->accept(this);
    textSegment.push_back(new Jump("jmp",func.top()->id.name + ".If" + std::to_string(index) + "_End"));
    textSegment.push_back(new Label(func.top()->id.name + ".If" + std::to_string(index) + "_Else"));
    stmt->elseBody->accept(this);
    textSegment.push_back(new Label(func.top()->id.name + ".If" + std::to_string(index) + "_End"));
}

void CodeGenVisitor::visitReturn(Return* stmt) {
    if(func.top()->returnType.type != TypeIdentifierType::VOID) {
        stmt->value->accept(this, 7);
    }

    int r = allocator.allocate();
    for(int i = 0; i < func.top()->args.size(); i++) {
        pop(allocator.getReg(r));
    }
    allocator.free(r);

    bool* wasUsed = allocator.getWasUsed();

    for(int i = std::size(REGS)-1; i >= 0; i--) {
        if(wasUsed[i]) {
            pop(REGS[i]);
        }
    }

    textSegment.push_back(new Move("rsp", "rbp"));
    textSegment.push_back(new Pop("rbp"));
    offset = 0;
    textSegment.push_back(new ReturnOp());

    if(!parameterStack.empty()) {
        parameters = parameterStack.top();
        parameterStack.pop();
    }
    func.pop();
}

void CodeGenVisitor::visitCallStatement(CallStatement* stmt) {
    for(int i = 0; i < std::size(usedRegs); i++) {
        if(usedRegs[i]) push(GPREGS[i+FIRST_ARG]);
    }
    usedRegStack.push(usedRegs);
    for(bool& b : usedRegs) b = false;

    if(stmt->id.name == "syscall") {
        for (int i = 0; i < stmt->arguments.size(); i++)
        {
            stmt->arguments.at(i)->accept(this, i+FIRST_ARG);
            usedRegs[i] = true;
        }

        textSegment.push_back(new Syscall());
    } else {
        for (int i = 0; i < stmt->arguments.size(); i++)
        {
            auto arg = stmt->arguments.at(i);
            arg->accept(this, i+FIRST_ARG+1);
            usedRegs[i+1] = true;
        }
        textSegment.push_back(new Call(stmt->id.name));
    }

    std::swap(usedRegs, usedRegStack.top());
    usedRegStack.pop();
    for(int i = std::size(usedRegs)-1; i >= 0; i--) {
        if(usedRegs[i]) {
            pop(GPREGS[i+FIRST_ARG]);
        }
    }
}

void CodeGenVisitor::visitVarAssignment(VarAssignment *stmt) {
    int left = allocator.allocate();
    int right = allocator.allocate();
    
    loadAddress = true;
    stmt->lhs->accept(this, left);
    loadAddress = false;
    stmt->rhs->accept(this, right);
    textSegment.push_back(new Move("[" + GPREGS[left] + "]", GPREGS[right]));

    allocator.free(left);
    allocator.free(right);
}

void CodeGenVisitor::visitVarDeclaration(VarDeclaration* stmt) {
    if(func.top() != nullptr) {
        push("qword 0");
        current->addVar(stmt->id, Var{offset, stmt->type});
    } else {
        if(stmt->size != nullptr) {
            IntLit* value = dynamic_cast<IntLit*>(stmt->size);
            if(value == nullptr) {
                throw std::runtime_error("expected IntLit but found: " + stmt->size->toString(0));
            }
            bssSegment.push_back(new DefineVar(stmt->id.name, "resb", std::to_string(value->value)));
            globalVars.insert({stmt->id.name, stmt->type});
            globals.push_back(stmt->id.name);
        } else {
            dataSegment.push_back(new DefineVar(stmt->id.name, "dq", "0"));
            globalVars.insert({stmt->id.name, stmt->type});
            globals.push_back(stmt->id.name);
        }
    }
}

void CodeGenVisitor::visitVarDeclAssign(VarDeclAssign* stmt) {
    if(func.top() != nullptr) {
        int r = allocator.allocate();
        stmt->value->accept(this, r);
        push(allocator.getReg(r));
        current->addVar(stmt->id, Var{offset, stmt->type});
        allocator.free(r);
    } else {
        IntLit* expr = dynamic_cast<IntLit*>(stmt->value);
        StringLit* str = dynamic_cast<StringLit*>(stmt->value);
        if(expr != nullptr) {
            if(stmt->constant)
                ROSegment.push_back(new DefineVar(stmt->id.name, "dq", std::to_string(expr->value)));
            else 
                dataSegment.push_back(new DefineVar(stmt->id.name, "dq", std::to_string(expr->value)));
            globalVars.insert({stmt->id.name, stmt->type});
            globals.push_back(stmt->id.name);
        } else if(str != nullptr) {
            if(stmt->constant)
                ROSegment.push_back(new DefineVar(stmt->id.name, "db", str->value));
            else
                dataSegment.push_back(new DefineVar(stmt->id.name, "db", str->value));
            globalVars.insert({stmt->id.name, stmt->type});
            globals.push_back(stmt->id.name);
        } else {
            std::cerr << "Expected either IntLit or StringLit after global assign but found: " << stmt->value->toString(0) << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void CodeGenVisitor::visitWhile(While* stmt)
{
    int r = allocator.allocate();

    int i = whileIndex++;
    textSegment.push_back(new Label(".while" + std::to_string(i) + "_start"));
    stmt->condition->accept(this, r);
    textSegment.push_back(new Compare(allocator.getReg(r), "0"));
    allocator.free(r);
    textSegment.push_back(new Jump("je", func.top()->id.name + ".while" + std::to_string(i) + "_end"));
    stmt->body->accept(this);
    textSegment.push_back(new Jump("jmp", func.top()->id.name + ".while" + std::to_string(i) + "_start"));
    textSegment.push_back(new Label(".while" + std::to_string(i) + "_end"));

    allocator.free(r);
}

void CodeGenVisitor::visitFunctionDefinition(FunctionDefinition *def) {
    globals.push_back(def->id.name);
    textSegment.push_back(new Label(def->id.name));
    textSegment.push_back(new Push("rbp"));
    textSegment.push_back(new Move("rbp", "rsp"));

    offsetStack.push(offset);
    offset = 0;

    CodeGenVisitor visit;
    visit.setParams(def->args);
    visit.pushFuncDef(def);
    visit.addGlobals(globalVars);

    def->body->accept(&visit);
    bool* wasUsed = visit.getScratchAlloctor()->getWasUsed();
    for(int i = 0; i < std::size(REGS); i++) {
        if(wasUsed[i]) {
            push(GPREGS[i]);
        }
    }

    for(auto op : visit.getTextSegment()) {
        textSegment.push_back(op);
    }
    for(auto op : visit.getDataSegment()) {
        dataSegment.push_back(op);
    }

    func.push(def);
}

void CodeGenVisitor::visitProgram(Program* prog) {
    for(auto glob : prog->externVars) {
        globalVars.insert(glob);
    }
}

void CodeGenVisitor::deref(const int depth, const std::string& reg)
{
    for (int i = 0 ; i < depth; i++)
    {
        textSegment.push_back(new Move(reg, "[" + reg + "]"));
    }
}

void CodeGenVisitor::makeType(const TypeIdentifierType type, const int reg)
{
    int r = allocator.allocate();
    switch (type)
    {
    case TypeIdentifierType::I64:
    case TypeIdentifierType::U64:
    case TypeIdentifierType::F64:
        break;
    case TypeIdentifierType::I8:
    case TypeIdentifierType::U8:
    case TypeIdentifierType::CHAR:
        textSegment.push_back(new XOR(ScratchAllocator::getReg(r), ScratchAllocator::getReg(r)));
        textSegment.push_back(new Move(ScratchAllocator::getReg8(r), GPREGS8[reg]));
        textSegment.push_back(new Move(GPREGS[reg], ScratchAllocator::getReg(r)));
        break;
    case TypeIdentifierType::I16:
    case TypeIdentifierType::U16:
        textSegment.push_back(new XOR(ScratchAllocator::getReg(r), ScratchAllocator::getReg(r)));
        textSegment.push_back(new Move(ScratchAllocator::getReg16(r), GPREGS16[reg]));
        textSegment.push_back(new Move(GPREGS8[reg], ScratchAllocator::getReg(r)));
        break;
    case TypeIdentifierType::I32:
    case TypeIdentifierType::U32:
    case TypeIdentifierType::F32:
        textSegment.push_back(new XOR(ScratchAllocator::getReg(r), ScratchAllocator::getReg(r)));
        textSegment.push_back(new Move(ScratchAllocator::getReg32(r), GPREGS32[reg]));
        textSegment.push_back(new Move(GPREGS[reg], ScratchAllocator::getReg(r)));
        break;
    case TypeIdentifierType::VOID:
    case TypeIdentifierType::BOOL:
        std::cerr << "Void and Bool types are unsupported right now" << std::endl;
        exit(EXIT_FAILURE);
      break;
    }
    allocator.free(r);
}


void CodeGenVisitor::setParams(std::map<std::string, FunctionDefinition::ParamData> p) {
    for(auto arg : p) {
        push(GPREGS[arg.second.index+FIRST_ARG+1]);
        current->addVar(Identifier{arg.first}, {offset, arg.second.type});
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

std::vector<OpCode*> CodeGenVisitor::getROSegment() {
    return ROSegment;
}

std::vector<OpCode*> CodeGenVisitor::getBssSegment() {
    return bssSegment;
}

std::vector<std::string> CodeGenVisitor::getGlobals() {
    return globals;
}