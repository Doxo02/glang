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
};

struct Token {
    TokType type;
    unsigned int line;
    std::optional<int> intValue;
    std::optional<std::string> stringValue;

    std::string toString() {
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
        default:
            out.append("def");
        }

        if(intValue.has_value()) {
            out.append(": " + intValue.value());
        }
        if(stringValue.has_value()) {
            out.append(": " + stringValue.value());
        }

        return out;
    }
};

#endif