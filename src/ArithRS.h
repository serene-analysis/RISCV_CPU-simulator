#pragma once

#include "utils.h"

struct ArithRS{
    const static int EntrySize_ = 32;
    Unit<ArithRSEntry> ent[EntrySize_];
    Unit<IS_ArithRS_Buffer> buf;
    Unit<bool> stall;
    void move(ArithRS_ALU_Buffer &ABuf, CDB_Broadcast_Buffer &CBuf, bool &ISStall){
        
    }
};