#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <optional>
#include <string>

enum TokType {
    INT_LIT,
    IDENTIFIER,
    STRING_LITERAL,
    CHAR_LITERAL,
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
    MOD,
    COMMA,
    ASSIGN,
    COLON,
    EQUALS,
    GREATER,
    LESS,
    LEQUALS,
    GEQUALS,
    NEQUALS,
    BIT_OR,
    BIT_AND,
    LOGIC_OR,
    LOGIC_AND,
};

inline std::string tokTypeToString(TokType type) {
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
        case CHAR_LITERAL:
            out.append("CHAR_LITERAL");
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
        case NEQUALS:
            out.append("NEQUALS");
            break;
        case BIT_OR:
            out.append("BIT_OR");
            break;
        case BIT_AND:
            out.append("BIT_AND");
            break;
        case LOGIC_OR:
            out.append("LOGIC_OR");
            break;
        case LOGIC_AND:
            out.append("LOGIC_AND");
            break;
        default:
            out.append("");
        }
    return out;
}

struct Token {
    TokType type;
    unsigned int line;
    std::optional<int> intValue;
    std::optional<std::string> stringValue;
    std::optional<char> charValue;
    int col;

    [[nodiscard]] std::string toString() const
    {
        std::string out = tokTypeToString(type);

        if(intValue.has_value()) {
            out.append(": " + std::to_string(intValue.value()));
        }
        if(stringValue.has_value()) {
            out.append(": " + stringValue.value());
        }
        if(charValue.has_value()) {
            out.append(": ");
            out.append(std::string(1, charValue.value()));
        }

        return out;
    }
};

#endif