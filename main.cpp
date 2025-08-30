#include <iostream>
#include <fstream>
#include <sstream>
#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"
#include "binarygen.h"
#include "bin2asm.h"
#include "vm.h"

std::string readFile(const std::string &filename)
{
    std::ifstream inFile(filename);
    if (!inFile)
        throw std::runtime_error("Could not open source file: " + filename);

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

void writeFile(const std::string &filename, const std::vector<std::string> &lines)
{
    std::ofstream outFile(filename);
    if (!outFile)
        throw std::runtime_error("Could not write to file: " + filename);

    for (const auto &line : lines)
        outFile << line << "\n";
}

std::vector<std::string> readAssembly(const std::string &filename)
{
    std::ifstream inFile(filename);
    if (!inFile)
        throw std::runtime_error("Could not open assembly file: " + filename);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line))
    {
        if (!line.empty())
            lines.push_back(line);
    }
    return lines;
}

bool hasSBSuffix(const std::string &filename)
{
    return filename.size() >= 3 && filename.substr(filename.size() - 3) == ".sb";
}

void writeBinaryAsBitLines(const std::string &binFilename, const std::string &txtFilename);

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <source_file.sb>\n";
            return 1;
        }

        std::string inputFile = argv[1];

        // ✅ Enforce .sb extension
        if (!hasSBSuffix(inputFile))
        {
            std::cerr << "Error: Source file must have a .sb extension.\n";
            return 1;
        }

        std::string asmFile = "program.asm";

        std::string code = readFile(inputFile);

        Tokenizer tokenizer(code);
        std::vector<Token> tokens = tokenizer.tokenize();

        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> ast = parser.parse();

        CodeGenerator generator;
        std::vector<std::string> asmCode = generator.generate(ast);

        writeFile(asmFile, asmCode);

        BinaryGenerator binGen;
        binGen.generateBinary(asmCode, "program.bin");

        writeBinaryAsBitLines("program.bin", "program_bits.txt");

        BinToAsmConverter reconvert;
        reconvert.convert("program_bits.txt", "reconstructed.asm");

        std::vector<std::string> loadedAssembly = readAssembly(asmFile);
        VirtualMachine vm;
        vm.loadProgram(loadedAssembly);
        vm.run();

        // std::vector<std::string> reconstructedAsm = readAssembly("reconstructed.asm");
        // VirtualMachine vm2;
        // vm2.loadProgram(reconstructedAsm);
        // vm2.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
