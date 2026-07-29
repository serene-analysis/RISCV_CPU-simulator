#pragma once

#include "utils.h"

struct RegFile{
    uint_32 reg[32] = {};
    uint_32 read(uint_32 pos){ return reg[pos];}
    void write(uint_32 pos, bool enabled, uint_32 val){
        reg[pos] = MUX(reg[pos], val, enabled && !ISZERO(pos));
        return;
    }
};