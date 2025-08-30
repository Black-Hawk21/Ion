#ifndef BINARYGEN_H
#define BINARYGEN_H

#include <string>
#include <vector>
#include <unordered_map>

class BinaryGenerator
{
public:
    // Converts assembly code to binary and writes to output file
    void generateBinary(const std::vector<std::string>& asmCode, const std::string& outFilename);

private:
    std::unordered_map<std::string, uint8_t> opcodeMap;
    std::unordered_map<std::string, uint8_t> registerMap;
    std::unordered_map<std::string, uint8_t> labelToId;
    std::unordered_map<std::string, uint8_t> stringToId;

    void initializeMaps();
    void resolveLabelsAndStrings(const std::vector<std::string>& asmCode);
    std::vector<uint8_t> encodeInstruction(const std::string& line);
};

#endif
