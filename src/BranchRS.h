#pragma once

#include "utils.h"

void BranchRS::move(BranchRS_BU_Buffer &Bbuf, CDB_Broadcast_Buffer &Cbuf, bool &ISStall){
    if(flushed.curr){
        for(int i=0;i<EntrySize_;i++){
            ent[i].next.busy = false;
        }
        return;
    }
    uint_32 got = 0, epos = EntrySize_, bpos = EntrySize_;
    for(int i=0;i<EntrySize_;i++){
        if(Cbuf.valid && ent[i].curr.busy){
            if(ent[i].curr.qj == Cbuf.rob_tag){
                ent[i].next.qj = 0;
                ent[i].next.vj = Cbuf.result;
            }
            if(ent[i].curr.qk == Cbuf.rob_tag){
                ent[i].next.qk = 0;
                ent[i].next.vk = Cbuf.result;
            }
            if(ent[i].curr.qj == 0 && ent[i].curr.qk == 0 && ent[i].curr.is_jalr && ent[i].curr.jump_dest == 0){
                ent[i].next.jump_dest = ent[i].curr.vj + ent[i].curr.imm; // Need an adder
            }
        }
        if(ent[i].curr.busy){
            got++;
            if(ent[i].curr.qj == 0 && ent[i].curr.qk == 0 && ent[i].curr.jump_dest != 0){
                bpos = i;
            }
        }
        else{
            epos = i;
        }
    }
    if(got >= EntrySize_ - 4){
        fprintf(stderr, "BranchRS:ISStall\n");
        ISStall = true;
    }
    if(!stall.curr && bpos != EntrySize_){
        Bbuf.valid = true;
        Bbuf.rob_tag = buf.curr.rob_tag;
        Bbuf.vj = ent[bpos].curr.vj, Bbuf.vk = ent[bpos].curr.vk;
        Bbuf.is_jump = ent[bpos].curr.is_jump, Bbuf.is_jalr = ent[bpos].curr.is_jalr;
        Bbuf.jump_dest = ent[bpos].curr.jump_dest, Bbuf.nojump_dest = ent[bpos].curr.nojump_dest;
        Bbuf.PC = ent[bpos].curr.PC, Bbuf.imm = ent[bpos].curr.imm;
        Bbuf.funct3 = ent[bpos].curr.funct3;
        Bbuf.predicted_jump = ent[bpos].curr.predicted_jump;
        ent[bpos].next.busy = false;
    }
    if(buf.curr.valid){
        assert(epos != EntrySize_);
        ent[epos].next.busy = true;
        ent[epos].next.rob_tag = buf.curr.rob_tag;
        ent[epos].next.funct3 = buf.curr.funct3;
        ent[epos].next.is_jalr = buf.curr.is_jalr, ent[epos].next.is_jump = buf.curr.is_jump;
        ent[epos].next.jump_dest = buf.curr.jump_dest, ent[epos].next.nojump_dest = buf.curr.nojump_dest;
        ent[epos].next.predicted_jump = buf.curr.predicted_jump;
        ent[epos].next.vj = buf.curr.vj, ent[epos].next.qj = buf.curr.qj;
        ent[epos].next.vk = buf.curr.vk, ent[epos].next.qk = buf.curr.qk;
        ent[epos].next.PC = buf.curr.PC, ent[epos].next.imm = buf.curr.imm;
        ent[epos].next.busy = true;
    }
    return;
}
void BranchRS::flush(){
    flushed.next = true;
    return;
}
void BranchRS::tick(){
    for(int i=0;i<EntrySize_;i++){
        ent[i].tick();
    }
    buf.tick();
    buf.next.valid = false;
    stall.tick(), flushed.tick();
    stall.next = flushed.next = false;
    return;
}