#pragma once

#include "utils.h"
#include <map>
#include <cassert>

struct DMEM{
    const static int Memsize_ = 8;
    std::map<uint_32, Unit<uint_8>> DMEM;
    Unit<LSQ_DMEM_Buffer> buf;
    Unit<DMEMEntry> mem[Memsize_];
    Unit<uint_32> submitted_id;
    Unit<bool> accepted, flushed;
    void move(DMEM_CDB_Buffer &Cbuf, bool &LSQStall){
        if(flushed.curr){
            for(int i=0;i<Memsize_;i++){
                mem[i].next.valid = false;
            }
            return;
        }
        uint_32 got = 0, epos = Memsize_, bpos = Memsize_;
        if(accepted.curr){
            mem[submitted_id.curr].next.valid = false;
        }
        for(int i=0;i<Memsize_;i++){
            if(mem[i].curr.valid){
                got++;
                bpos = i;
            }
            else{
                epos = i;
            }
        }
        if(got >= Memsize_ - 4){
            LSQStall = true;
        }
        if(buf.curr.valid){
            assert(epos != Memsize_);
            mem[epos].next = (DMEMEntry){true, buf.curr.rob_tag, buf.curr.rd, 0, 3, buf.curr.goal_addr,
                buf.curr.write_data, buf.curr.mem_read, buf.curr.mem_write, buf.curr.mem_unsigned, buf.curr.mem_mask};
        }
        if(bpos != Memsize_){
            if(mem[bpos].curr.mem_read){
                if(mem[bpos].curr.remaining_round != 0){
                    mem[bpos].next.remaining_round = mem[bpos].curr.remaining_round - 1;
                }
                else{
                    Cbuf.valid = true;
                    Cbuf.rob_tag = mem[bpos].curr.rob_tag;
                    Cbuf.is_read = true;
                    DMEM_operation(mem[bpos].curr.write_data, mem[bpos].curr.mem_read, mem[bpos].curr.mem_write,
                        mem[bpos].curr.mem_unsigned, mem[bpos].curr.mem_mask, mem[bpos].curr.write_addr, Cbuf.load_result);
                    Cbuf.rd = mem[bpos].curr.rd;
                    Cbuf.submitted_id = bpos;
                }
            }
            else{
                Cbuf.valid = true;
                Cbuf.rob_tag = mem[bpos].curr.rob_tag;
                Cbuf.is_read = false;
                Cbuf.write_addr = mem[bpos].curr.write_addr, Cbuf.write_data = mem[bpos].curr.write_data;
                Cbuf.submitted_id = bpos;
                Cbuf.mem_unsigned = mem[bpos].curr.mem_unsigned, Cbuf.mem_mask = mem[bpos].curr.mem_mask;
            }
        }
    }
    void flush(){
        flushed.next = true;
    }
    void tick(){
        for(auto &now : DMEM){
            now.second.curr = now.second.next;
        }
        buf.tick();
        buf.next.valid = false;
        for(int i=0;i<Memsize_;i++){
            mem[i].tick();
        }
        submitted_id.tick();
        accepted.tick(), flushed.tick();
        accepted.next = flushed.next = false;
        return;
    }
    void DMEM_operation(uint_32 write_data, bool read, bool write, bool nosign, uint_32 mask, uint_32 addr, uint_32 &ret){
        if(read){
            switch(mask){
                case 0:{
                    uint_8 nv = DMEM[addr].curr;
                    ret = MUX(static_cast<uint_32>(static_cast<int_32>(static_cast<int_8>(nv))),
                        static_cast<uint_32>(nv), nosign);
                    break;
                }
                case 1:{
                    uint_16 nv = DMEM[addr].curr | (DMEM[addr + 1].curr << 8);
                    ret = MUX(static_cast<uint_32>(static_cast<int_32>(static_cast<int_16>(nv))),
                        static_cast<uint_32>(nv), nosign);
                    break;
                }
                case 2:{
                    uint_32 nv = DMEM[addr].curr | (DMEM[addr + 1].curr << 8) | (DMEM[addr + 2].curr << 16) | (DMEM[addr + 3].curr << 24);
                    ret = nv;
                    break;
                }
                default:{
                    fflush(stderr);
                    throw false;
                }
            }
        }
        if(write){
            uint_8 v0 = write_data & 255, v1 = (write_data >> 8) & 255,
                v2 = (write_data >> 16) & 255, v3 = write_data >> 24;
            switch(mask){
                case 0:{
                    DMEM[addr].next = v0;
                    break;
                }
                case 1:{
                    DMEM[addr].next = v0, DMEM[addr + 1].next = v1;
                    break;
                }
                case 2:{
                    DMEM[addr].next = v0, DMEM[addr + 1].next = v1;
                    DMEM[addr + 2].next = v2, DMEM[addr + 3].next = v3;
                    break;
                }
                default:{
                    fflush(stderr);
                    throw false;
                }
            }
        }
        return;
    }
};