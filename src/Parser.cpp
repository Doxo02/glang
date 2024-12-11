#include "Parser.hpp"
#include "AST.hpp"
#include "Token.hpp"

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = std::move(tokens);
}

std::map<std::string, FunctionDefinition::ParamData> Parser::parseParameters() {
    std::map<std::string, FunctionDefinition::ParamData> args;
    int index = 0;
    while(peek().type != RPAREN) {
        if(peek().type != IDENTIFIER) {
            std::cerr << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string id = consumeString().value();

        consume(COLON, std::to_string(peek().line) + ": expected ':' but found: " + peek().toString());

        if(peek().type != IDENTIFIER) {
            std::cerr << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
            exit(EXIT_FAILURE);
        }
        TypeIdentifierType t = strToTypeId(consumeString().value());
        int ptrDepth = 0;
        while(peek().type == STAR) {
            consume(STAR, "");
            ptrDepth++;
        }
        args.insert({id, {TypeIdentifier{t, ptrDepth}, index++}});
        if(peek().type != RPAREN) {
            consume(COMMA, std::to_string(peek().line) + ": expected ',' but found: " + peek().toString());
        }
    }

    consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());

    return args;
}

Program* Parser::parse() {
    std::vector<FunctionDefinition*> defs;
    std::vector<VarDeclaration*> decls;
    while(counter < tokens.size()) {
        if(std::string str = consumeString().value(); str == "fn") {
            if(peek().type != IDENTIFIER) {
                std::cerr << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
                exit(EXIT_FAILURE);
            }
            const Identifier id{consumeString().value()};
            consume(LPAREN, std::to_string(peek().line) + ": expected '(' but found: " + peek().toString());

            const auto args = parseParameters();

            consume(RARROW, std::to_string(peek().line) + ": expected '->' but found: " + peek().toString());

            if(peek().type != IDENTIFIER) {
                std::cerr << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
                exit(EXIT_FAILURE);
            }

            const auto typeId = consumeString().value();
            const auto type = TypeIdentifier{strToTypeId(typeId)};

            Statement* body = parseStatement(true);

            defs.push_back(new FunctionDefinition(id, body, type, args));
        } else if(str == "let") {
            const Identifier id{consumeString().value()};
            consume(COLON, std::to_string(peek().line) + ": expected ':' but found: " + peek().toString());
            const auto type = TypeIdentifier{strToTypeId(consumeString().value())};
            consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());
            decls.push_back(new VarDeclaration(id, type));
        }
    }
    return new Program{decls, defs};
}

Statement* Parser::parseStatement(const bool funcBody) {
    if(peek().type == LCURLY) {
        const unsigned int line = peek().line;
        consume(LCURLY, "");
        
        std::vector<Statement*> statements;
        while(counter != tokens.size() && peek().type != RCURLY) {
            statements.push_back(parseStatement());
        }

        if(funcBody && dynamic_cast<Return*>(statements.back()) == nullptr) {
            statements.push_back(new Return(nullptr));
        }

        consume(RCURLY, std::to_string(line) + ": Compound Statement never ends");

        statements.push_back(new EndCompound());
        return new Compound(statements);
    } else if(peek().type == IDENTIFIER) {
        std::string id = consumeString().value();

        if(id == "return") {
            Expression* expr = parseExpression(findNext(SEMI, static_cast<int>(tokens.size())));
            consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());
            return new Return(expr);
        } else if(id == "let") {
            const Identifier identifier{consumeString().value()};
            consume(COLON, std::to_string(peek().line) + ": expected ':' but found: " + peek().toString());

            if(peek().type != IDENTIFIER) {
                std::cerr << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string typeStr = consumeString().value();
            int ptrDepth = 0;
            while(peek().type == STAR) {
                consume(STAR, "");
                ptrDepth++;
            }
            const auto type = TypeIdentifier{strToTypeId(typeStr), ptrDepth};
            if(peek().type == SEMI) {
                consume(SEMI, "");
                return new VarDeclaration(identifier, type);
            } else {
                consume(ASSIGN, std::to_string(peek().line) + ": expected '=' but found: " + peek().toString());
                Expression* expr = parseExpression(findNext(SEMI, static_cast<int>(tokens.size())));
                consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());

                return new VarDeclAssign(Identifier{identifier}, type, expr);
            }
        } else {  
            if(peek().type == LPAREN) {
                consume(LPAREN, "");
                const std::vector<Expression*> args = parseArgs(findEndParen());
                
                consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());
                consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());

                return new CallStatement(Identifier{id}, args);
            } else if(peek().type == ASSIGN) {
                consume(ASSIGN, "");
                Expression* val = parseExpression(findNext(SEMI, static_cast<int>(tokens.size())));
                consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());

                return new VarAssignment(Identifier{id}, val);
            }
        }
    }

    return nullptr;
}

Expression* Parser::parseExpression(const int until)
{
    return parseAddSub(until);
}

Expression* Parser::parseCallExpression(const int until) {
    if(peek().type != IDENTIFIER || peek(1).type != LPAREN) return parseParen(until);
    Identifier id{consumeString().value()};
    
    consume(LPAREN, "");
    std::vector<Expression*> args = parseArgs(findEndParen());
    consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());

    return new CallExpression(id, args);
}

