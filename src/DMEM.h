#pragma once

#include "utils.h"
#include <map>
#include <cassert>

struct DMEM{
    std::map<uint_32, uint_8> DMEM;
    uint_8 DMEM_operation(Instruction inst, uint_32 addr){
        uint_32 write_data = inst.rs2;
        bool read = inst.mem_read, write = inst.mem_write,
            nosign = inst.mem_unsigned;
        uint_32 mask = inst.mem_mask;
        uint_32 ret = 0;
        switch(read){
            case true:{
                switch(mask){
                    case 0:{
                        uint_8 nv = DMEM[addr];
                        ret = MUX(static_cast<uint_32>(static_cast<int_32>(static_cast<int_8>(nv))),
                            static_cast<uint_32>(nv), nosign);
                        break;
                    }
                    case 1:{
                        uint_16 nv = DMEM[addr] | (DMEM[addr + 1] << 8);
                        ret = MUX(static_cast<uint_32>(static_cast<int_32>(static_cast<int_16>(nv))),
                            static_cast<uint_32>(nv), nosign);
                        break;
                    }
                    case 2:{
                        uint_32 nv = DMEM[addr] | (DMEM[addr + 1] << 8) | (DMEM[addr + 2] << 16) | (DMEM[addr + 3] << 24);
                        ret = nv;
                        break;
                    }
                    default: throw false;
                }
                break;
            }
            case false:{
                assert(write);
                uint_8 v0 = write_data & 255, v1 = (write_data >> 8) & 255,
                    v2 = (write_data >> 16) & 255, v3 = write_data >> 24;
                switch(mask){
                    case 0:{
                        DMEM[addr] = v0;
                        break;
                    }
                    case 1:{
                        DMEM[addr] = v0, DMEM[addr + 1] = v1;
                        break;
                    }
                    case 2:{
                        DMEM[addr] = v0, DMEM[addr + 1] = v1, DMEM[addr + 2] = v2, DMEM[addr + 3] = v3;
                        break;
                    }
                    default: throw false;
                }
                break;
            }
        }
        return ret;
    }
};