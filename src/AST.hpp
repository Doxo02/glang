#ifndef AST_HPP
#define AST_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <stack>

#include "OpCode.hpp"

class Visitor;

enum class TypeIdentifierType {
    I8, I16, I32, I64, VOID, CHAR, F32, F64, BOOL
};

enum class BinaryOperator {
    PLUS, MINUS, MUL, DIV
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
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor) = 0;

    int derefDepth = 0;
};

class IntLit : public Expression {
public:
    IntLit(int value) {
        this->value = value;
    }
    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IntLit: ");
        out.append(std::to_string(value));
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor) override;
    
    int value;
};

class StringLit : public Expression {
public:
    StringLit(std::string value) {
        this->value = value;
    }
    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("StringLit: ");
        out.append(value);
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor) override;
    
    std::string value;
};

class IdExpression : public Expression {
public:
    IdExpression(Identifier id) {
        this->id = id;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IdExpression: ");
        out.append(id.name);
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");
        return out;
    }

    void accept(Visitor* visitor) override;

    Identifier id;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(BinaryOperator op, Expression* left, Expression* right) {
        this->op = op;
        this->left = left;
        this->right = right;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
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
        }
        out.append("(");
        out.append(std::to_string(derefDepth));
        out.append(")");

        out.append("\n");
        out.append(left->toString(indentLevel + 1));
        out.append("\n");
        out.append(right->toString(indentLevel + 1));

        return out;
    }

    void accept(Visitor* visitor) override;

    BinaryOperator op;
    Expression* left;
    Expression* right;
};

class CallExpression : public Expression {
public:
    CallExpression(Identifier id, std::vector<Expression*> args) {
        this->id = id;
        this->args = args;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
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

    void accept(Visitor* visitor) override;

    Identifier id;
    std::vector<Expression*> args;
};

class Statement {
public:
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor) = 0;
};

