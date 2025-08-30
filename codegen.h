#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>

class CodeGenerator
{
public:
    CodeGenerator();
    std::vector<std::string> generate(const std::vector<std::unique_ptr<Stmt>> &statements);

private:
    std::unordered_map<std::string, std::string> variableToRegister;

    std::unordered_map<std::string, std::string> stringTable;
    int stringCounter = 0;

    // Returns label like "str_0", "str_1", etc.
    std::string getStringLabel(const std::string &str)
    {
        if (stringTable.find(str) == stringTable.end())
        {
            stringTable[str] = "str_" + std::to_string(stringCounter++);
        }
        return stringTable[str];
    }

    int registerCounter;
    int labelCounter;

    std::string newLabel(const std::string &base);
    std::string getRegisterForVariable(const std::string &name);

    void generateStmt(Stmt *stmt, std::vector<std::string> &output);
    void generateExpr(Expr *expr, std::vector<std::string> &output, const std::string &targetReg);
};

#endif
