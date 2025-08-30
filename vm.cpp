#include "vm.h"
#include <iostream>
#include <sstream>

VirtualMachine::VirtualMachine()
{
    for (int &reg : registers)
        reg = 0;
    for (int &mem : memory)
        mem = 0;
    pc = 0;
    running = true;
}

void VirtualMachine::loadProgram(const std::vector<std::string> &program)
{
    instructions.clear();
    stringData.clear();

    for (const std::string &line : program)
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "DATA")
        {
            std::string label;
            iss >> label;

            std::string rest;
            std::getline(iss, rest); // get the remainder of the line
            size_t firstQuote = rest.find('"');
            size_t lastQuote = rest.rfind('"');

            if (firstQuote == std::string::npos || lastQuote == std::string::npos || lastQuote <= firstQuote)
            {
                throw std::runtime_error("Invalid DATA string format: " + line);
            }

            std::string value = rest.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            stringData[label] = value;
        }
        else
        {
            instructions.push_back(line); // only actual instructions go here
        }
    }

    parseLabels(); // must come after filtering lines
}

void VirtualMachine::parseLabels()
{
    for (int i = 0; i < instructions.size(); ++i)
    {
        std::string line = instructions[i];
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        if (word == "LABEL")
        {
            std::string label;
            iss >> label;
            labelMap[label] = i;
        }
    }
}

int VirtualMachine::getRegisterIndex(const std::string &reg)
{
    if (reg.length() == 2 && reg[0] == 'R')
    {
        return reg[1] - '0';
    }
    throw std::runtime_error("Invalid register: " + reg);
}

void VirtualMachine::run()
{
    while (running && pc < instructions.size())
    {
        std::string line = instructions[pc];
        executeInstruction(line);
        ++pc;
    }
}

std::string cleanToken(std::string token)
{
    if (!token.empty() && token.back() == ',')
        token.pop_back();
    return token;
}

void VirtualMachine::executeInstruction(const std::string &line)
{
    std::istringstream iss(line);
    std::string op;
    iss >> op;

    if (op == "LOAD")
    {
        std::string reg;
        int value;
        iss >> reg >> value;
        reg = cleanToken(reg);
        registers[getRegisterIndex(reg)] = value;
    }
    else if (op == "MOV")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = cleanToken(dst);
        src = cleanToken(src);
        registers[getRegisterIndex(dst)] = registers[getRegisterIndex(src)];
    }
    else if (op == "ADD")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = cleanToken(dst);
        src = cleanToken(src);
        registers[getRegisterIndex(dst)] += registers[getRegisterIndex(src)];
    }
    else if (op == "SUB")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = cleanToken(dst);
        src = cleanToken(src);
        registers[getRegisterIndex(dst)] -= registers[getRegisterIndex(src)];
    }
    else if (op == "MUL")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = cleanToken(dst);
        src = cleanToken(src);
        registers[getRegisterIndex(dst)] *= registers[getRegisterIndex(src)];
    }
    else if (op == "DIV")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = cleanToken(dst);
        src = cleanToken(src);
        registers[getRegisterIndex(dst)] /= registers[getRegisterIndex(src)];
    }
    else if (op == "CMP")
    {
        std::string lhs, rhs;
        iss >> lhs >> rhs;
        lhs = cleanToken(lhs);
        rhs = cleanToken(rhs);

        int r1 = registers[getRegisterIndex(lhs)];
        int r2;

        // Check if rhs is a register or a literal
        if (rhs[0] == 'R')
        {
            r2 = registers[getRegisterIndex(rhs)];
        }
        else
        {
            r2 = std::stoi(rhs);
        }

        if (r1 == r2)
            registers[0] = 0;
        else if (r1 < r2)
            registers[0] = -1;
        else
            registers[0] = 1;
    }
    else if (op == "JMP")
    {
        std::string label;
        iss >> label;
        pc = labelMap[label] - 1;
    }
    else if (op == "JE")
    {
        std::string label;
        iss >> label;
        if (registers[0] == 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "JNE")
    {
        std::string label;
        iss >> label;
        if (registers[0] != 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "JLT")
    {
        std::string label;
        iss >> label;
        if (registers[0] < 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "JGT")
    {
        std::string label;
        iss >> label;
        if (registers[0] > 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "JLE")
    {
        std::string label;
        iss >> label;
        if (registers[0] <= 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "JGE")
    {
        std::string label;
        iss >> label;
        if (registers[0] >= 0)
            pc = labelMap[label] - 1;
    }
    else if (op == "PRINT")
    {
        std::string reg;
        iss >> reg;

        if (reg.length() != 2 || reg[0] != 'R' || !isdigit(reg[1]))
            throw std::runtime_error("Invalid register format: " + reg);

        int regIndex = reg[1] - '0';
        if (regIndex < 0 || regIndex >= 8)
            throw std::runtime_error("Register out of bounds: " + reg);

        std::cout << registers[regIndex] << std::endl;
    }
    else if (op == "PRINTS")
    {
        std::string label;
        iss >> label;

        if (stringData.find(label) == stringData.end())
        {
            throw std::runtime_error("Unknown string label: " + label);
        }

        std::cout << stringData[label] << std::endl;
    }
    else if (op == "HALT")
    {
        running = false;
    }
    else if (op == "LABEL")
    {
        // do nothing — label already parsed
    }
    else
    {
        throw std::runtime_error("Unknown instruction: " + op);
    }
}
