#include "codegen.h"
#include <sstream>
#include <iostream>

CodeGenerator::CodeGenerator() : registerCounter(1), labelCounter(0) {}

std::string CodeGenerator::newLabel(const std::string &base)
{
    return base + "_" + std::to_string(labelCounter++);
}

std::string CodeGenerator::getRegisterForVariable(const std::string &name)
{
    if (variableToRegister.find(name) == variableToRegister.end())
    {
        variableToRegister[name] = "R" + std::to_string(registerCounter++);
    }
    return variableToRegister[name];
}

std::vector<std::string> CodeGenerator::generate(const std::vector<std::unique_ptr<Stmt>> &statements)
{
    std::vector<std::string> output;

    for (const auto &stmt : statements)
    {
        generateStmt(stmt.get(), output);
    }

    for (const auto &entry : stringTable)
    {
        output.push_back("DATA " + entry.second + " \"" + entry.first + "\"");
    }

    output.push_back("HALT");
    return output;
}

void CodeGenerator::generateStmt(Stmt *stmt, std::vector<std::string> &output)
{
    if (stmt->type == StmtType::VAR_DECL)
    {
        auto *decl = static_cast<VarDeclStmt *>(stmt);
        std::string reg = getRegisterForVariable(decl->varName);
        generateExpr(decl->initializer.get(), output, reg);
    }
    else if (stmt->type == StmtType::ASSIGN)
    {
        auto *assign = static_cast<AssignStmt *>(stmt);
        std::string reg = getRegisterForVariable(assign->varName);
        generateExpr(assign->value.get(), output, reg);
    }
    else if (stmt->type == StmtType::IF)
    {
        auto *ifStmt = static_cast<IfStmt *>(stmt);

        std::string endLabel = newLabel("endif");

        // Label generator
        auto nextBlockLabel = newLabel("else");

        // Evaluate the main `if` condition
        generateExpr(ifStmt->condition.get(), output, "R0");
        output.push_back("CMP R0, 0");
        output.push_back("JE " + nextBlockLabel);

        // then block
        for (const auto &s : ifStmt->thenBranch)
        {
            generateStmt(s.get(), output);
        }
        output.push_back("JMP " + endLabel);

        output.push_back("LABEL " + nextBlockLabel);

        // else-if (recursively nested IfStmt)
        if (ifStmt->elseIfStmt)
        {
            generateStmt(ifStmt->elseIfStmt.get(), output);
        }
        // else block
        else if (!ifStmt->elseBranch.empty())
        {
            for (const auto &s : ifStmt->elseBranch)
            {
                generateStmt(s.get(), output);
            }
        }

        output.push_back("LABEL " + endLabel);
    }

    else if (stmt->type == StmtType::WHILE)
    {
        auto *loop = static_cast<WhileStmt *>(stmt);
        std::string startLabel = newLabel("while");
        std::string endLabel = newLabel("endwhile");
        std::string condReg = "R0";

        output.push_back("LABEL " + startLabel);
        generateExpr(loop->condition.get(), output, condReg);
        output.push_back("CMP " + condReg + ", 0");
        output.push_back("JE " + endLabel);

        for (const auto &s : loop->body)
        {
            generateStmt(s.get(), output);
        }

        output.push_back("JMP " + startLabel);
        output.push_back("LABEL " + endLabel);
    }
    if (stmt->type == StmtType::PRINT)
    {
        auto *printStmt = static_cast<PrintStmt *>(stmt);

        if (printStmt->expression->type == ExprType::STRING_LITERAL)
        {
            auto *strExpr = static_cast<StringLiteralExpr *>(printStmt->expression.get());
            std::string label = getStringLabel(strExpr->value);
            output.push_back("PRINTS " + label);
        }
        else
        {
            generateExpr(printStmt->expression.get(), output, "R0");
            output.push_back("PRINT R0");
        }
    }
}

void CodeGenerator::generateExpr(Expr *expr, std::vector<std::string> &output, const std::string &targetReg)
{
    if (expr->type == ExprType::LITERAL)
    {
        auto *lit = static_cast<LiteralExpr *>(expr);
        output.push_back("LOAD " + targetReg + ", " + lit->value);
    }
    else if (expr->type == ExprType::VARIABLE)
    {
        auto *var = static_cast<VariableExpr *>(expr);
        std::string reg = getRegisterForVariable(var->name);
        output.push_back("MOV " + targetReg + ", " + reg);
    }
    else if (expr->type == ExprType::BINARY)
    {
        auto *bin = static_cast<BinaryExpr *>(expr);

        std::string leftReg = "R6";
        std::string rightReg = "R7";
        generateExpr(bin->left.get(), output, leftReg);
        generateExpr(bin->right.get(), output, rightReg);

        if (bin->op == "+")
        {
            output.push_back("MOV " + targetReg + ", " + leftReg);
            output.push_back("ADD " + targetReg + ", " + rightReg);
        }
        else if (bin->op == "-")
        {
            output.push_back("MOV " + targetReg + ", " + leftReg);
            output.push_back("SUB " + targetReg + ", " + rightReg);
        }
        else if (bin->op == "*")
        {
            output.push_back("MOV " + targetReg + ", " + leftReg);
            output.push_back("MUL " + targetReg + ", " + rightReg);
        }
        else if (bin->op == "/")
        {
            output.push_back("MOV " + targetReg + ", " + leftReg);
            output.push_back("DIV " + targetReg + ", " + rightReg);
        }
        else if (bin->op == "==" || bin->op == "!=" ||
                 bin->op == "<" || bin->op == "<=" ||
                 bin->op == ">" || bin->op == ">=")
        {
            output.push_back("CMP " + leftReg + ", " + rightReg);
            std::string setReg = targetReg;

            std::string labelTrue = newLabel("cmp_true");
            std::string labelEnd = newLabel("cmp_end");

            std::string jmpInstr;
            if (bin->op == "==")
                jmpInstr = "JE";
            else if (bin->op == "!=")
                jmpInstr = "JNE";
            else if (bin->op == "<")
                jmpInstr = "JLT";
            else if (bin->op == "<=")
                jmpInstr = "JLE";
            else if (bin->op == ">")
                jmpInstr = "JGT";
            else if (bin->op == ">=")
                jmpInstr = "JGE";

            output.push_back(jmpInstr + " " + labelTrue);
            output.push_back("LOAD " + setReg + ", 0");
            output.push_back("JMP " + labelEnd);
            output.push_back("LABEL " + labelTrue);
            output.push_back("LOAD " + setReg + ", 1");
            output.push_back("LABEL " + labelEnd);
        }
    }
}
