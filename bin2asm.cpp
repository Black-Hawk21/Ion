#include "bin2asm.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <bitset>

std::string BinToAsmConverter::decodeInstruction(uint8_t opcode, uint8_t a1, uint8_t a2, uint8_t a3)
{
    static std::unordered_map<uint8_t, std::string> opMap = {
        {0x01, "LOAD"}, {0x02, "MOV"}, {0x03, "ADD"}, {0x04, "SUB"}, {0x05, "MUL"}, {0x06, "DIV"}, {0x07, "CMP"}, {0x08, "JMP"}, {0x09, "JE"}, {0x0A, "JNE"}, {0x0B, "JLT"}, {0x0C, "JGT"}, {0x0D, "JLE"}, {0x0E, "JGE"}, {0x0F, "PRINTS"}, {0x11, "PRINT"}, {0x10, "HALT"}, {0xFD, "DATA"}, {0xFE, "LABEL"}};

    static std::unordered_map<uint8_t, std::string> regMap = {
        {0, "R0"}, {1, "R1"}, {2, "R2"}, {3, "R3"}, {4, "R4"}, {5, "R5"}, {6, "R6"}, {7, "R7"}, {8, "R8"}, {9, "R9"}};

    std::ostringstream result;
    std::string mnemonic = opMap.count(opcode) ? opMap[opcode] : "UNKNOWN";
    result << mnemonic;

    if (opcode == 0x10)
        return result.str(); // HALT
    if (opcode == 0xFD)
        return "DATA str_" + std::to_string(a1); // handled elsewhere
    if (opcode == 0xFE)
        return "LABEL label_" + std::to_string(a1);

    // Register-only instructions
    auto reg = [&](uint8_t r) -> std::string
    {
        return regMap.count(r) ? regMap[r] : std::to_string(r);
    };

    // Decode based on expected operand type
    switch (opcode)
    {
    case 0x01: // LOAD reg, immediate
    case 0x07: // CMP reg, immediate
        result << " " << reg(a1) << ", " << std::to_string(a2);
        break;

    case 0x02: // MOV reg1, reg2
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06: // ADD, SUB, MUL, DIV
        result << " " << reg(a1) << ", " << reg(a2);
        break;

    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E: // JMP, JE, etc
        result << " label_" << std::to_string(a1);
        break;

    case 0x0F: // PRINTS str_id
        result << " str_" << std::to_string(a1);
        break;

    case 0x11: // PRINT reg
        result << " R" << std::to_string(a1);
        break;

    default:
        result << " " << std::to_string(a1) << ", " << std::to_string(a2);
        break;
    }

    return result.str();
}

void BinToAsmConverter::convert(const std::string &bitFile, const std::string &asmOutputFile)
{
    std::ifstream in(bitFile);
    if (!in)
        throw std::runtime_error("Cannot open bit text file: " + bitFile);

    std::ofstream out(asmOutputFile);
    if (!out)
        throw std::runtime_error("Cannot open output asm file: " + asmOutputFile);

    std::vector<uint8_t> bytes;
    std::string line;

    // Read each line and convert to byte
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        std::istringstream iss(line);
        std::string b;

        while (iss >> b)
        {
            if (b.length() != 8)
                continue;
            bytes.push_back(static_cast<uint8_t>(std::bitset<8>(b).to_ulong()));
        }
    }

    in.close();

    size_t i = 0;
    while (i + 3 < bytes.size())
    {
        uint8_t opcode = bytes[i];
        uint8_t a1 = bytes[i + 1];
        uint8_t a2 = bytes[i + 2];
        uint8_t a3 = bytes[i + 3];
        i += 4;

        if (opcode == 0xFD)
        {
            int len = a2;
            std::string str;
            for (int j = 0; j < len && i < bytes.size(); ++j, ++i)
                str += static_cast<char>(bytes[i]);

            int total = 4 + len;
            int pad = (4 - (total % 4)) % 4;
            i += pad;

            out << "DATA str_" << static_cast<int>(a1) << " \"" << str << "\"" << std::endl;
        }
        else
        {
            out << decodeInstruction(opcode, a1, a2, a3) << std::endl;
        }
    }

    out.close();
}
