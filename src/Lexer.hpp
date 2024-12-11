#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

#include "Token.hpp"

class Lexer {
public:
    Lexer();
    ~Lexer();

    void passLine(const std::string& line, unsigned int number);
    void printTokens();
    std::vector<Token> getTokens();

private:
    std::vector<Token> tokens;
};

#endif