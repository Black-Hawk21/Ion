#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>

enum class ExprType
{
    LITERAL,
    VARIABLE,
    BINARY,
    STRING_LITERAL
};

enum class StmtType
{
    VAR_DECL,
    PRINT,
    IF,
    WHILE,
    ASSIGN
};

// === Expression Base ===
struct Expr
{
    ExprType type;
    virtual ~Expr() = default;
};

struct LiteralExpr : public Expr
{
    std::string value;
    LiteralExpr(const std::string &val) : value(val)
    {
        type = ExprType::LITERAL;
    }
};

struct VariableExpr : public Expr
{
    std::string name;
    VariableExpr(const std::string &name) : name(name)
    {
        type = ExprType::VARIABLE;
    }
};

struct StringLiteralExpr : Expr
{
    std::string value;

    StringLiteralExpr(const std::string &value) : value(value)
    {
        type = ExprType::STRING_LITERAL;
    }
};

struct BinaryExpr : public Expr
{
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, const std::string &op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right))
    {
        type = ExprType::BINARY;
    }
};

// === Statement Base ===
struct Stmt
{
    StmtType type;
    virtual ~Stmt() = default;
};

struct VarDeclStmt : public Stmt
{
    std::string varType;
    std::string varName;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(const std::string &type, const std::string &name, std::unique_ptr<Expr> init)
        : varType(type), varName(name), initializer(std::move(init))
    {
        this->type = StmtType::VAR_DECL;
    }
};

struct PrintStmt : public Stmt
{
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr))
    {
        this->type = StmtType::PRINT;
    }
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;  // for else or else-if
    std::unique_ptr<IfStmt> elseIfStmt; // nested else-if block

    IfStmt(std::unique_ptr<Expr> condition,
           std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch = {},
           std::unique_ptr<IfStmt> elseIfStmt = nullptr)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)),
          elseIfStmt(std::move(elseIfStmt)) {
        this->type = StmtType::IF;
    }
};

struct WhileStmt : public Stmt
{
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition(std::move(condition)), body(std::move(body))
    {
        this->type = StmtType::WHILE;
    }
};

struct AssignStmt : public Stmt
{
    std::string varName;
    std::unique_ptr<Expr> value;

    AssignStmt(const std::string &name, std::unique_ptr<Expr> value)
        : varName(name), value(std::move(value))
    {
        this->type = StmtType::ASSIGN;
    }
};

#endif
