#ifndef VM_H
#define VM_H

#include <string>
#include <vector>
#include <unordered_map>

class VirtualMachine {
public:
    VirtualMachine();
    void loadProgram(const std::vector<std::string>& program);
    void run();

private:
    int registers[8];
    int memory[1024];
    int pc;
    bool running;

    std::vector<std::string> instructions;
    std::unordered_map<std::string, int> labelMap;
    std::unordered_map<std::string, std::string> stringData;

    int getRegisterIndex(const std::string& reg);
    void parseLabels();
    void executeInstruction(const std::string& line);
};

#endif
