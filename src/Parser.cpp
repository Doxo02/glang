#include "Parser.hpp"

#include <fstream>

#include "AST.hpp"
#include "OpCode.hpp"
#include "Token.hpp"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include "Lexer.hpp"

Parser::Parser(std::vector<Token> tokens, std::string path, bool core) {
    this->tokens = std::move(tokens);
    this->core = core;
    this->path = path;
}

std::vector<FunctionDefinition::ParamData> Parser::parseParameters() {
    std::vector<FunctionDefinition::ParamData> args;
    int index = 0;
    while(peek().type != RPAREN) {
        expectIdentifier();
        std::string id = consumeString().value();

        consume(COLON);

        expectIdentifier();
        TypeIdentifierType t = strToTypeId(consumeString().value());
        int ptrDepth = 0;
        while(peek().type == STAR) {
            consume(STAR);
            ptrDepth++;
        }
        args.push_back(FunctionDefinition::ParamData{id, TypeIdentifier{t, ptrDepth}, index++});
        if(peek().type != RPAREN) {
            consume(COMMA);
        }
    }

    consume(RPAREN);

    return args;
}

Program* Parser::parse() {
    auto* program = new Program();

    // import core
    if(core) {
        std::string corePath("stdlib/core.glang");
        std::fstream file(corePath);
        std::string line;
        unsigned int number = 1;
        Lexer lexer;
        while(std::getline(file, line)) {
            lexer.passLine(line, number++);
        }
        file.close();
        Parser parser(lexer.getTokens(), corePath, false);
        Program* import_prog = parser.parse();

        for (FunctionDefinition* def : import_prog->functions) {
            program->externFunctions.insert({def->id.name, def});
            program->addExtern(def->id.name);
        }
        for (VarDeclaration* decl : import_prog->declarations) {
            program->addExtern(decl->id.name, decl->type);
            program->addExtern(decl->id.name);
        }
        for(VarDeclAssign* decl : import_prog->declAssigns) {
            program->addExtern(decl->id.name, decl->type);
            program->addExtern(decl->id.name);
        }
        for(auto ext : import_prog->externs) {
            program->addExtern(ext);
        }
        for(auto ext : import_prog->externVars) {
            program->addExtern(ext.first, ext.second);
        }
    }

    while(counter < tokens.size()) {
        expectIdentifier();

        int lineNum = peek().line;
        int colNum = peek().col;
        std::string str = consumeString().value();
        if(str == "fn") {
            expectIdentifier();
            const Identifier id{consumeString().value()};
            consume(LPAREN);

            const auto args = parseParameters();

            consume(RARROW);

            expectIdentifier();

            const auto typeId = consumeString().value();
            const auto type = TypeIdentifier{strToTypeId(typeId)};

            Statement* body = parseStatement(true);

            auto* def = new FunctionDefinition(id, body, type, args);
            def->lineNum = lineNum;
            def->colNum = colNum;
            def->path = path;

            program->functions.push_back(def);
        }
        else if(str == "let") {
            const Identifier id{consumeString().value()};
            consume(COLON);
            auto type = TypeIdentifier{strToTypeId(consumeString().value())};
            if(peek().type == SEMI) {
                consume(SEMI);
                auto* decl = new VarDeclaration(id, type);
                decl->lineNum = lineNum;
                decl->colNum = colNum;
                decl->path = path;
                program->declarations.push_back(decl);
            } else if(peek().type == ASSIGN) {
                consume(ASSIGN);
                Expression* expr = parseExpression(findNext(SEMI, tokens.size()));
                consume(SEMI);
                auto* decl = new VarDeclAssign(id, type, expr);
                decl->lineNum = lineNum;
                decl->colNum = colNum;
                decl->path = path;
                program->declAssigns.push_back(decl);
            } else {
                consume(LBRACE);
                Expression* expr = parseExpression(findNext(RBRACE, tokens.size()));
                consume(RBRACE);
                consume(SEMI);
                type.ptrDepth = 1;
                auto* decl = new VarDeclaration(id, type, expr);
                decl->lineNum = lineNum;
                decl->colNum = colNum;
                decl->path = path;
                program->declarations.push_back(decl);
            }
        }
        else if(str == "const") {
            const Identifier id{consumeString().value()};
            consume(COLON);
            const auto type = TypeIdentifier{strToTypeId(consumeString().value())};
            consume(ASSIGN);
            Expression* expr = parseExpression(findNext(SEMI, tokens.size()));
            consume(SEMI);
            auto* decl = new VarDeclAssign(id, type, expr, true);
            decl->lineNum = lineNum;
            decl->colNum = colNum;
            decl->path = path;
            program->declAssigns.push_back(decl);
        }
        else if (str == "import") {
            consume(LPAREN);
            if (peek().type != STRING_LITERAL) {
                std::cerr << path << ":" << peek().line << ": expected STRING_LITERAL but found: " << peek().toString() << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string importPath = consumeString().value();
            consume(RPAREN);
            consume(SEMI);

            if (importPath.find(".glang") == std::string::npos) {
                importPath.append(".glang");
            }

            std::fstream file(importPath);
            std::string line;
            unsigned int number = 1;
            Lexer lexer;
            while(std::getline(file, line)) {
                lexer.passLine(line, number++);
            }
            file.close();
            Parser parser(lexer.getTokens(), importPath, false);
            Program* import_prog = parser.parse();

            for (FunctionDefinition* def : import_prog->functions) {
                program->externFunctions.insert({def->id.name, def});
                program->addExtern(def->id.name);
            }
            for (VarDeclaration* decl : import_prog->declarations) {
                program->addExtern(decl->id.name, decl->type);
                program->addExtern(decl->id.name);
            }
            for(VarDeclAssign* decl : import_prog->declAssigns) {
                program->addExtern(decl->id.name, decl->type);
                program->addExtern(decl->id.name);
            }
            for(auto ext : import_prog->externs) {
                program->addExtern(ext);
            }
            for(auto ext : import_prog->externVars) {
                program->addExtern(ext.first, ext.second);
            }
        }
        else {
            std::cerr << path << ":" << peek().line << ": expected 'fn', 'let' or 'import' but found: " << peek().toString() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
  
    return program;
}

Statement* Parser::parseStatement(const bool funcBody) {
    int line = peek().line;
    int col = peek().col;
    if(peek().type == LCURLY) {
        consume(LCURLY);
        
        std::vector<Statement*> statements;
        while(counter != tokens.size() && peek().type != RCURLY) {
            statements.push_back(parseStatement());
        }

        if(funcBody && dynamic_cast<Return*>(statements.back()) == nullptr) {
            statements.push_back(new Return(nullptr));
        }

        consume(RCURLY);

        auto* compound = new Compound(statements);
        compound->lineNum = line;
        compound->colNum = col;
        compound->path = path;

        statements.push_back(new EndCompound());
        return compound;
    }
    if(peek().type == IDENTIFIER) {
        std::string id = peek().stringValue.value();

        if(id == "return") {
            consume(IDENTIFIER);
            Expression* expr = parseExpression(findNext(SEMI, static_cast<int>(tokens.size())));
            consume(SEMI);
            auto* ret = new Return(expr);
            ret->lineNum = line;
            ret->colNum = col;
            ret->path = path;
            return ret;
        }
        if(id == "let") {
            consume(IDENTIFIER);
            const Identifier identifier{consumeString().value()};
            consume(COLON);

            expectIdentifier();

            std::string typeStr = consumeString().value();
            int ptrDepth = 0;
            while(peek().type == STAR) {
                consume(STAR);
                ptrDepth++;
            }
            const auto type = TypeIdentifier{strToTypeId(typeStr), ptrDepth};
            if(peek().type == SEMI) {
                consume(SEMI);

                auto* decl = new VarDeclaration(identifier, type);
                decl->lineNum = line;
                decl->colNum = col;
                decl->path = path;
                return decl;
            }
            consume(ASSIGN);
            Expression* expr = parseExpression(findNext(SEMI, static_cast<int>(tokens.size())));
            consume(SEMI);

            auto* decl = new VarDeclAssign(identifier, type, expr);
            decl->lineNum = line;
            decl->colNum = col;
            decl->path = path;
            return decl;
        }
        if (id == "while") {
            consume(IDENTIFIER);
            consume(LPAREN);
            Expression* condition = parseExpression(findEndParen());
            consume(RPAREN);
            Statement* body = parseStatement();

            auto* whl = new While(condition, body);
            whl->lineNum = line;
            whl->colNum = col;
            whl->path = path;
            return whl;
        }
        if(id == "if") {
            consume(IDENTIFIER);
            consume(LPAREN);
            Expression* condition = parseExpression(findEndParen());
            consume(RPAREN);
            Statement* body = parseStatement();
            if(peek().type == IDENTIFIER && peek().stringValue.value() == "else") {
                consume(IDENTIFIER);
                Statement* elseBody = parseStatement();

                auto* ifElse = new IfElse(condition, body, elseBody);
                ifElse->lineNum = line;
                ifElse->colNum = col;
                ifElse->path = path;
                return ifElse;
            }

            auto* ifStmt = new If(condition, body);
            ifStmt->lineNum = line;
            ifStmt->colNum = col;
            ifStmt->path = path;
            return ifStmt;
        }
        if(peek(1).type == LPAREN) {
            consume(IDENTIFIER);
            consume(LPAREN);
            const std::vector<Expression*> args = parseArgs(findEndParen());
                
            consume(RPAREN);
            consume(SEMI);

            auto* call = new CallStatement(Identifier{id}, args);
            call->lineNum = line;
            call->colNum = col;
            call->path = path;
            return call;
        }
        int nextSemi = findNext(SEMI, tokens.size());
        int nextAssign = findNext(ASSIGN, nextSemi);

        if(nextAssign == -1) {
            std::cerr << path << ":" << peek().line << ":" << peek().col << ": expected 'return', 'let', 'while', 'if' or a function call but found: " << peek().toString() << std::endl;;
            exit(EXIT_FAILURE);
        }

        Expression* lhs = parseExpression(nextAssign);
        consume(ASSIGN);
        Expression* rhs = parseExpression(nextSemi);
        consume(SEMI);

        auto* varAssign = new VarAssignment(lhs, rhs);
        varAssign->lineNum = line;
        varAssign->colNum = col;
        varAssign->path = path;
        return varAssign;
    }

    return nullptr;
}

Expression* Parser::parseExpression(const int until)
{
    return parseBitOrAnd(until);
}

Expression* Parser::parseCallExpression(const int until) {
    if(peek().type != IDENTIFIER || counter + 1 >= tokens.size() || peek(1).type != LPAREN) return parseParen(until);
    int line = peek().line;
    int col = peek().col;
    Identifier id{consumeString().value()};
    
    consume(LPAREN);
    std::vector<Expression*> args = parseArgs(findEndParen());
    consume(RPAREN);

    auto* call = new CallExpression(id, args);
    call->lineNum = line;
    call->colNum = col;
    call->path = path;
    return call;
}

Expression* Parser::parseParen(const int until) {
    if (peek().type != LPAREN) return parseSingle();

    consume(LPAREN);
    Expression* expr = parseAddSub(findEndParen());
    consume(RPAREN);

    return expr;
}

Expression* Parser::parseCompares(const int until)
{
    const int nextEq = findLastOutsideParen(EQUALS, until);
    const int nextNEq = findLastOutsideParen(NEQUALS, until);
    const int nextLess = findLastOutsideParen(LESS, until);
    const int nextLessEq = findLastOutsideParen(LEQUALS, until);
    const int nextGreater = findLastOutsideParen(GREATER, until);
    const int nextGreaterEq = findLastOutsideParen(GEQUALS, until);

    if (nextEq == -1 && nextNEq == -1 && nextLess == -1 && nextLessEq == -1 && nextGreater == -1 && nextGreaterEq == -1) {
        return parseCallExpression(until);
    }
    int line = peek().line;
    int col = peek().col;

    std::vector<int> nexts;
    if (nextEq != -1) nexts.push_back(nextEq);
    if (nextNEq != -1) nexts.push_back(nextNEq);
    if (nextLess != -1) nexts.push_back(nextLess);
    if (nextLessEq != -1) nexts.push_back(nextLessEq);
    if (nextGreater != -1) nexts.push_back(nextGreater);
    if (nextGreaterEq != -1) nexts.push_back(nextGreaterEq);
    int next = *std::max_element(nexts.begin(), nexts.end());

    Expression* left = parseCompares(next);
    BinaryOperator op;
    if (next == nextEq) op = BinaryOperator::EQUALS;
    else if (next == nextNEq) op = BinaryOperator::NEQUALS;
    else if (next == nextLess) op = BinaryOperator::LESS;
    else if (next == nextLessEq) op = BinaryOperator::LEQUALS;
    else if (next == nextGreater) op = BinaryOperator::GREATER;
    else if (next == nextGreaterEq) op = BinaryOperator::GEQUALS;
    else {
        std::cerr << path << ":" << peek().line << ": expected comparison operator but found: " << peek().toString() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (op == BinaryOperator::EQUALS) consume(EQUALS);
    else if (op == BinaryOperator::NEQUALS) consume(NEQUALS);
    else if (op == BinaryOperator::LESS) consume(LESS);
    else if (op == BinaryOperator::LEQUALS) consume(LEQUALS);
    else if (op == BinaryOperator::GREATER) consume(GREATER);
    else consume(GEQUALS);

    Expression* right = parseCallExpression(until);

    auto* expr = new BinaryExpression(op, left, right);
    expr->lineNum = line;
    expr->colNum = col;
    expr->path = path;
    return expr;
}

Expression* Parser::parseSingle() {
    int line = peek().line;
    int col = peek().col;

    Expression* expr = nullptr;
    if(peek().type == INT_LIT) {
        expr = new IntLit(consumeInt().value());
    }
    else if(peek().type == STRING_LITERAL) {
        expr = new StringLit(consumeString().value());
    }
    else if(peek().type == IDENTIFIER) {
        const Identifier id{consumeString().value()};
        if(peek().type == LBRACE) {
            consume(LBRACE);
            expr = parseExpression(findNext(RBRACE, tokens.size()));
            consume(RBRACE);
        }
        expr = new IdExpression(id, expr);
    }
    else if(peek().type == CHAR_LITERAL) {
        expr = new CharLit(consumeChar().value());
    }
    else if(peek().type == MINUS) {
        consume(MINUS);
        expr = new IntLit(-consumeInt().value());
    }

    if(expr == nullptr) {
        std::cerr << path << ":" << peek().line << ": expected INT_LIT, STRING_LITERAL, IDENTIFIER, CHAR_LITERAL or MINUS but found: " << peek().toString() << std::endl;
        exit(EXIT_FAILURE);
    }
    expr->lineNum = line;
    expr->colNum = col;
    expr->path = path;

    return expr;
}

Expression* Parser::parseBitOrAnd(const int until) {
    int nextOr = findLastOutsideParen(BIT_OR, until);
    int NextAnd = findLastOutsideParen(BIT_AND, until);

    if(nextOr == -1 && NextAnd == -1) return parseAddSub(until);

    int line = peek().line;
    int col = peek().col;

    int next;
    BinaryOperator op;
    if((nextOr < NextAnd && NextAnd != -1) || nextOr == -1) {
        next = NextAnd;
        op = BinaryOperator::BIT_AND;
    } else {
        next = nextOr;
        op = BinaryOperator::BIT_OR;
    }

    Expression* left = parseBitOrAnd(next);
    if(op == BinaryOperator::BIT_OR) consume(BIT_OR);
    else consume(BIT_AND);
    Expression* right = parseAddSub(until);

    auto* expr = new BinaryExpression(op, left, right);
    expr->lineNum = line;
    expr->colNum = col;
    expr->path = path;
    return new BinaryExpression(op, left, right);
}

Expression* Parser::parseAddSub(const int until) {
    int nextPlus = findLastOutsideParen(PLUS, until);
    int nextMinus = findLastOutsideParen(MINUS, until);
    if(nextMinus == counter) nextMinus = -1;

    if(nextPlus == -1 && nextMinus == -1) return parseMulDiv(until);

    int line = peek().line;
    int col = peek().col;

    int next;
    BinaryOperator op;
    if((nextPlus < nextMinus && nextMinus != -1) || nextPlus == -1) {
        next = nextMinus;
        op = BinaryOperator::MINUS;
    } else {
        next = nextPlus;
        op = BinaryOperator::PLUS;
    }

    Expression* left = parseAddSub(next);
    if(op == BinaryOperator::PLUS) consume(PLUS);
    else consume(MINUS);
    Expression* right = parseMulDiv(until);

    auto* expr = new BinaryExpression(op, left, right);
    expr->lineNum = line;
    expr->colNum = col;
    expr->path = path;
    return expr;
}

Expression* Parser::parseMulDiv(const int until) {
    int derefDepth = 0;
    while(peek().type == STAR) {
        derefDepth++;
        consume(STAR);
    }
    const int nextMul = findLastOutsideParen(STAR, until);
    const int nextDiv = findLastOutsideParen(FSLASH, until);
    const int nextMod = findLastOutsideParen(MOD, until);

    if(nextMul == -1 && nextDiv == -1 && nextMod == -1) {
        Expression* expr = parseCompares(until);
        expr->derefDepth = derefDepth;
        return expr;
    }

    int line = peek().line;
    int col = peek().col;

    std::vector<int> nexts;
    if(nextMul != -1) nexts.push_back(nextMul);
    if(nextDiv != -1) nexts.push_back(nextDiv);
    if(nextMod != -1) nexts.push_back(nextMod);

    int next = *std::max_element(nexts.begin(), nexts.end());
    Expression* left = parseMulDiv(next);

    BinaryOperator op;
    if(next == nextMul) op = BinaryOperator::MUL;
    else if(next == nextDiv) op = BinaryOperator::DIV;
    else if(next == nextMod) op = BinaryOperator::MOD;

    if(op == BinaryOperator::MUL) consume(STAR);
    else if(op == BinaryOperator::DIV) consume(FSLASH);
    else if(op == BinaryOperator::MOD) consume(MOD);

    Expression* right = parseCompares(until);

    auto* expr = new BinaryExpression(op, left, right);
    expr->derefDepth = derefDepth;
    expr->lineNum = line;
    expr->colNum = col;
    expr->path = path;

    return expr;
}

std::vector<Expression*> Parser::parseArgs(const int until)
{
    std::vector<Expression*> exprs;

    for(int i = counter; i < until && peek().type != RPAREN; i++) {
        const int nextComma = findNextOutsideParen(COMMA, until);
        
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

void Parser::consume(const TokType type) {
    if(peek().type != type) {
        std::cerr << path << ":" << std::to_string(peek().line) << ":" << std::to_string(peek().col);
        std::cerr << ": expected " << tokTypeToString(type) << " but found: " << tokTypeToString(peek().type) << std::endl;
        exit(EXIT_FAILURE);
    }
    counter++;
}

std::optional<int> Parser::consumeInt() {
    const Token tok = tokens.at(counter++);
    return tok.intValue;
}

std::optional<std::string> Parser::consumeString() {
    Token tok = tokens.at(counter++);
    return tok.stringValue;
}

std::optional<char> Parser::consumeChar() {
    Token tok = tokens.at(counter++);
    return tok.charValue;
}

void Parser::expectIdentifier() {
    if(peek().type != IDENTIFIER) {
        std::cerr << path << ":" << peek().line << ": expected IDENTIFIER but found: " << peek().toString() << std::endl;
        exit(EXIT_FAILURE);
    }
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
    int braceOpenings = 0;
    for(int i = counter; i < until; i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == LBRACE) braceOpenings++;
        if(tokens.at(i).type == RPAREN && openings > 0) openings--;
        if(tokens.at(i).type == RBRACE && braceOpenings > 0) braceOpenings--;

        if(tokens.at(i).type == type && openings == 0 && braceOpenings == 0) return i;
    }

    return -1;
}

int Parser::findLastOutsideParen(TokType type, int until) const
{
    int openings = 0;
    int braceOpenings = 0;
    for(int i = until-1; i >= counter; i--) {
        if(tokens.at(i).type == RPAREN) openings++;
        if(tokens.at(i).type == RBRACE) braceOpenings++;
        if(tokens.at(i).type == LPAREN && openings > 0) openings--;
        if(tokens.at(i).type == LBRACE && braceOpenings > 0) braceOpenings--;

        if(tokens.at(i).type == type && openings == 0 && braceOpenings == 0) return i;
    }

    return -1;
}

int Parser::findEndParen() const
{
    int openings = 0;
    int braceOpenings = 0;
    for(int i = counter; i < tokens.size(); i++) {
        if(tokens.at(i).type == LPAREN) openings++;
        if(tokens.at(i).type == LBRACE) braceOpenings++;
        if(tokens.at(i).type == RBRACE && braceOpenings > 0) braceOpenings--;
        if(tokens.at(i).type == RPAREN) {
            if(openings == 0 && braceOpenings == 0) return i;
            if(openings != 0) openings--;
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
    if(str == "u8") {
        return TypeIdentifierType::U8;
    }
    if(str == "u16") {
        return TypeIdentifierType::U16;
    }
    if(str == "u32") {
        return TypeIdentifierType::U32;
    }
    if(str == "u64") {
        return TypeIdentifierType::U64;
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

    throw std::runtime_error("unrecognized type: " + str);
}
