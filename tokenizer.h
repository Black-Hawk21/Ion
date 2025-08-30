#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include <vector>

class Tokenizer {
public:
    Tokenizer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    bool isAtEnd() const;
    void scanToken();
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void identifier();
    void number();
    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& text);
};

#endif