Expression* Parser::parseParen(const int until) {
    if(peek().type != LPAREN) return parseSingle();

    consume(LPAREN, "");
    Expression* expr = parseAddSub(findEndParen());
    consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());

    return expr;
}

Expression* Parser::parseSingle() {
    if(peek().type == INT_LIT) {
        return new IntLit(consumeInt().value());
    } else if(peek().type == STRING_LITERAL) {
        return new StringLit(consumeString().value());
    } else if(peek().type == IDENTIFIER) {
        Identifier id{consumeString().value()};

        return new IdExpression(id);
    } else {
        std::cerr << peek().line << ": expected INT_LIT, STRING_LIT or IDENTIFIER but found: " << peek().toString() << std::endl;
    }
    return nullptr;
}

Expression* Parser::parseAddSub(const int until) {
    int nextPlus = findNextOutsideParen(PLUS, until);
    int nextMinus = findNextOutsideParen(MINUS, until);

    if(nextPlus == -1 && nextMinus == -1) return parseMulDiv(until);

    int next;
    BinaryOperator op;
    if((nextPlus > nextMinus && nextMinus != -1) || nextPlus == -1) {
        next = nextMinus;
        op = BinaryOperator::MINUS;
    } else {
        next = nextPlus;
        op = BinaryOperator::PLUS;
    }

    Expression* left = parseMulDiv(next);
    if(op == BinaryOperator::PLUS) consume(PLUS, std::to_string(peek().line) + ": expected '+' but found: " + peek().toString());
    else consume(MINUS, std::to_string(peek().line) + ": expected '-' but found: " + peek().toString());
    Expression* right = parseAddSub(until);

    return new BinaryExpression(op, left, right);
}

Expression* Parser::parseMulDiv(int until) {
    int derefDepth = 0;
    while(peek().type == STAR) {
        derefDepth++;
        consume(STAR, "");
    }
    const int nextMul = findNextOutsideParen(STAR, until);
    const int nextDiv = findNextOutsideParen(FSLASH, until);

    if(nextMul == -1 && nextDiv == -1) {
        Expression* expr = parseCallExpression(until);
        expr->derefDepth = derefDepth;
        return expr;
    }

    int next;
    BinaryOperator op;
    if(nextMul > nextDiv && nextDiv != -1) {
        next = nextDiv;
        op = BinaryOperator::DIV;
    } else {
        next = nextMul;
        op = BinaryOperator::MUL;
    }

    Expression* left = parseCallExpression(next);
    if(op == BinaryOperator::MUL) consume(STAR, std::to_string(peek().line) + ": expected '*' but found: " + peek().toString());
    else consume(FSLASH, std::to_string(peek().line) + ": expected '/' but found: " + peek().toString());
    Expression* right = parseCallExpression(until);

    auto* expr = new BinaryExpression(op, left, right);
    expr->derefDepth = derefDepth;

    return expr;
}

std::vector<Expression*> Parser::parseArgs(const int until)
{
    std::vector<Expression*> exprs;

    for(int i = counter; i < until && peek().type != RPAREN; i++) {
        const int nextComma = findNext(COMMA, until);
        
        int next;
        
        bool foundNextComma = true;
        if(nextComma == -1){
            next = until;
            foundNextComma = false;
        } else next = nextComma;
        
        exprs.push_back(parseExpression(next));
        if(foundNextComma) consume(COMMA, std::to_string(peek().line) + ": expected ',' but found: " + peek().toString());
    }

    return exprs;
}

void Parser::consume(const TokType type, const std::string& errorMsg) {
    if(const Token tok = tokens.at(counter++); tok.type != type) {
        std::cerr << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::optional<int> Parser::consumeInt() {
    const Token tok = tokens.at(counter++);
    return tok.intValue;
}

std::optional<std::string> Parser::consumeString() {
    Token tok = tokens.at(counter++);
    return tok.stringValue;
}

Token Parser::peek(int i)
{
    return tokens.at(counter+i);
}

int Parser::findNext(const TokType type, const int until) const
{
    for(int i = counter; i < until; i++) {
        if(tokens.at(i).type == type) return i;
    }
    return -1;
}

int Parser::findNextOutsideParen(const TokType type, const int until) const
{
    int openings = 0;
    for(int i = counter; i < until; i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == RPAREN && openings > 0) openings--;

        if(tokens.at(i).type == type && openings == 0) return i;
    }

    return -1;
}

int Parser::findEndParen() const
{
    int openings = 0;
    for(int i = counter; i < tokens.size(); i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == RPAREN) {
            if(openings == 0) return i;
            else openings--;
        }
    }

    return -1;
}

// ReSharper disable once CppNotAllPathsReturnValue
TypeIdentifierType Parser::strToTypeId(const std::string& str) {
    if(str == "i8") {
        return TypeIdentifierType::I8;
    }
    if(str == "i16") {
        return TypeIdentifierType::I16;
    }
    if(str == "i32") {
        return TypeIdentifierType::I32;
    }
    if(str == "i64") {
        return TypeIdentifierType::I64;
    }
    if(str == "void") {
        return TypeIdentifierType::VOID;
    }
    if(str == "char") {
        return TypeIdentifierType::CHAR;
    }
    if(str == "f32") {
        return TypeIdentifierType::F32;
    }
    if(str == "f64") {
        return TypeIdentifierType::F64;
    }
}
