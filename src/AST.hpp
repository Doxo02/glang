#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <map>
#include <stack>

#include "OpCode.hpp"

class Visitor;

enum class TypeIdentifierType {
    I8, I16, I32, I64, VOID, CHAR, F32, F64
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
    }
    return "";
}

struct TypeIdentifier {
    TypeIdentifierType type;
};

struct Identifier {
    std::string name;
};

class Expression {
public:
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor) = 0;
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
        return out;
    }

    void accept(Visitor* visitor) override;
    
    std::string value;
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

class Statement {
public:
    virtual std::string toString(int indentLevel) = 0;

    virtual void accept(Visitor* visitor) = 0;
};

class Scope : public Statement {
public:
    Scope(std::vector<Statement*> statements) {
        this->statements = statements;
    }

    std::string toString(int indentLevel) override {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Scope\n");

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
        out.append(value->toString(indentLevel + 1));

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

class FunctionDefinition {
public:
    FunctionDefinition(Identifier id, Statement* body, TypeIdentifier returnType, std::map<Identifier, TypeIdentifier> args) {
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
        out.append("\n");

        for(int i = 0; i < indentLevel+1; i++) out.append("  ");
        out.append("Args:\n");
        for(auto pair : args) {
            for(int i = 0; i < indentLevel+2; i++) out.append("  ");
            out.append(pair.first.name);
            out.append(": ");
            out.append(typeIdentifierTypeToString(pair.second.type));
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
    std::map<Identifier, TypeIdentifier> args;
};

class Program {
public:
    void accept(Visitor* visitor);

    std::vector<FunctionDefinition*> functions;
};

class Visitor {
public:
    virtual void visitIntLit(IntLit* expr) = 0;
    virtual void visitStringLit(StringLit* expr) = 0;
    virtual void visitBinaryExpression(BinaryExpression* expr) = 0;

    virtual void visitScope(Scope* stmt) = 0;
    virtual void visitIf(If* stmt) = 0;
    virtual void visitReturn(Return* stmt) = 0;
    virtual void visitCallStatement(CallStatement* stmt) = 0;

    virtual void visitFunctionDefinition(FunctionDefinition* def) = 0;
    virtual void visitProgram(Program* prog) = 0;
};

class PrintVisitor : public Visitor {
public:
    void visitIntLit(IntLit* expr) override;
    void visitStringLit(StringLit* expr) override;

    void visitScope(Scope* stmt) override;
    void visitIf(If* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;
};

class CodeGenVisitor : public Visitor {
public:
    void visitIntLit(IntLit* expr) override;
    void visitStringLit(StringLit* expr) override;
    void visitBinaryExpression(BinaryExpression* expr) override;

    void visitScope(Scope *stmt) override;
    void visitIf(If* stmt) override;
    void visitReturn(Return* stmt) override;
    void visitCallStatement(CallStatement* stmt) override;

    void visitFunctionDefinition(FunctionDefinition* def) override;
    void visitProgram(Program* prog) override;

    std::stack<std::string> getStack();

    std::vector<OpCode*> getDataSegment();
    std::vector<OpCode*> getTextSegment();

private:
    std::stack<std::string> stack;

    std::vector<OpCode*> dataSegment;
    std::vector<OpCode*> textSegment;

    int stringIndex = 0;
};

#endif