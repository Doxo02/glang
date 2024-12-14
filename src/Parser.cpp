#include "Parser.hpp"

#include <cstddef>
#include <fstream>

#include "AST.hpp"
#include "Token.hpp"

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include "Lexer.hpp"

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
    auto* program = new Program();
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

            program->functions.push_back(new FunctionDefinition(id, body, type, args));
        }
        else if(str == "let") {
            const Identifier id{consumeString().value()};
            consume(COLON, std::to_string(peek().line) + ": expected ':' but found: " + peek().toString());
            const auto type = TypeIdentifier{strToTypeId(consumeString().value())};
            consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());
            program->declarations.push_back(new VarDeclaration(id, type));
        }
        else if (str == "import") {
            consume(LPAREN, std::to_string(peek().line) + ": expected '(' but found: " + peek().toString());
            if (peek().type != STRING_LITERAL) {
                std::cerr << peek().line << ": expected STRING_LITERAL but found: " << peek().toString() << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string path = consumeString().value();
            consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());
            consume(SEMI, std::to_string(peek().line) + ": expected ';' but found: " + peek().toString());

            if (path.find(".glang") == std::string::npos) {
                path.append(".glang");
            }

            std::fstream file(path);
            std::string line;
            unsigned int number = 1;
            Lexer lexer;
            while(std::getline(file, line)) {
                lexer.passLine(line, number++);
            }
            file.close();
            Parser parser(lexer.getTokens());
            Program* import_prog = parser.parse();

            for (FunctionDefinition* def : import_prog->functions) {
                program->functions.push_back(def);
            }
            for (VarDeclaration* decl : import_prog->declarations) {
                program->declarations.push_back(decl);
            }
        }
        else {
            std::cerr << peek().line << ": expected 'fn', 'let' or 'import' but found: " << peek().toString() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return program;
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
        }
        if(id == "let") {
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
        }
        if (id == "while") {
            consume(LPAREN, std::to_string(peek().line) + ": expected '(' but found: " + peek().toString());
            Expression* condition = parseExpression(findEndParen());
            consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());
            Statement* body = parseStatement();
            return new While(condition, body);
        }
        if(id == "if") {
            consume(LPAREN, std::to_string(peek().line) + ": expected '(' but found: " + peek().toString());
            Expression* condition = parseExpression(findEndParen());
            consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());
            Statement* body = parseStatement();
            if(peek().type == IDENTIFIER && peek().stringValue.value() == "else") {
                consume(IDENTIFIER, "");
                Statement* elseBody = parseStatement();
                return new IfElse(condition, body, elseBody);
            }
            return new If(condition, body);
        }
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

    return nullptr;
}

Expression* Parser::parseExpression(const int until)
{
    return parseAddSub(until);
}

Expression* Parser::parseCallExpression(const int until) {
    if(peek().type != IDENTIFIER || counter + 1 >= tokens.size() || peek(1).type != LPAREN) return parseParen(until);
    Identifier id{consumeString().value()};
    
    consume(LPAREN, "");
    std::vector<Expression*> args = parseArgs(findEndParen());
    consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());

    return new CallExpression(id, args);
}

Expression* Parser::parseParen(const int until) {
    if (peek().type != LPAREN) return parseSingle();

    consume(LPAREN, "");
    Expression* expr = parseAddSub(findEndParen());
    consume(RPAREN, std::to_string(peek().line) + ": expected ')' but found: " + peek().toString());

    return expr;
}

Expression* Parser::parseCompares(const int until)
{
    const int nextEq = findNextOutsideParen(EQUALS, until);
    const int nextNEq = findNextOutsideParen(NEQUALS, until);
    const int nextLess = findNextOutsideParen(LESS, until);
    const int nextLessEq = findNextOutsideParen(LEQUALS, until);
    const int nextGreater = findNextOutsideParen(GREATER, until);
    const int nextGreaterEq = findNextOutsideParen(GEQUALS, until);

    if (nextEq == -1 && nextNEq == -1 && nextLess == -1 && nextLessEq == -1 && nextGreater == -1 && nextGreaterEq == -1) {
        return parseCallExpression(until);
    }

    std::vector<int> nexts;
    if (nextEq != -1) nexts.push_back(nextEq);
    if (nextNEq != -1) nexts.push_back(nextNEq);
    if (nextLess != -1) nexts.push_back(nextLess);
    if (nextLessEq != -1) nexts.push_back(nextLessEq);
    if (nextGreater != -1) nexts.push_back(nextGreater);
    if (nextGreaterEq != -1) nexts.push_back(nextGreaterEq);
    int next = *std::min_element(nexts.begin(), nexts.end());

    Expression* left = parseCallExpression(next);
    BinaryOperator op;
    if (next == nextEq) op = BinaryOperator::EQUALS;
    else if (next == nextNEq) op = BinaryOperator::NEQUALS;
    else if (next == nextLess) op = BinaryOperator::LESS;
    else if (next == nextLessEq) op = BinaryOperator::LEQUALS;
    else if (next == nextGreater) op = BinaryOperator::GREATER;
    else if (next == nextGreaterEq) op = BinaryOperator::GEQUALS;
    else {
        std::cerr << peek().line << ": expected comparison operator but found: " << peek().toString() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (op == BinaryOperator::EQUALS) consume(EQUALS, std::to_string(peek().line) + ": expected '==' but found: " + peek().toString());
    else if (op == BinaryOperator::NEQUALS) consume(NEQUALS, std::to_string(peek().line) + ": expected '!=' but found: " + peek().toString());
    else if (op == BinaryOperator::LESS) consume(LESS, std::to_string(peek().line) + ": expected '<' but found: " + peek().toString());
    else if (op == BinaryOperator::LEQUALS) consume(LEQUALS, std::to_string(peek().line) + ": expected '<=' but found: " + peek().toString());
    else if (op == BinaryOperator::GREATER) consume(GREATER, std::to_string(peek().line) + ": expected '>' but found: " + peek().toString());
    else consume(GEQUALS, std::to_string(peek().line) + ": expected '>=' but found: " + peek().toString());

    Expression* right = parseCallExpression(until);

    return new BinaryExpression(op, left, right);
}


Expression* Parser::parseSingle() {
    if(peek().type == INT_LIT) {
        return new IntLit(consumeInt().value());
    } else if(peek().type == STRING_LITERAL) {
        return new StringLit(consumeString().value());
    } else if(peek().type == IDENTIFIER) {
        const Identifier id{consumeString().value()};

        return new IdExpression(id);
    } else {
        std::cerr << peek().line << ": expected INT_LIT, STRING_LIT or IDENTIFIER but found: " << peek().toString() << std::endl;
        exit(EXIT_FAILURE);
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

Expression* Parser::parseMulDiv(const int until) {
    int derefDepth = 0;
    while(peek().type == STAR) {
        derefDepth++;
        consume(STAR, "");
    }
    const int nextMul = findNextOutsideParen(STAR, until);
    const int nextDiv = findNextOutsideParen(FSLASH, until);

    if(nextMul == -1 && nextDiv == -1) {
        Expression* expr = parseCompares(until);
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

    Expression* left = parseCompares(next);
    if(op == BinaryOperator::MUL) consume(STAR, std::to_string(peek().line) + ": expected '*' but found: " + peek().toString());
    else consume(FSLASH, std::to_string(peek().line) + ": expected '/' but found: " + peek().toString());
    Expression* right = parseCompares(until);

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
