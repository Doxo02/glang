#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <optional>
#include <string>

enum TokType {
    INT_LIT,
    IDENTIFIER,
    STRING_LITERAL,
    SEMI,
    LPAREN,
    RPAREN,
    LCURLY,
    RCURLY,
    LBRACE,
    RBRACE,
    RARROW,
    PLUS,
    MINUS,
    STAR,
    FSLASH,
    COMMA,
    ASSIGN,
    COLON,
    EQUALS,
    GREATER,
    LESS,
    LEQUALS,
    GEQUALS
};

struct Token {
    TokType type;
    unsigned int line;
    std::optional<int> intValue;
    std::optional<std::string> stringValue;

    [[nodiscard]] std::string toString() const
    {
        std::string out;

        switch (type)
        {
        case INT_LIT:
            out.append("INT_LIT");
            break;
        case IDENTIFIER:
            out.append("IDENTIFIER");
            break;
        case STRING_LITERAL:
            out.append("STRING_LITERAL");
            break;
        case SEMI:
            out.append("SEMI");
            break;
        case LPAREN:
            out.append("LPAREN");
            break;
        case RPAREN:
            out.append("RPAREN");
            break;
        case LCURLY:
            out.append("LCURLY");
            break;
        case RCURLY:
            out.append("RCURLY");
            break;
        case LBRACE:
            out.append("LBRACE");
            break;
        case RBRACE:
            out.append("RBRACE");
            break;
        case RARROW:
            out.append("RARROW");
            break;
        case PLUS:
            out.append("PLUS");
            break;
        case MINUS:
            out.append("MINUS");
            break;
        case STAR:
            out.append("STAR");
            break;
        case FSLASH:
            out.append("FSLASH");
            break;
        case COMMA:
            out.append("COMMA");
            break;
        case ASSIGN:
            out.append("ASSIGN");
            break;
        case EQUALS:
            out.append("EQUALS");
            break;
        case GREATER:
            out.append("GREATER");
            break;
        case LESS:
            out.append("LESS");
            break;
        case GEQUALS:
            out.append("GEQUALS");
            break;
        case LEQUALS:
            out.append("LEQUALS");
            break;
        case COLON:
            out.append("COLON");
            break;
        default:
            out.append("");
        }

        if(intValue.has_value()) {
            out.append(": " + std::to_string(intValue.value()));
        }
        if(stringValue.has_value()) {
            out.append(": " + stringValue.value());
        }

        return out;
    }
};

#endif