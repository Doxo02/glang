#ifndef AST_HPP
#define AST_HPP

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <stack>

#include "OpCode.hpp"
#include "ScratchAllocator.h"

class Visitor;

enum class TypeIdentifierType {
    I8, I16, I32, I64, U8, U16, U32, U64, VOID, CHAR, F32, F64, BOOL
};

inline std::string typeIdentifierTypeToString(TypeIdentifierType type) {
    switch(type) {
        case TypeIdentifierType::I8:
            return "I8";
        case TypeIdentifierType::I16:
            return "I16";
        case TypeIdentifierType::I32:
            return "I32";
        case TypeIdentifierType::I64:
            return "I64";
        case TypeIdentifierType::VOID:
            return "VOID";
        case TypeIdentifierType::CHAR:
            return "CHAR";
        case TypeIdentifierType::F32:
            return "F32";
        case TypeIdentifierType::F64:
            return "F64";
        case TypeIdentifierType::BOOL:
            return "BOOL";
        case TypeIdentifierType::U8:
            return "U8";
        case TypeIdentifierType::U16:
            return "U16";
        case TypeIdentifierType::U32:
            return "U32";
        case TypeIdentifierType::U64:
            return "U64";
        }
    return "";
}

struct TypeIdentifier {
    TypeIdentifierType type;
    int ptrDepth = 0;
};

struct Identifier {
    std::string name;
};

class Expression {
public:
    virtual ~Expression() = default;
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor, int reg) = 0;

    int derefDepth = 0;
    TypeIdentifier type;
    int lineNum = 0;
    int colNum = 0;
    std::string path;
};

class IntLit final : public Expression {
public:
    explicit IntLit(const int value, TypeIdentifierType type = TypeIdentifierType::I64) {
        this->value = value;
    }
    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IntLit: ");
        out.append(std::to_string(value));
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor, int reg) override;
    
    int value;
};

class CharLit final : public Expression {
public:
    CharLit(const char value, TypeIdentifierType type = TypeIdentifierType::CHAR) : value(value) {}

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("CharLit: ");
        out.append(std::string(1, value));
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor, int reg) override;

    const char value;
};

class StringLit final : public Expression {
public:
    explicit StringLit(const std::string& value, TypeIdentifierType type = TypeIdentifierType::U64) {
        this->value = value;
    }
    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("StringLit: ");
        out.append(value);
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor, int reg) override;
    
    std::string value;
};

class IdExpression final : public Expression {
public:
    explicit IdExpression(const Identifier& id, Expression* index = nullptr) {
        this->id = id;
        this->index = index;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IdExpression: ");
        out.append(id.name);
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        if(index != nullptr) {
            out.append("\n");
            for(int i = 0; i < indentLevel + 1; i++) out.append("  ");
            out.append("Index:\n");
            out.append(index->toString(indentLevel + 2));
        }
        return out;
    }

    void accept(Visitor* visitor, int reg) override;

    Identifier id;
    Expression* index;
};

class BinaryExpression final : public Expression {
public:
    BinaryExpression(const BinaryOperator op, Expression* left, Expression* right) {
        this->op = op;
        this->left = left;
        this->right = right;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        switch(op) {
            case BinaryOperator::PLUS:
                out.append("Plus: ");
                break;
            case BinaryOperator::MINUS:
                out.append("Minus: ");
                break;
            case BinaryOperator::MUL:
                out.append("Mul: ");
                break;
            case BinaryOperator::DIV:
                out.append("Div: ");
                break;
            case BinaryOperator::EQUALS:
                out.append("Equals: ");
                break;
            case BinaryOperator::NEQUALS:
                out.append("NotEquals: ");
                break;
            case BinaryOperator::LESS:
                out.append("Less: ");
                break;
            case BinaryOperator::GREATER:
                out.append("Greater: ");
                break;
            case BinaryOperator::LEQUALS:
                out.append("LessEquals: ");
                break;
            case BinaryOperator::GEQUALS:
                out.append("GreaterEquals: ");
                break;
            case BinaryOperator::BIT_AND:
                out.append("BitAnd: ");
                break;
            case BinaryOperator::BIT_OR:
                out.append("BitOr: ");
                break;
            case BinaryOperator::MOD:
                out.append("Mod: ");
                break;
            }
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")\n");
        out.append(left->toString(indentLevel + 1));
        out.append("\n");
        out.append(right->toString(indentLevel + 1));

        return out;
    }

    void accept(Visitor* visitor, int reg) override;

    BinaryOperator op;
    Expression* left;
    Expression* right;
};

