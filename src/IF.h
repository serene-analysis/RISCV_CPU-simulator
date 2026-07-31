#pragma once

#include "utils.h"
#include "converter.h"

struct IF{
    Converter conv;
    Unit<uint_32> PC;
    Unit<bool> stall;
    void move(IF_IS_Buffer &buf){
        
    }
};