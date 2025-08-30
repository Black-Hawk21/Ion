#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();
    std::vector<std::unique_ptr<Stmt>> block();

private:
    const std::vector<Token>& tokens;
    int current = 0;

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const std::string& message);

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> assignment();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
};

#endif