class CallExpression : public Expression {
public:
    CallExpression(const Identifier& id, const std::vector<Expression*>& args) {
        this->id = id;
        this->args = args;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("CallExpression: ");
        out.append(id.name );
        out.append("\n");
        for(Expression* expr : args) {
            out.append(expr->toString(indentLevel + 1));
            out.append("\n");
        }
        out.erase(out.find_last_of('\n'));
        return out;
    }

    void accept(Visitor* visitor, int reg) override;

    Identifier id;
    std::vector<Expression*> args;
};

class Statement {
public:
    virtual ~Statement() = default;
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor) = 0;

    int lineNum = 0;
    int colNum = 0;
    std::string path;
};

class Compound final : public Statement {
public:
    explicit Compound(const std::vector<Statement*>& statements) {
        this->statements = statements;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Compound\n");

        for(Statement* statement : statements) {
            out.append(statement->toString(indentLevel + 1));
            out.append("\n");
        }
        out.erase(out.find_last_of('\n'));
        return out;
    }

    void accept(Visitor* visitor) override;

    std::vector<Statement*> statements;
};

class EndCompound final : public Statement {
public:
    EndCompound() = default;

    std::string toString(int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("EndCompound");
        return out;
    }

    void accept(Visitor* visitor) override;
};

class If : public Statement {
public:
    If(Expression* condition, Statement* body) {
        this->condition = condition;
        this->body = body;
    }

    std::string toString(int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("If:\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Condition:\n");
        out.append(condition->toString(indentLevel+2));
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("IfBody:\n");
        out.append(body->toString(indentLevel+2));

        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* condition;
    Statement* body;
};

class IfElse : public Statement {
public:
    IfElse(Expression* condition, Statement* ifBody, Statement* elseBody) {
        this->condition = condition;
        this->ifBody = ifBody;
        this->elseBody = elseBody;
    }

    std::string toString(int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IfElse:\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Condition:\n");
        out.append(condition->toString(indentLevel+2));
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("IfBody:\n");
        out.append(ifBody->toString(indentLevel+2));
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("ElseBody:\n");
        out.append(elseBody->toString(indentLevel+2));

        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* condition;
    Statement* ifBody;
    Statement* elseBody;
};

class Return final : public Statement {
public:
    explicit Return(Expression* value) {
        this->value = value;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Return:\n");
        if(value != nullptr) out.append(value->toString(indentLevel + 1));

        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* value;
};

class CallStatement final : public Statement {
public:
    CallStatement(const Identifier& id, const std::vector<Expression*>& arguments) {
        this->id = id;
        this->arguments = arguments;
    }
    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("CallStatement: ");
        out.append(id.name );
        out.append("\n");
        for(Expression* expr : arguments) {
            out.append(expr->toString(indentLevel + 1));
            out.append("\n");
        }
        out.erase(out.find_last_of('\n'));
        return out;
    }

    void accept(Visitor* visitor) override;
    
