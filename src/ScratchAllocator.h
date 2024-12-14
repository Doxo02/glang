//
// Created by larsz on 13.12.2024.
//

#ifndef SCRATCHALLOCATOR_H
#define SCRATCHALLOCATOR_H

#include <string>

const std::string REGS[] =          {"rbx", "r10", "r11", "r12", "r13", "r14", "r15"};
const std::string REGS32[] =        {"ebx", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"};
const std::string REGS16[] =        {"bx", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"};
const std::string REGS8[] =         {"bl", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"};
const int TO_PRESERVE[] =   {0, 3, 4, 5, 6};

class ScratchAllocator {
public:
    ScratchAllocator();
    ~ScratchAllocator() = default;

    int allocate();
    void free(int reg);
    static std::string getReg(const int reg) { return REGS[reg]; }
    static std::string getReg32(const int reg) { return REGS32[reg]; }
    static std::string getReg16(const int reg) { return REGS16[reg]; }
    static std::string getReg8(const int reg) { return REGS8[reg]; }
    bool* getWasUsed() { return wasUsed; }
private:
    bool used[std::size(REGS)];

    bool wasUsed[std::size(REGS)];
};

#endif //SCRATCHALLOCATOR_H
