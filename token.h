#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    // Keywords
    IF, ELSE, WHILE, INT, BOOL, TRUE, FALSE, PRINT,

    // Identifiers and literals
    IDENTIFIER, NUMBER, STRING_LITERAL,

    // Operators
    PLUS, MINUS, STAR, SLASH,
    EQUAL, EQEQ, NEQ, LT, LTE, GT, GTE,

    // Punctuation
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    SEMICOLON,

    // Special
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType type, const std::string& lexeme, int line)
        : type(type), lexeme(lexeme), line(line) {}
};

#endif
