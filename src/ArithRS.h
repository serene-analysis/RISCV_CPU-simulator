#pragma once

#include "utils.h"

void ArithRS::move(ArithRS_ALU_Buffer &Abuf, const CDB_Broadcast_Buffer &Cbuf, bool &ISStall){
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
        }
        if(ent[i].curr.busy){
            got++;
            if(ent[i].curr.qj == 0 && ent[i].curr.qk == 0){
                bpos = i;
            }
        }
        else{
            epos = i;
        }
    }
    if(got >= EntrySize_ - 4){
        ISStall = true;
    }
    if(!stall.curr && bpos != EntrySize_){
        uint_32 va = MUX(ent[bpos].curr.vj, ent[bpos].curr.PC, ent[bpos].curr.alu_src_a),
            vb = MUX(ent[bpos].curr.vk, ent[bpos].curr.imm, ent[bpos].curr.alu_src_b);
        Abuf.valid = true;
        Abuf.alu_sel = ent[bpos].curr.alu_sel, Abuf.operand_a = va, Abuf.operand_b = vb;
        Abuf.rob_tag = ent[bpos].curr.rob_tag;
        ent[bpos].next.busy = false;
    }
    if(buf.curr.valid){
        assert(epos != EntrySize_);
        ent[epos].next.busy = true;
        ent[epos].next.alu_sel = buf.curr.alu_sel;
        ent[epos].next.alu_src_a = buf.curr.alu_src_a, ent[epos].next.alu_src_b = buf.curr.alu_src_b;
        ent[epos].next.rob_tag = buf.curr.rob_tag;
        ent[epos].next.vj = buf.curr.vj, ent[epos].next.qj = buf.curr.qj;
        ent[epos].next.vk = buf.curr.vk, ent[epos].next.qk = buf.curr.qk;
        ent[epos].next.PC = buf.curr.PC, ent[epos].next.imm = buf.curr.imm;
    }
    return;
}
void ArithRS::flush(){
    flushed.next = true;
    return;
}
void ArithRS::tick(){
    for(int i=0;i<EntrySize_;i++){
        ent[i].tick();
    }
    buf.tick();
    buf.next.valid = false;
    stall.tick(), flushed.tick();
    stall.next = flushed.next = false;
    return;
}