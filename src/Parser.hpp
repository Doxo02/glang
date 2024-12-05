#ifndef PARSER_HPP
#define PARSER_HPP

#include "AST.hpp"
#include "Token.hpp"

#include <vector>
#include <optional>

class Parser {
public:
    Parser(std::vector<Token> tokens);
    ~Parser() {}

    Program* parse();

private:
    std::vector<Token> tokens;
    int counter = 0;

    Statement* parseStatement();
    std::vector<Expression*> parseArgs(int until);

    Expression* parseExpression(int until);
    Expression* parseParen(int until);
    Expression* parseAddSub(int until);
    Expression* parseMulDiv(int until);
    Expression* parseSingle(int until);

    bool consume(TokType type);
    std::optional<int> consumeInt();
    std::optional<std::string> consumeString();
    Token peek();
    int findNext(TokType type, int until);
    int findNextOutsideParen(TokType type, int until);
    int findEndParen();
};

#endif