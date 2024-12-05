#include "Parser.hpp"

#include <cstddef>
#include <iostream>

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = tokens;
}

Program Parser::parse() {
    std::string str = consumeString().value();

    if(str == "fn") {
        auto id_opt = consumeString();
        if(!id_opt.has_value()) {
            std::cout << "Ya messed up bitch!" << std::endl;
            exit(EXIT_FAILURE);
        }
        Identifier id{id_opt.value()};
        if(!consume(LPAREN)) {
            std::cout << "Ya messed up bitch!" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        if(peek().type == RPAREN) {
            consume(RPAREN);
        } else {
            // TODO: implement arg list
        }

        if(!consume(RARROW)) {
            std::cout << "Ya messed up bitch!" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto typeId_opt = consumeString();

        if(!typeId_opt.has_value()) {
            std::cout << "Ya messed up bitch!" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto typeId = typeId_opt.value();
        TypeIdentifier type;

        if(typeId == "i8") {
            type = {TypeIdentifierType::I8};
        } else if(typeId == "i16") {
            type = {TypeIdentifierType::I16};
        } else if(typeId == "i32") {
            type = {TypeIdentifierType::I32};
        } else if(typeId == "i64") {
            type = {TypeIdentifierType::I64};
        } else if(typeId == "void") {
            type = {TypeIdentifierType::VOID};
        } else if(typeId == "char") {
            type = {TypeIdentifierType::CHAR};
        } else if(typeId == "f32") {
            type = {TypeIdentifierType::F32};
        } else if(typeId == "f64") {
            type = {TypeIdentifierType::F64};
        }

        Statement* body = parseStatement();

        return Program{FunctionDefinition(id, body, type, {})};
    }
}

Statement* Parser::parseStatement()
{
    if(peek().type == LCURLY) {
        consume(LCURLY);
        
        std::vector<Statement*> statements;
        while(peek().type != RCURLY) {
            statements.push_back(parseStatement());
        }

        return new Scope(statements);
    } else if(peek().type == IDENTIFIER) {
        std::string id = consumeString().value();

        if(id == "return") {
            Expression* expr = parseExpression();
            if(!consume(SEMI)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }
            return new Return(expr);
        } else {
            //std::cout << peek().toString() << std::endl;         
            if(!consume(LPAREN)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::vector<Expression*> args = parseArgs();
            
            if(!consume(RPAREN) || !consume(SEMI)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }

            return new CallStatement(Identifier{id}, args);
        }
    }

    return nullptr;
}

Expression* Parser::parseExpression()
{
    if(peek().type == STRING_LITERAL) {
        return new StringLit(consumeString().value());
    } else if(peek().type == INT_LIT) {
        return new IntLit(consumeInt().value());
    }

    return nullptr;
}

std::vector<Expression*> Parser::parseArgs()
{
    std::vector<Expression*> exprs;

    while(peek().type != RPAREN) {
        exprs.push_back(parseExpression());
    }

    return exprs;
}

bool Parser::consume(TokType type) {
    Token tok = tokens.at(0);
    tokens.erase(tokens.begin());
    return tok.type == type;
}

std::optional<int> Parser::consumeInt() {
    Token tok = tokens.at(0);
    tokens.erase(tokens.begin());
    return tok.intValue;
}

std::optional<std::string> Parser::consumeString() {
    Token tok = tokens.at(0);
    tokens.erase(tokens.begin());
    return tok.stringValue;
}

Token Parser::peek()
{
    return tokens.at(0);
}
