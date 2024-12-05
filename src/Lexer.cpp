#include "Lexer.hpp"
#include <iostream>

Lexer::Lexer() {}
Lexer::~Lexer() {}

void Lexer::passLine(std::string line, unsigned int number) {
    if(line == "") return;
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
                    tokens.push_back(Token{TokType::SEMI});
                    break;
                case '(':
                    tokens.push_back(Token{TokType::LPAREN});
                    break;
                case ')':
                    tokens.push_back(Token{TokType::RPAREN});
                    break;
                case '{':
                    tokens.push_back(Token{TokType::LCURLY});
                    break;
                case '}':
                    tokens.push_back(Token{TokType::RCURLY});
                    break;
                case '[':
                    tokens.push_back(Token{TokType::LBRACE});
                    break;
                case ']':
                    tokens.push_back(Token{TokType::RBRACE});
                    break;
                case '-':
                    if(line.at(i+1) == '>')
                        tokens.push_back(Token{RARROW});
                    else
                        tokens.push_back(Token{MINUS});
                    break;
                case '/':
                    if(line.at(i+1) == '/')
                        commentFound = true;
                    else
                        tokens.push_back(Token{DIV});
                    break;
                case '+':
                    tokens.push_back(Token{PLUS});
                    break;
                case '*':
                    tokens.push_back(Token{MUL});
                    break;
                case ',':
                    tokens.push_back(Token{COMMA});
                    break;
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