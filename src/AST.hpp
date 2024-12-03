#ifndef AST_HPP
#define AST_HPP

#include "Token.hpp"
#include <vector>
#include <map>
#include <iostream>

enum class TypeIdentifierType {
    I8, I16, I32, I64, VOID, CHAR, F32, F64
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
    virtual std::string toString(int indentLevel) {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Expression");
        return out;
    }
};

class IntLit : public Expression {
public:
    IntLit(int value) {
        this->value = value;
    }
    virtual std::string toString(int indentLevel) {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("IntLit: ");
        out.append(std::to_string(value));
        return out;
    }
    int value;
};

class StringLit : public Expression {
public:
    StringLit(std::string value) {
        this->value = value;
    }
    virtual std::string toString(int indentLevel) {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("StringLit: ");
        out.append(value);
        return out;
    }
    std::string value;
};

class Statement {
public:
    virtual std::string toString(int indentLevel) {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Statement");
        return out;
    }
};

class Scope : public Statement {
public:
    Scope(std::vector<Statement*> statements) {
        this->statements = statements;
    }

    virtual std::string toString(int indentLevel) {
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

    virtual std::string toString(int indentLevel) {
        std::string out = "";
        for(int i = 0; i < indentLevel; i++) out.append("  ");
        out.append("Return: ");
        out.append(value->toString(0));

        return out;
    }

    Expression* value;
};

class CallStatement : public Statement {
public:
    CallStatement(Identifier id, std::vector<Expression*> arguments) {
        this->id = id;
        this->arguments = arguments;
    }
    virtual std::string toString(int indentLevel) {
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
    
    Identifier id;
    Statement* body;
    TypeIdentifier returnType;
    std::map<Identifier, TypeIdentifier> args;
};

class Program {
public:
    FunctionDefinition main;
};

#endif