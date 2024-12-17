#include "Lexer.hpp"
#include "Token.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

Lexer::Lexer() = default;
Lexer::~Lexer() = default;

void Lexer::passLine(const std::string& line, const unsigned int number) {
    if(line.empty()) return;
    //std::cout << line << std::endl;
    //printTokens();
    bool commentFound = false;
    for(int i = 0; i < line.size(); i++) {
        if(commentFound) break;

        char c = line.at(i);

        if(isspace(c)) continue;

        if(isalpha(c)) {
            std::string buf;
            buf.push_back(c);
            i++;
            c = line.at(i);
            while((isalnum(c) || c == '_') && i < line.size()) {
                buf.push_back(c);
                i++;
                c = line.at(i);
            }
            i--;
            
            tokens.push_back(Token{TokType::IDENTIFIER, number, {}, buf, {}, i});
        } else if(c == '"') {
            std::string buf;
            i++;
            c = line.at(i);
            while(c != '"' && i < line.size()) {
                buf.push_back(c);
                i++;
                c = line.at(i);
            }

            tokens.push_back(Token{TokType::STRING_LITERAL, number, {}, buf, {}, i});
        } else if(c >= '0' && c <= '9') {
            std::string buf;
            buf.push_back(c);
            i++;
            c = line.at(i);
            while(c >= '0' && c <= '9' && i < line.size()) {
                buf.push_back(c);
                i++;
                c = line.at(i);
            }
            i--;

            tokens.push_back(Token{TokType::INT_LIT, number, std::stoi(buf), {}, {}, i});
        } else if(c == '\'') {
            if(line.at(i+1) == '\\') {
                switch(line.at(i+2)) {
                    case 'n':
                        tokens.push_back(Token{CHAR_LITERAL, number, {}, {}, '\n', i});
                        break;
                    case '\\':
                        tokens.push_back(Token{CHAR_LITERAL, number, {}, {}, '\\', i});
                        break;
                    case 't':
                        tokens.push_back(Token{CHAR_LITERAL, number, {}, {}, '\t', i});
                        break;
                    case '0':
                        tokens.push_back(Token{CHAR_LITERAL, number, {}, {}, '\0', i});
                }
                if(line.at(i+3) != '\'') {
                    throw std::runtime_error(std::to_string(number) + ": expected \"'\" but found: " + line.at(i+3));
                }

                i += 3;
                continue;
            } else {
                tokens.push_back(Token{CHAR_LITERAL, number, {}, {}, line.at(i+1), i});
                if(line.at(i+2) != '\'') {
                    throw std::runtime_error(std::to_string(number) + ": expected \"'\" but found: " + line.at(i+2));
                }
                i += 2;
                continue;
            }
        } else {
            switch(c) {
                case ';':
                    tokens.push_back(Token{TokType::SEMI, number, {}, {}, {}, i});
                    break;
                case '(':
                    tokens.push_back(Token{TokType::LPAREN, number, {}, {}, {}, i});
                    break;
                case ')':
                    tokens.push_back(Token{TokType::RPAREN, number, {}, {}, {}, i});
                    break;
                case '{':
                    tokens.push_back(Token{TokType::LCURLY, number, {}, {}, {}, i});
                    break;
                case '}':
                    tokens.push_back(Token{TokType::RCURLY, number, {}, {}, {}, i});
                    break;
                case '[':
                    tokens.push_back(Token{TokType::LBRACE, number, {}, {}, {}, i});
                    break;
                case ']':
                    tokens.push_back(Token{TokType::RBRACE, number, {}, {}, {}, i});
                    break;
                case '-':
                    if(line.at(i+1) == '>') {
                        tokens.push_back(Token{RARROW, number, {}, {}, {}, i});
                        i++;
                    } else
                        tokens.push_back(Token{MINUS, number, {}, {}, {}, i});
                    break;
                case '/':
                    if(line.at(i+1) == '/')
                        commentFound = true;
                    else
                        tokens.push_back(Token{FSLASH, number, {}, {}, {}, i});
                    break;
                case '+':
                    tokens.push_back(Token{PLUS, number, {}, {}, {}, i});
                    break;
                case '*':
                    tokens.push_back(Token{STAR, number, {}, {}, {}, i});
                    break;
                case ',':
                    tokens.push_back(Token{COMMA, number, {}, {}, {}, i});
                    break;
                case '=':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{EQUALS, number, {}, {}, {}, i});
                        i++;
                    } else
                        tokens.push_back(Token{ASSIGN, number, {}, {}, {}, i});
                    break;
                case ':':
                    tokens.push_back(Token{COLON, number, {}, {}, {}, i});
                    break;
                case '>':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{GEQUALS, number, {}, {}, {}, i});
                        i++;
                    } else
                        tokens.push_back(Token{GREATER, number, {}, {}, {}, i});
                    break;
                case '<':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{LEQUALS, number, {}, {}, {}, i});
                        i++;
                    } else
                        tokens.push_back(Token{LESS, number, {}, {}, {}, i});
                    break;
                case '!':
                    if (line.at(i+1) == '=') {
                        tokens.push_back(Token{NEQUALS, number, {}, {}, {}, i});
                        i++;
                    } else {
                        std::cerr << number << ": Unknown token: " << line.at(i) << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    break;
                case '|':
                    if(line.at(i+1) == '|') {
                        tokens.push_back(Token{LOGIC_OR, number, {}, {}, {}, i});
                        i++;
                    } else {
                        tokens.push_back(Token{BIT_OR, number, {}, {}, {}, i});
                    }
                    break;
                case '&':
                    if(line.at(i+1 == '&')) {
                        tokens.push_back(Token{LOGIC_AND, number, {}, {}, {}, i});
                        i++;
                    } else {
                        tokens.push_back(Token{BIT_AND, number, {}, {}, {}, i});
                    }
                    break;
                case '%':
                    tokens.push_back(Token{MOD, number, {}, {}, {}, i});
                    break;
                default:
                    std::cerr << number << ":" << i << ": Unknown token type: " << line.at(i) << std::endl;
                    exit(EXIT_FAILURE);
            }
        }
    }
}
        
void Lexer::printTokens() {
    for(Token tok : tokens) {
        std::cout << tok.toString() << std::endl;
    }
}    

std::vector<Token> Lexer::getTokens() {
    return tokens;
}