    Identifier id;
    std::vector<Expression*> arguments;
};

class VarAssignment final : public Statement {
public:
    VarAssignment(Expression* lhs, Expression* rhs) {
        this->lhs = lhs;
        this->rhs = rhs;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Assign:\n");
        for(int i = 0; i < indentLevel + 1; i++) out.append("  ");
        out.append("Left:\n");
        out.append(lhs->toString(indentLevel + 2));
        out.append(":\n");
        for(int i = 0; i < indentLevel + 1; i++) out.append("  ");
        out.append("Right:\n");
        out.append(rhs->toString(indentLevel + 2));
        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* lhs;
    Expression* rhs;
};

class VarDeclaration final : public Statement {
public:
    VarDeclaration(const Identifier& id, const TypeIdentifier type, Expression* size = nullptr) {
        this->id = id;
        this->type = type;
        this->size = size;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append(id.name);
        out.append(": ");
        out.append(typeIdentifierTypeToString(type.type));
        out.append("(");
        out.append(std::to_string(type.ptrDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor) override;
    
    Identifier id;
    TypeIdentifier type;
    Expression* size;
};

class VarDeclAssign final : public Statement {
public:
    VarDeclAssign(const Identifier& id, const TypeIdentifier type, Expression* value, bool constant = false) {
        this->id = id;
        this->type = type;
        this->value = value;
        this->constant = constant;
    }

    std::string toString(const int indentLevel) override {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append(id.name);
        out.append(": ");
        out.append(typeIdentifierTypeToString(type.type));
        out.append("(");
        out.append(std::to_string(type.ptrDepth));
        out.append(")");
        out.append(" =\n");
        out.append(value->toString(indentLevel + 1));
        return out;
    }

    void accept(Visitor* visitor) override;

    Identifier id;
    TypeIdentifier type;
    Expression* value;
    bool constant;
};

class While final : public Statement
{
public:
    While(Expression* condition, Statement* body) {
        this->condition = condition;
        this->body = body;
    }

    std::string toString(const int indentLevel) override
    {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("While:\n");
        for (int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Condition:\n");
        out.append(condition->toString(indentLevel+2));
        out.append("\n");
        for (int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Body:\n");
        out.append(body->toString(indentLevel+2));
        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* condition;
    Statement* body;
};

class FunctionDefinition {
public:
    struct ParamData {
        std::string name;
        TypeIdentifier type;
        int index;
    };

    FunctionDefinition(const Identifier& id, Statement* body, const TypeIdentifier returnType, const std::vector<ParamData>& args) {
        this->id = id;
        this->body = body;
        this->returnType = returnType;
        this->args = args;
    }

    [[nodiscard]] std::string toString(const int indentLevel) const
    {
        std::string out;
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Function: ");
        out.append(id.name);
        out.append("\n");
        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("ReturnType: ");
        out.append(typeIdentifierTypeToString(returnType.type));
        out.append("(");
        out.append(std::to_string(returnType.ptrDepth));
        out.append(")");
        out.append("\n");

        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Args:\n");
        for(const auto& data : args) {
            for(int i = 0; i < indentLevel+2; i++) out.append("  ");
            out.append(data.name);
            out.append(": ");
            out.append(typeIdentifierTypeToString(data.type.type));
            out.append("(");
            out.append(std::to_string(data.type.ptrDepth));
            out.append(")");
            out.append("\n");
        }

        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Body:\n");
        out.append(body->toString(indentLevel+2));

        return out;
    }
    
    void accept(Visitor* visitor);

    Identifier id;
    Statement* body;
    TypeIdentifier returnType;
    std::vector<ParamData> args;

    int lineNum = 0;
    int colNum = 0;
    std::string path;
};

class Program {
public:
    void accept(Visitor* visitor);

    inline void addExtern(std::string label) {
        externs.push_back(label);
    }

    inline std::vector<std::string> getExterns() {
        return externs;
    }

    inline void addExtern(std::string label, TypeIdentifier type) {
        externVars.insert({label, type});
    }

    std::vector<VarDeclaration*> declarations;
    std::vector<VarDeclAssign*> declAssigns;
    std::vector<FunctionDefinition*> functions;
    std::vector<std::string> externs;
    std::map<std::string, TypeIdentifier> externVars;
    std::map<std::string, FunctionDefinition*> externFunctions;
};

struct Var {
    size_t offset;
    TypeIdentifier type;
};

class Scope {
public:
    explicit Scope(Scope* parent) {
        this->parent = parent;
    }

    [[nodiscard]] Scope* getParent() const
    {
        return parent;
    }

    void addVar(const Identifier& id, Var info) {
        vars.insert({id.name, info});
    }

    Var* getVar(const Identifier& id) {
        if(!vars.contains(id.name)) {
            if(parent != nullptr)
                return parent->getVar(id);
            return nullptr;
        }

        return &vars.find(id.name)->second;
    }

    [[nodiscard]] int getNumVars() const
    {
        return vars.size();
    }

private:
    Scope* parent;
    std::map<std::string, Var> vars;
};

class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visitIntLit(IntLit* expr, int reg) = 0;
    virtual void visitStringLit(StringLit* expr, int reg) = 0;
    virtual void visitCharLit(CharLit* expr, int reg) = 0;
    virtual void visitIdExpression(IdExpression* expr, int reg) = 0;
    virtual void visitBinaryExpression(BinaryExpression* expr, int reg) = 0;
    virtual void visitCallExpression(CallExpression* expr, int reg) = 0;

    virtual void visitCompound(Compound* stmt) = 0;
    virtual void visitEndCompound(EndCompound* stmt) = 0;
    virtual void visitIf(If* stmt) = 0;
    virtual void visitIfElse(IfElse* stmt) = 0;
    virtual void visitReturn(Return* stmt) = 0;
    virtual void visitCallStatement(CallStatement* stmt) = 0;
    virtual void visitVarAssignment(VarAssignment* stmt) = 0;
    virtual void visitVarDeclaration(VarDeclaration* stmt) = 0;
    virtual void visitVarDeclAssign(VarDeclAssign* stmt) = 0;
    virtual void visitWhile(While* stmt) = 0;

    virtual void visitFunctionDefinition(FunctionDefinition* def) = 0;
    virtual void visitProgram(Program* prog) = 0;
};

class ConstExprVisitor : public Visitor {
public:
    void visitIntLit(IntLit* expr, int reg) override;
    void visitStringLit(StringLit* expr, int reg) override;
    void visitCharLit(CharLit* expr, int reg) override;
    void visitIdExpression(IdExpression* expr, int reg) override;
    void visitBinaryExpression(BinaryExpression* expr, int reg) override;

    void visitCompound(Compound* stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
    void visitIfElse(IfElse* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;
    void visitVarAssignment(VarAssignment* stmt) override;
    void visitVarDeclaration(VarDeclaration* stmt) override;
    void visitVarDeclAssign(VarDeclAssign* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

private:
    std::stack<std::optional<int>> stack;
};

class TypeChecker : public Visitor
{
public:
    TypeChecker();

    void visitIntLit(IntLit* expr, int reg) override;
    void visitStringLit(StringLit* expr, int reg) override;
    void visitCharLit(CharLit* expr, int reg) override;
    void visitIdExpression(IdExpression* expr, int reg) override;
    void visitBinaryExpression(BinaryExpression* expr, int reg) override;
    void visitCallExpression(CallExpression* expr, int reg) override;
    void visitCompound(Compound* stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
    void visitIfElse(IfElse* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;
    void visitVarAssignment(VarAssignment* stmt) override;
    void visitVarDeclaration(VarDeclaration* stmt) override;
    void visitVarDeclAssign(VarDeclAssign* stmt) override;
    void visitWhile(While* stmt) override;
    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

    void addGlobals(const std::map<std::string, TypeIdentifier>& globals) {
        for(const auto& pair : globals) {
            globalVars.insert(pair);
        }
    }

    void addFunctions(const std::map<std::string, FunctionDefinition*>& functions) {
        for(const auto& pair : functions) {
            this->functions.insert(pair);
        }
    }
private:
    void checkFunctionCall(const FunctionDefinition* function, const std::vector<Expression*>& args, bool isExpr = false);

    std::stack<TypeIdentifier> typeStack;
    std::map<std::string, TypeIdentifier> globalVars;
    std::vector<FunctionDefinition::ParamData> parameters;
    std::stack<std::map<std::string, FunctionDefinition::ParamData>> parameterStack;
    std::map<std::string, FunctionDefinition*> functions;
    std::map<std::string, FunctionDefinition*> externFunctions;

    Scope* root;
    Scope* current;

    FunctionDefinition* currentFunction;
};

class CodeGenVisitor final : public Visitor {
public:
    CodeGenVisitor();

    void visitIntLit(IntLit* expr, int reg) override;
    void visitStringLit(StringLit* expr, int reg) override;
    void visitCharLit(CharLit* expr, int reg) override;
    void visitIdExpression(IdExpression* expr, int reg) override;
    void visitBinaryExpression(BinaryExpression* expr, int reg) override;
    void visitCallExpression(CallExpression* expr, int reg) override;

    void visitCompound(Compound *stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
    void visitIfElse(IfElse* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;
    void visitVarAssignment(VarAssignment* stmt) override;
    void visitVarDeclaration(VarDeclaration* stmt) override;
    void visitVarDeclAssign(VarDeclAssign* stmt) override;
    void visitWhile(While* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

    std::stack<size_t> getStack();

    std::vector<OpCode*> getDataSegment();
    std::vector<OpCode*> getTextSegment();
    std::vector<OpCode*> getBssSegment();
    std::vector<OpCode*> getROSegment();
    std::vector<std::string> getGlobals();

    ScratchAllocator* getScratchAlloctor() { return &allocator; }
    void setParams(std::vector<FunctionDefinition::ParamData> p);
    inline void addGlobals(std::map<std::string, TypeIdentifier> globals) {
        for(auto pair : globals) {
            globalVars.insert(pair);
        }
    }

    void pushFuncDef(FunctionDefinition* funcDef) { func.push(funcDef); }

private:
    void push(const std::string& what, size_t bytes);
    void pop(const std::string& where, size_t bytes);

    int getReg();

    void deref(int depth, int typeDepth, const std::string& reg, const std::string& addr);
    void makeType(TypeIdentifierType type, int reg);

    Scope* root;
    Scope* current;

    std::stack<FunctionDefinition*> func;
    std::stack<size_t> offsetStack;
    std::stack<std::map<std::string, FunctionDefinition::ParamData>> parameterStack;
    std::stack<std::array<bool, 7>> usedRegStack;

    std::map<std::string, FunctionDefinition::ParamData> parameters;
    std::map<std::string, TypeIdentifier> globalVars;

    std::vector<OpCode*> dataSegment;
    std::vector<OpCode*> textSegment;
    std::vector<OpCode*> bssSegment;
    std::vector<OpCode*> ROSegment;
    std::vector<std::string> globals;

    int stringIndex = 0;
    int whileIndex = 0;
    int ifIndex = 0;

    size_t offset = 0;

    ScratchAllocator allocator;

    bool loadAddress = false;
};

#endif