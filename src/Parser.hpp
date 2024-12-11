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

    Statement* parseStatement(bool funcBody = false);
    std::vector<Expression*> parseArgs(int until);
    std::map<std::string, FunctionDefinition::ParamData> parseParameters();

    Expression* parseExpression(int until);
    Expression* parseCallExpression(int until);
    Expression* parseParen(int until);
    Expression* parseAddSub(int until);
    Expression* parseMulDiv(int until);
    Expression* parseSingle(int until);

    void consume(TokType type, std::string errorMsg);
    std::optional<int> consumeInt();
    std::optional<std::string> consumeString();
    Token peek(int i = 0);
    int findNext(TokType type, int until);
    int findNextOutsideParen(TokType type, int until);
    int findEndParen();

    TypeIdentifierType strToTypeId(std::string str);
};

#endif