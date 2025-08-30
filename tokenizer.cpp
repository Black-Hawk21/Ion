#include "tokenizer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>

using namespace std;

Tokenizer::Tokenizer(const string &src) : source(src) {}

vector<Token> Tokenizer::tokenize()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}

bool Tokenizer::isAtEnd() const
{
    return current >= source.length();
}

TokenType checkKeyword(const string &text)
{
    if (text == "if")
        return TokenType::IF;
    if (text == "else")
        return TokenType::ELSE;
    if (text == "while")
        return TokenType::WHILE;
    if (text == "int")
        return TokenType::INT;
    if (text == "bool")
        return TokenType::BOOL;
    if (text == "true")
        return TokenType::TRUE;
    if (text == "false")
        return TokenType::FALSE;
    if (text == "print")
        return TokenType::PRINT;
    return TokenType::IDENTIFIER;
}

char Tokenizer::advance()
{
    return source[current++];
}

char Tokenizer::peek() const
{
    if (isAtEnd())
        return '\0';
    return source[current];
}

char Tokenizer::peekNext() const
{
    if (current + 1 >= source.length())
        return '\0';
    return source[current + 1];
}

bool Tokenizer::match(char expected)
{
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;
    current++;
    return true;
}

void Tokenizer::addToken(TokenType type)
{
    addToken(type, source.substr(start, current - start));
}

void Tokenizer::addToken(TokenType type, const string &text)
{
    tokens.emplace_back(type, text, line);
}

void Tokenizer::identifier()
{
    while (isalnum(peek()) || peek() == '_')
        advance();
    string text = source.substr(start, current - start);
    TokenType type = checkKeyword(text);
    addToken(type, text);
}

void Tokenizer::number()
{
    while (isdigit(peek()))
        advance();
    string text = source.substr(start, current - start);
    addToken(TokenType::NUMBER, text);
}

void Tokenizer::scanToken()
{
    char c = advance();

    switch (c)
    {
    case '"':
    {
        std::string value;

        while (!isAtEnd() && peek() != '"')
        {
            value += peek();
            advance();
        }

        if (isAtEnd())
        {
            throw std::runtime_error("Unterminated string literal at line " + std::to_string(line));
        }

        advance(); // consume closing quote
        tokens.emplace_back(TokenType::STRING_LITERAL, value, line);
        break;
    }
    case '(':
        addToken(TokenType::LPAREN);
        break;
    case ')':
        addToken(TokenType::RPAREN);
        break;
    case '{':
        addToken(TokenType::LBRACE);
        break;
    case '}':
        addToken(TokenType::RBRACE);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '+':
        addToken(TokenType::PLUS);
        break;
    case '-':
        addToken(TokenType::MINUS);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '/':
        addToken(TokenType::SLASH);
        break;
    case '=':
        addToken(match('=') ? TokenType::EQEQ : TokenType::EQUAL);
        break;
    case '!':
        if (match('='))
        {
            addToken(TokenType::NEQ, "!=");
        }
        else
        {
            addToken(TokenType::ERROR, "!");
        }
        break;
    case '<':
        addToken(match('=') ? TokenType::LTE : TokenType::LT);
        break;
    case '>':
        addToken(match('=') ? TokenType::GTE : TokenType::GT);
        break;
    case ' ':
    case '\r':
    case '\t':
        // Ignore whitespace
        break;
    case '\n':
        line++;
        break;
    default:
        if (isalpha(c) || c == '_')
        {
            identifier();
        }
        else if (isdigit(c))
        {
            number();
        }
        else
        {
            addToken(TokenType::ERROR, string(1, c));
        }
        break;
    }
}

void printExpr(Expr *expr)
{
    if (expr->type == ExprType::LITERAL)
    {
        auto *lit = static_cast<LiteralExpr *>(expr);
        std::cout << lit->value;
    }
    else if (expr->type == ExprType::VARIABLE)
    {
        auto *var = static_cast<VariableExpr *>(expr);
        std::cout << var->name;
    }
    else if (expr->type == ExprType::BINARY)
    {
        auto *bin = static_cast<BinaryExpr *>(expr);
        std::cout << "(";
        printExpr(bin->left.get());
        std::cout << " " << bin->op << " ";
        printExpr(bin->right.get());
        std::cout << ")";
    }
    else if (expr->type == ExprType::STRING_LITERAL)
    {
        auto *strExpr = static_cast<StringLiteralExpr *>(expr);
        std::cout << "\"" << strExpr->value << "\"";
    }
}

void printAST(const std::vector<std::unique_ptr<Stmt>> &stmts);

void printStmt(Stmt *stmt)
{
    if (stmt->type == StmtType::VAR_DECL)
    {
        auto *var = static_cast<VarDeclStmt *>(stmt);
        std::cout << "VarDecl: " << var->varType << " " << var->varName << " = ";
        printExpr(var->initializer.get());
        std::cout << "\n";
    }
    else if (stmt->type == StmtType::PRINT)
    {
        std::cout << "Print(";
        auto *print = static_cast<PrintStmt *>(stmt);
        printExpr(print->expression.get());
        std::cout << ")\n";
    }
    else if (stmt->type == StmtType::IF)
    {
        auto *ifs = static_cast<IfStmt *>(stmt);
        std::cout << "If(";
        printExpr(ifs->condition.get());
        std::cout << ") {\n";
        printAST(ifs->thenBranch);
        std::cout << "}";

        if (ifs->elseIfStmt)
        {
            std::cout << " else ";
            printStmt(ifs->elseIfStmt.get()); // ✅ Fixed here
        }
        else if (!ifs->elseBranch.empty())
        {
            std::cout << " else {\n";
            printAST(ifs->elseBranch);
            std::cout << "}";
        }

        std::cout << "\n";
    }
    else if (stmt->type == StmtType::WHILE)
    {
        auto *loop = static_cast<WhileStmt *>(stmt);
        std::cout << "While(";
        printExpr(loop->condition.get());
        std::cout << ") {\n";
        printAST(loop->body);
        std::cout << "}\n";
    }
    else if (stmt->type == StmtType::ASSIGN)
    {
        auto *assign = static_cast<AssignStmt *>(stmt);
        std::cout << "Assign: " << assign->varName << " = ";
        printExpr(assign->value.get());
        std::cout << "\n";
    }
}


void printAST(const std::vector<std::unique_ptr<Stmt>> &stmts)
{
    for (const auto &stmt : stmts)
    {
        printStmt(stmt.get());
    }
}
