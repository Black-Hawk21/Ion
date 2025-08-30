#include "parser.h"
#include <stdexcept>
#include <iostream>

using namespace std;

Parser::Parser(const vector<Token> &tokens) : tokens(tokens) {}

vector<unique_ptr<Stmt>> Parser::parse()
{
    vector<unique_ptr<Stmt>> statements;
    while (!isAtEnd())
    {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration()
{
    if (match({TokenType::INT, TokenType::BOOL}))
    {
        return varDeclaration();
    }
    if (match({TokenType::PRINT}))
    {
        return printStatement();
    }
    if (match({TokenType::IF}))
    {
        return ifStatement();
    }
    if (match({TokenType::WHILE}))
    {
        return whileStatement();
    }
    // fallback: general statement (e.g., assignment)
    return statement();
}

std::unique_ptr<Stmt> Parser::statement()
{
    return assignment();
}

std::unique_ptr<Stmt> Parser::assignment()
{
    if (match({TokenType::IDENTIFIER}))
    {
        std::string name = previous().lexeme;

        if (match({TokenType::EQUAL}))
        {
            std::unique_ptr<Expr> value = expression();
            consume(TokenType::SEMICOLON, "Expected ';' after assignment.");
            return std::make_unique<AssignStmt>(name, std::move(value));
        }
        else
        {
            throw std::runtime_error("Expected '=' in assignment.");
        }
    }

    throw std::runtime_error("Expected assignment statement.");
}

unique_ptr<Stmt> Parser::varDeclaration()
{
    string varType = previous().lexeme;

    const Token &name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    consume(TokenType::EQUAL, "Expected '=' after variable name.");

    unique_ptr<Expr> init = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

    return make_unique<VarDeclStmt>(varType, name.lexeme, move(init));
}

unique_ptr<Stmt> Parser::printStatement()
{
    consume(TokenType::LPAREN, "Expected '(' after print.");
    unique_ptr<Expr> expr = expression();
    consume(TokenType::RPAREN, "Expected ')' after expression.");
    consume(TokenType::SEMICOLON, "Expected ';' after print.");
    return make_unique<PrintStmt>(move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::block()
{
    std::vector<std::unique_ptr<Stmt>> stmts;

    while (!check(TokenType::RBRACE) && !isAtEnd())
    {
        stmts.push_back(declaration());
    }

    consume(TokenType::RBRACE, "Expected '}' after block.");
    return stmts;
}

std::unique_ptr<Stmt> Parser::ifStatement()
{
    consume(TokenType::LPAREN, "Expected '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");
    consume(TokenType::LBRACE, "Expected '{' after if condition.");
    auto thenBranch = block();

    std::vector<std::unique_ptr<Stmt>> elseBranch;
    std::unique_ptr<IfStmt> elseIfStmt = nullptr;

    if (match({TokenType::ELSE}))
    {
        if (match({TokenType::IF}))
        {
            // else if
            std::unique_ptr<Stmt> elseIfRaw = ifStatement();
            elseIfStmt = std::unique_ptr<IfStmt>(static_cast<IfStmt *>(elseIfRaw.release()));
        }
        else
        {
            // else
            consume(TokenType::LBRACE, "Expected '{' after 'else'.");
            elseBranch = block();
        }
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch),
        std::move(elseIfStmt));
}

std::unique_ptr<Stmt> Parser::whileStatement()
{
    consume(TokenType::LPAREN, "Expected '(' after 'while'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");
    consume(TokenType::LBRACE, "Expected '{' to start while block.");

    std::vector<std::unique_ptr<Stmt>> body = block();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Expr> Parser::term()
{
    std::unique_ptr<Expr> expr = factor();

    while (match({TokenType::PLUS, TokenType::MINUS}))
    {
        std::string op = previous().lexeme;
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor()
{
    std::unique_ptr<Expr> expr = primary();

    while (match({TokenType::STAR, TokenType::SLASH}))
    {
        std::string op = previous().lexeme;
        std::unique_ptr<Expr> right = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality()
{
    std::unique_ptr<Expr> expr = comparison();

    while (match({TokenType::EQEQ, TokenType::NEQ}))
    {
        std::string op = previous().lexeme;
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison()
{
    std::unique_ptr<Expr> expr = term();

    while (match({TokenType::LT, TokenType::LTE, TokenType::GT, TokenType::GTE}))
    {
        std::string op = previous().lexeme;
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::expression()
{
    return equality();
}

unique_ptr<Expr> Parser::primary()
{
    if (match({TokenType::NUMBER, TokenType::TRUE, TokenType::FALSE}))
    {
        return make_unique<LiteralExpr>(previous().lexeme);
    }

    if (match({TokenType::IDENTIFIER}))
    {
        return make_unique<VariableExpr>(previous().lexeme);
    }

    if (match({TokenType::STRING_LITERAL}))
    {
        return std::make_unique<StringLiteralExpr>(previous().lexeme);
    }

    throw runtime_error("Expected expression.");
}

// ===== Helper Functions =====

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

const Token &Parser::peek() const
{
    return tokens[current];
}

const Token &Parser::previous() const
{
    return tokens[current - 1];
}

const Token &Parser::advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return false;
    return peek().type == type;
}

bool Parser::match(initializer_list<TokenType> types)
{
    for (TokenType type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

const Token &Parser::consume(TokenType type, const string &message)
{
    if (check(type))
        return advance();
    throw runtime_error("Parse error: " + message + " at line " + to_string(peek().line));
}
