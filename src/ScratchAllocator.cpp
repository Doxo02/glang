//
// Created by larsz on 13.12.2024.
//

#include "ScratchAllocator.h"

ScratchAllocator::ScratchAllocator()
{
    for(bool& i : used) {
        i = false;
    }
}

int ScratchAllocator::allocate()
{
    for(int i = 0; i < std::size(REGS); i++) {
        if(!used[i]) {
            used[i] = true;
            return i;
        }
    }

    return -1;
}

void ScratchAllocator::free(const int reg)
{
    used[reg] = false;
}
