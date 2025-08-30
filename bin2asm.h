#ifndef BIN2ASM_H
#define BIN2ASM_H

#include <string>
#include <vector>

class BinToAsmConverter {
public:
    void convert(const std::string &bitFile, const std::string &asmOutputFile);

private:
    std::string decodeInstruction(uint8_t opcode, uint8_t a1, uint8_t a2, uint8_t a3);
};

#endif
