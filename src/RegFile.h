#pragma once

#include "utils.h"

struct RegFile{
    uint_32 reg[32] = {}, reg_next[32] = {};
    uint_32 read(uint_32 pos){ return reg[pos];}
    void write(uint_32 pos, bool enabled, uint_32 val){
        reg_next[pos] = MUX(reg[pos], val, enabled && !ISZERO(pos));
        return;
    }
    void RegFile_tick(){
        for(int i=0;i<32;i++){
            reg[i] = reg_next[i];
        }
        return;
    }
};