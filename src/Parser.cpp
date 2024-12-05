#include "Parser.hpp"
#include "AST.hpp"
#include "Token.hpp"

#include <iostream>
#include <ostream>
#include <vector>

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = tokens;
}

Program* Parser::parse() {
    std::vector<FunctionDefinition*> defs;
    while(counter < tokens.size()) {
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

            defs.push_back(new FunctionDefinition(id, body, type, {}));
        }
    }
    return new Program{defs};
}

Statement* Parser::parseStatement()
{
    if(peek().type == LCURLY) {
        consume(LCURLY);
        
        std::vector<Statement*> statements;
        while(peek().type != RCURLY) {
            statements.push_back(parseStatement());
        }

        consume(RCURLY);

        return new Scope(statements);
    } else if(peek().type == IDENTIFIER) {
        std::string id = consumeString().value();

        if(id == "return") {
            Expression* expr = parseExpression(findNext(SEMI, tokens.size()));
            if(!consume(SEMI)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }
            return new Return(expr);
        } else {  
            if(!consume(LPAREN)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::vector<Expression*> args = parseArgs(findEndParen());
            
            if(!consume(RPAREN) || !consume(SEMI)) {
                std::cout << "Ya messed up bitch!" << std::endl;
                exit(EXIT_FAILURE);
            }

            return new CallStatement(Identifier{id}, args);
        }
    }

    return nullptr;
}

Expression* Parser::parseExpression(int until)
{
    return parseAddSub(until);
}

Expression* Parser::parseParen(int until) {
    if(peek().type != LPAREN) return parseSingle(until);

    consume(LPAREN);
    Expression* expr = parseAddSub(findEndParen());
    consume(RPAREN);

    return expr;
}

Expression* Parser::parseSingle(int until) {
    if(peek().type == INT_LIT) {
        return new IntLit(consumeInt().value());
    } else if(peek().type == STRING_LITERAL) {
        return new StringLit(consumeString().value());
    }
}

Expression* Parser::parseAddSub(int until) {
    int nextPlus = findNextOutsideParen(PLUS, until);
    int nextMinus = findNextOutsideParen(MINUS, until);

    if(nextPlus == -1 && nextMinus == -1) return parseMulDiv(until);

    int next;
    BinaryOperator op;
    if(nextPlus > nextMinus && nextMinus != -1) {
        next = nextMinus;
        op = BinaryOperator::MINUS;
    } else {
        next = nextPlus;
        op = BinaryOperator::PLUS;
    }

    Expression* left = parseMulDiv(next);
    if(op == BinaryOperator::PLUS) consume(PLUS);
    else if(op == BinaryOperator::MINUS) consume(MINUS);
    Expression* right = parseAddSub(until);

    return new BinaryExpression(op, left, right);
}

Expression* Parser::parseMulDiv(int until) {
    int nextMul = findNextOutsideParen(MUL, until);
    int nextDiv = findNextOutsideParen(DIV, until);

    if(nextMul == -1 && nextDiv == -1) return parseParen(until);

    int next;
    BinaryOperator op;
    if(nextMul > nextDiv && nextDiv != -1) {
        next = nextDiv;
        op = BinaryOperator::DIV;
    } else {
        next = nextMul;
        op = BinaryOperator::MUL;
    }

    Expression* left = parseParen(next);
    if(op == BinaryOperator::MUL) consume(MUL);
    else if(op == BinaryOperator::DIV) consume(DIV);
    Expression* right = parseMulDiv(until);

    return new BinaryExpression(op, left, right);
}

std::vector<Expression*> Parser::parseArgs(int until)
{
    std::vector<Expression*> exprs;

    for(int i = counter; i < until && peek().type != RPAREN; i++) {
        int nextComma = findNext(COMMA, until);
        
        int next;
        
        bool foundNextComma = true;
        if(nextComma == -1){
            next = until;
            foundNextComma = false;
        } else next = nextComma;
        
        exprs.push_back(parseExpression(next));
        if(foundNextComma) consume(COMMA);
    }

    return exprs;
}

bool Parser::consume(TokType type) {
    Token tok = tokens.at(counter++);
    return tok.type == type;
}

std::optional<int> Parser::consumeInt() {
    Token tok = tokens.at(counter++);
    return tok.intValue;
}

std::optional<std::string> Parser::consumeString() {
    Token tok = tokens.at(counter++);
    return tok.stringValue;
}

Token Parser::peek()
{
    return tokens.at(counter);
}

int Parser::findNext(TokType type, int until) {
    for(int i = counter; i < until; i++) {
        if(tokens.at(i).type == type) return i;
    }
    return -1;
}

int Parser::findNextOutsideParen(TokType type, int until) {
    int openings = 0;
    for(int i = counter; i < until; i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == RPAREN && openings > 0) openings--;

        if(tokens.at(i).type == type && openings == 0) return i;
    }

    return -1;
}

int Parser::findEndParen() {
    int openings = 0;
    for(int i = counter; i < tokens.size(); i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == RPAREN) {
            if(openings == 0) return i;
            else openings--;
        }
    }
}