#pragma once

#include "utils.h"

struct RegFile{
    Unit<uint_32> reg[32] = {};
    void read(const uint_32 &pos, uint_32 &ret){
        ret = reg[pos].curr;
        return;
    }
    void write(const uint_32 &pos, const uint_32 &value){
        if(pos != 0){
            reg[pos].next = value;
        }
        return;
    }
    void tick(){
        for(int i=0;i<32;i++){
            reg[i].tick();
        }
        return;
    }
};