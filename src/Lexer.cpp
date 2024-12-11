#include "Lexer.hpp"
#include "Token.hpp"
#include <iostream>
#include <ostream>

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
            
            tokens.push_back(Token{TokType::IDENTIFIER, number, {}, buf});
        } else if(c == '"') {
            std::string buf;
            i++;
            c = line.at(i);
            while(c != '"' && i < line.size()) {
                buf.push_back(c);
                i++;
                c = line.at(i);
            }

            tokens.push_back(Token{TokType::STRING_LITERAL, number, {}, buf});
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

            tokens.push_back(Token{TokType::INT_LIT, number, std::stoi(buf), {}});
        } else {
            switch(c) {
                case ';':
                    tokens.push_back(Token{TokType::SEMI, number});
                    break;
                case '(':
                    tokens.push_back(Token{TokType::LPAREN, number});
                    break;
                case ')':
                    tokens.push_back(Token{TokType::RPAREN, number});
                    break;
                case '{':
                    tokens.push_back(Token{TokType::LCURLY, number});
                    break;
                case '}':
                    tokens.push_back(Token{TokType::RCURLY, number});
                    break;
                case '[':
                    tokens.push_back(Token{TokType::LBRACE, number});
                    break;
                case ']':
                    tokens.push_back(Token{TokType::RBRACE, number});
                    break;
                case '-':
                    if(line.at(i+1) == '>') {
                        tokens.push_back(Token{RARROW, number});
                        i++;
                    } else
                        tokens.push_back(Token{MINUS, number});
                    break;
                case '/':
                    if(line.at(i+1) == '/')
                        commentFound = true;
                    else
                        tokens.push_back(Token{FSLASH, number});
                    break;
                case '+':
                    tokens.push_back(Token{PLUS, number});
                    break;
                case '*':
                    tokens.push_back(Token{STAR, number});
                    break;
                case ',':
                    tokens.push_back(Token{COMMA, number});
                    break;
                case '=':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{EQUALS, number});
                        i++;
                    } else
                        tokens.push_back(Token{ASSIGN, number});
                    break;
                case ':':
                    tokens.push_back(Token{COLON, number});
                    break;
                case '>':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{GEQUALS, number});
                        i++;
                    } else
                        tokens.push_back(Token{GREATER, number});
                case '<':
                    if(line.at(i+1) == '=') {
                        tokens.push_back(Token{LEQUALS, number});
                        i++;
                    } else
                        tokens.push_back(Token{LESS, number});
                default:
                    std::cerr << "Unknown token type: " << line.at(i) << std::endl;
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