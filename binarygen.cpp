#include "binarygen.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

void BinaryGenerator::initializeMaps()
{
    opcodeMap = {
        {"LOAD", 0x01},
        {"MOV", 0x02},
        {"ADD", 0x03},
        {"SUB", 0x04},
        {"MUL", 0x05},
        {"DIV", 0x06},
        {"CMP", 0x07},
        {"JMP", 0x08},
        {"JE", 0x09},
        {"JNE", 0x0A},
        {"JLT", 0x0B},
        {"JGT", 0x0C},
        {"JLE", 0x0D},
        {"JGE", 0x0E},
        {"PRINTS", 0x0F},
        {"PRINT", 0x11},
        {"HALT", 0x10},
        {"DATA", 0xFD},
        {"LABEL", 0xFE}};

    for (int i = 0; i <= 9; ++i)
        registerMap["R" + std::to_string(i)] = i;
}

void BinaryGenerator::resolveLabelsAndStrings(const std::vector<std::string> &asmCode)
{
    int labelId = 0;
    int strId = 0;
    for (const auto &line : asmCode)
    {
        std::istringstream iss(line);
        std::string word, label;
        iss >> word >> label;

        if (word == "LABEL")
        {
            labelToId[label] = labelId++;
        }
        else if (word == "DATA")
        {
            stringToId[label] = strId++;
        }
    }
}

void writeBinaryAsBitLines(const std::string &binFilename, const std::string &txtFilename)
{
    std::ifstream in(binFilename, std::ios::binary);
    if (!in)
        throw std::runtime_error("Could not open binary file for reading: " + binFilename);

    std::ofstream out(txtFilename);
    if (!out)
        throw std::runtime_error("Could not open text file for writing: " + txtFilename);

    char buffer[4];
    while (in.read(buffer, 4))
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int bit = 7; bit >= 0; --bit)
            {
                out << ((buffer[i] >> bit) & 1);
            }
            if (i < 3)
                out << " "; // space between bytes
        }
        out << "\n";
    }

    in.close();
    out.close();
}

std::vector<uint8_t> BinaryGenerator::encodeInstruction(const std::string &line)
{
    std::istringstream iss(line);
    std::string word;
    iss >> word;

    std::vector<uint8_t> bytes;

    if (word == "LABEL")
    {
        std::string label;
        iss >> label;
        bytes = {0xFE, labelToId[label], 0x00, 0x00};
    }
    else if (word == "DATA")
    {
        std::string label, str;
        iss >> label;
        std::getline(iss, str);
        str = str.substr(str.find_first_of('"') + 1);
        str.pop_back();
        bytes = {0xFD, stringToId[label], static_cast<uint8_t>(str.size()), 0x00};
        for (char c : str)
            bytes.push_back(static_cast<uint8_t>(c));

        // Pad to 4-byte boundary
        size_t totalSize = bytes.size();
        size_t padding = (4 - (totalSize % 4)) % 4;
        for (size_t i = 0; i < padding; ++i)
            bytes.push_back(0x00);
    }
    else if (opcodeMap.count(word))
    {
        uint8_t opcode = opcodeMap[word];
        std::string arg1, arg2;
        iss >> arg1 >> arg2;

        auto clean = [](std::string s)
        {
            while (!s.empty() && (s.back() == ',' || s.back() == ' ' || s.back() == '\t'))
                s.pop_back();
            while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
                s.erase(s.begin());
            return s;
        };

        arg1 = clean(arg1);
        arg2 = clean(arg2);

        auto getVal = [&](const std::string &token) -> uint8_t
        {
            if (registerMap.count(token))
                return registerMap[token];
            else if (labelToId.count(token))
                return labelToId[token];
            else if (stringToId.count(token))
                return stringToId[token];
            else
                return static_cast<uint8_t>(std::stoi(token));
        };

        bytes = {opcode, 0x00, 0x00, 0x00};
        if (!arg1.empty())
            bytes[1] = getVal(arg1);
        if (!arg2.empty())
            bytes[2] = getVal(arg2);
    }
    return bytes;
}

void BinaryGenerator::generateBinary(const std::vector<std::string> &asmCode, const std::string &outFilename)
{
    initializeMaps();
    resolveLabelsAndStrings(asmCode);

    std::ofstream out(outFilename, std::ios::binary);
    if (!out)
        throw std::runtime_error("Could not open output file: " + outFilename);

    for (const auto &line : asmCode)
    {
        auto bytes = encodeInstruction(line);
        if (!bytes.empty())
            out.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
    }

    out.close();
}