class Compound : public Statement {
public:
    Compound(std::vector<Statement*> statements) {
        this->statements = statements;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
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

class EndCompound : public Statement {
public:
    EndCompound() {}

    std::string toString(int indentLevel) override {
        return "EndCompound\n";
    }

    void accept(Visitor* visitor) override;
};

class If : Statement {
public:
    Expression* condition;
    Statement* body;
};

class Return : public Statement {
public:
    Return(Expression* value) {
        this->value = value;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Return:\n");
        if(value != nullptr) out.append(value->toString(indentLevel + 1));

        return out;
    }

    void accept(Visitor* visitor) override;

    Expression* value;
};

class CallStatement : public Statement {
public:
    CallStatement(Identifier id, std::vector<Expression*> arguments) {
        this->id = id;
        this->arguments = arguments;
    }
    std::string toString(int indentLevel) override {
        std::string out = "";
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

class VarAssignment : public Statement {
public:
    VarAssignment(Identifier id, Expression* value) {
        this->id = id;
        this->value = value;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append(id.name);
        out.append(" = ");
        out.append(value->toString(0));
        return out;
    }

    void accept(Visitor* visitor) override;

    Identifier id;
    Expression* value;
};

class VarDeclaration : public Statement {
public:
    VarDeclaration(Identifier id, TypeIdentifier type) {
        this->id = id;
        this->type = type;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
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
};

class VarDeclAssign : public Statement {
public:
    VarDeclAssign(Identifier id, TypeIdentifier type, Expression* value) {
        this->id = id;
        this->type = type;
        this->value = value;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
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
};

class FunctionDefinition {
public:
    struct ParamData {
        TypeIdentifier type;
        int index;
    };

    FunctionDefinition(Identifier id, Statement* body, TypeIdentifier returnType, std::map<std::string, ParamData> args) {
        this->id = id;
        this->body = body;
        this->returnType = returnType;
        this->args = args;
    }

    std::string toString(int indentLevel) {
        std::string out = "";
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
        for(auto pair : args) {
            for(int i = 0; i < indentLevel+2; i++) out.append("  ");
            out.append(pair.first);
            out.append(": ");
            out.append(typeIdentifierTypeToString(pair.second.type.type));
            out.append("(");
            out.append(std::to_string(pair.second.type.ptrDepth));
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
    std::map<std::string, ParamData> args;
};

class Program {
public:
    void accept(Visitor* visitor);

    std::vector<VarDeclaration*> declarations;
    std::vector<FunctionDefinition*> functions;
};

struct Var {
    size_t offset;
    TypeIdentifier type;
};

class Scope {
public:
    Scope(Scope* parent) {
        this->parent = parent;
    }

    Scope* getParent() {
        return parent;
    }

    void addVar(Identifier id, Var info) {
        vars.insert({id.name, info});
    }

    Var getVar(Identifier id) {
        if(vars.find(id.name) == vars.cend()) {
            return parent->getVar(id);
        }

        return vars.find(id.name)->second;
    }

private:
    Scope* parent;
    std::map<std::string, Var> vars;
};

class Visitor {
public:
    virtual void visitIntLit(IntLit* expr) = 0;
    virtual void visitStringLit(StringLit* expr) = 0;
    virtual void visitIdExpression(IdExpression* expr) = 0;
    virtual void visitBinaryExpression(BinaryExpression* expr) = 0;
    virtual void visitCallExpression(CallExpression* expr) = 0;

    virtual void visitCompound(Compound* stmt) = 0;
    virtual void visitEndCompound(EndCompound* stmt) = 0;
    virtual void visitIf(If* stmt) = 0;
    virtual void visitReturn(Return* stmt) = 0;
    virtual void visitCallStatement(CallStatement* stmt) = 0;
    virtual void visitVarAssignment(VarAssignment* stmt) = 0;
    virtual void visitVarDeclaration(VarDeclaration* stmt) = 0;
    virtual void visitVarDeclAssign(VarDeclAssign* stmt) = 0;

    virtual void visitFunctionDefinition(FunctionDefinition* def) = 0;
    virtual void visitProgram(Program* prog) = 0;
};

class ConstExprVisitor : public Visitor {
public:
    void visitIntLit(IntLit* expr) override;
    void visitStringLit(StringLit* expr) override;
    void visitIdExpression(IdExpression* expr) override;
    void visitBinaryExpression(BinaryExpression* expr) override;

    void visitCompound(Compound* stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
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

class CountVarDeclVisitor : public Visitor {
public:
    CountVarDeclVisitor();
    void visitIntLit(IntLit* expr) override;
    void visitStringLit(StringLit* expr) override;
    void visitIdExpression(IdExpression* expr) override;
    void visitBinaryExpression(BinaryExpression* expr) override;

    void visitCompound(Compound *stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;
    void visitVarAssignment(VarAssignment* stmt) override;
    void visitVarDeclaration(VarDeclaration* stmt) override;
    void visitVarDeclAssign(VarDeclAssign* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

    int getNumVarDecls();
private:
    int varDecls = 0;
};

class CodeGenVisitor : public Visitor {
public:
    CodeGenVisitor();

    void visitIntLit(IntLit* expr) override;
    void visitStringLit(StringLit* expr) override;
    void visitIdExpression(IdExpression* expr) override;
    void visitBinaryExpression(BinaryExpression* expr) override;
    void visitCallExpression(CallExpression* expr) override;

    void visitCompound(Compound *stmt) override;
    void visitEndCompound(EndCompound* stmt) override;
    void visitIf(If* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;
    void visitVarAssignment(VarAssignment* stmt) override;
    void visitVarDeclaration(VarDeclaration* stmt) override;
    void visitVarDeclAssign(VarDeclAssign* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

    std::stack<size_t> getStack();

    std::vector<OpCode*> getDataSegment();
    std::vector<OpCode*> getTextSegment();

private:
    void push(std::string what, size_t bytes);
    void pop(std::string where, size_t bytes);

    Scope* root;
    Scope* current;

    std::stack<FunctionDefinition*> func;
    std::stack<size_t> offsetStack;
    std::stack<std::map<std::string, FunctionDefinition::ParamData>> parameterStack;

    std::map<std::string, FunctionDefinition::ParamData> parameters;

    std::vector<OpCode*> dataSegment;
    std::vector<OpCode*> textSegment;

    int stringIndex = 0;

    size_t offset = 0;
};

#endif