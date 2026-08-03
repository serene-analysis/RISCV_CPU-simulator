#pragma once

#include "utils.h"

void BU::move(BU_CDB_Buffer &Cbuf, bool &BranchRSStall){
    if(flushed.curr){
        for(int i=0;i<Memsize_;i++){
            mem[i].next.valid = false;
        }
        return;
    }
    uint_32 got = 0, epos = Memsize_, bpos = Memsize_;
    if(accepted.curr){
        mem[submitted_id.curr].curr.valid = false;
        got--;
    }
    for(int i=0;i<Memsize_;i++){
        if(mem[i].curr.valid){
            bpos = i;
            got++;
        }
        else if(!accepted.curr || i != submitted_id.curr){
            epos = i;
        }
    }
    if(got >= Memsize_ - 4){
        BranchRSStall = true;
    }
    if(buf.curr.valid){
        assert(epos != Memsize_);
        bool condition_met = false;
        if(!buf.curr.is_jump){
            uint_32 vrs1 = buf.curr.vj, vrs2 = buf.curr.vk;
            logout && fprintf(stderr, "vrs1 = %u, vrs2 = %u\n", vrs1, vrs2);
            switch(buf.curr.funct3){
                case 0: condition_met = EQUAL(vrs1, vrs2); break;
                case 1: condition_met = !EQUAL(vrs1, vrs2); break;
                case 4: condition_met = SLT(vrs1, vrs2); break;
                case 5: condition_met = !SLT(vrs1, vrs2); break;
                case 6: condition_met = SLTU(vrs1, vrs2); break;
                case 7: condition_met = !SLTU(vrs1, vrs2); break;
                default:{
                    fflush(stderr);
                    throw false;
                }
            }
        }
        logout && fprintf(stderr, "condition_met = %d\n", condition_met);
        uint_32 cleared = (buf.curr.jump_dest >> 1) << 1; // Jal
        mem[epos].next.valid = true;
        mem[epos].next.rob_tag = buf.curr.rob_tag;
        mem[epos].next.mispredicted = (buf.curr.predicted_jump != (condition_met || buf.curr.is_jump));
        if(buf.curr.is_jump){
            mem[epos].next.actual_dest = (buf.curr.is_jalr ? buf.curr.jump_dest : cleared);
        }
        else{
            mem[epos].next.actual_dest = (condition_met ? buf.curr.jump_dest : buf.curr.nojump_dest);
        }
    }
    if(bpos){
        Cbuf.valid = true;
        Cbuf.rob_tag = mem[bpos].curr.rob_tag;
        Cbuf.actual_dest = mem[bpos].curr.actual_dest;
        Cbuf.mispredicted = mem[bpos].curr.mispredicted;
        Cbuf.submitted_id = bpos;
    }
}
void BU::flush(){
    flushed.next = true;
    return;
}
void BU::tick(){
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