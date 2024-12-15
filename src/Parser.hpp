#ifndef PARSER_HPP
#define PARSER_HPP

#include "AST.hpp"
#include "Token.hpp"

#include <vector>
#include <optional>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, bool core = true);
    ~Parser() = default;

    Program* parse();

private:
    std::vector<Token> tokens;
    int counter = 0;
    bool core;

    Statement* parseStatement(bool funcBody = false);
    std::vector<Expression*> parseArgs(int until);
    std::map<std::string, FunctionDefinition::ParamData> parseParameters();

    Expression* parseExpression(int until);
    Expression* parseCallExpression(int until);
    Expression* parseParen(int until);
    Expression* parseAddSub(int until);
    Expression* parseMulDiv(int until);
    Expression* parseCompares(int until);
    Expression* parseSingle();

    void consume(TokType type, const std::string& errorMsg);
    std::optional<int> consumeInt();
    std::optional<std::string> consumeString();
    Token peek(int i = 0);
    [[nodiscard]] int findNext(TokType type, int until) const;
    [[nodiscard]] int findNextOutsideParen(TokType type, int until) const;
    [[nodiscard]] int findEndParen() const;

    static TypeIdentifierType strToTypeId(const std::string& str);
};

#endif