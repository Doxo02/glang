//
// Created by larsz on 13.12.2024.
//

#include "ScratchAllocator.h"
#include <algorithm>
#include <iterator>

ScratchAllocator::ScratchAllocator()
{
    for(bool& i : used) {
        i = false;
    }
    for(bool& i : wasUsed) {
        i = false;
    }
}

int ScratchAllocator::allocate()
{
    for(int i = 0; i < std::size(REGS); i++) {
        if(!used[i]) {
            used[i] = true;
            if(!wasUsed[i] && std::find(std::begin(TO_PRESERVE), std::end(TO_PRESERVE), i) != std::end(TO_PRESERVE))
                wasUsed[i] = true;
            return i;
        }
    }

    return -1;
}

void ScratchAllocator::free(const int reg)
{
    used[reg] = false;
}
