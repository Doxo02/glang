#ifndef PARSER_HPP
#define PARSER_HPP

#include "AST.hpp"

#include <vector>
#include <optional>

class Parser {
public:
    Parser(std::vector<Token> tokens);
    ~Parser() {}

    Program parse();

private:
    std::vector<Token> tokens;

    Statement* parseStatement();
    Expression* parseExpression();
    std::vector<Expression*> parseArgs();

    bool consume(TokType type);
    std::optional<int> consumeInt();
    std::optional<std::string> consumeString();
    Token peek();
};

#endif