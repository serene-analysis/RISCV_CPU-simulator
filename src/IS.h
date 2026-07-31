#pragma once

#include "utils.h"
#include "decoder.h"

struct IS{
    Decoder dec;
    Unit<IF_IS_Buffer> buf;
    Unit<bool> stall;
    void move(IS_ArithRS_Buffer &Abuf, IS_BranchRS_Buffer &Bbuf, IS_LSQ_Buffer &Lbuf,
        RAT &rat, RegFile &regfile, ROB &rob, bool &IFStall){
        
    }
};