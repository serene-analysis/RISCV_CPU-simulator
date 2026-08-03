#pragma once

#include "utils.h"

/*
struct LoadEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vbase, qbase;
    bool base_ready;
    bool mem_unsigned;
    uint_8 mem_mask;
    uint_32 rd;
    uint_32 imm;
};
struct StoreEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vbase, qbase, vdata, qdata;
    bool base_ready, data_ready;
    bool mem_unsigned;
    uint_8 mem_mask;
    uint_32 imm;
    bool submitted;
};
*/
void LSQ::move(LSQ_DMEM_Buffer &Dbuf, CDB_Broadcast_Buffer &Cbuf, const ROB_CommitStore_Buffer &Rbuf, bool &ISStall){
    if(flushed.curr){
        for(int i=0;i<EntrySize_;i++){
            lq.v[i].next.busy = false;
            sq.v[i].next.busy = false;
        }
        return;
    }
    uint_32 lgot = 0, sgot = 0;
    for(int i=0;i<EntrySize_;i++){
        if(Cbuf.valid && lq.v[i].curr.busy){
            if(lq.v[i].curr.qbase == Cbuf.rob_tag){
                lq.v[i].next.qbase = 0;
                lq.v[i].next.vbase = Cbuf.result;
            }
            if(lq.v[i].curr.qbase == 0){
                lq.v[i].next.base_ready = true;
            }
        }
        if(Cbuf.valid && sq.v[i].curr.busy){
            if(sq.v[i].curr.qbase == Cbuf.rob_tag){
                sq.v[i].next.qbase = 0;
                sq.v[i].next.vbase = Cbuf.result;
            }
            if(sq.v[i].curr.qdata == Cbuf.rob_tag){
                sq.v[i].next.qdata = 0;
                sq.v[i].next.vdata = Cbuf.result;
            }
            if(sq.v[i].curr.qbase == 0){
                sq.v[i].next.base_ready = true;
            }
            if(sq.v[i].curr.qdata == 0){
                sq.v[i].next.data_ready = true;
            }
        }
        if(lq.v[i].curr.busy){
            lgot++;
        }
        if(sq.v[i].curr.busy){
            sgot++;
        }
    }
    if(lq.nearly_full() || sq.nearly_full()){
        ISStall = true;
    }
    if(Rbuf.valid){
        assert(Rbuf.rob_tag == sq.front().rob_tag);
        sq.pop();
        return;
    }
    if(!stall.curr){
        if(lq.empty() && sq.empty()){
            return;
        }
        if(lq.empty() || (!lq.empty() && !sq.empty() && lq.front().rob_tag > sq.front().rob_tag)){
            if(!sq.empty() && !sq.front().submitted){
                sq.v[sq.l.curr].next.submitted = true;
                Dbuf.valid = true;
                Dbuf.rob_tag = sq.front().rob_tag;
                Dbuf.mem_read = false, Dbuf.mem_write = true;
                Dbuf.mem_unsigned = sq.front().mem_unsigned, Dbuf.mem_mask = sq.front().mem_mask;
                Dbuf.goal_addr = sq.front().vbase + sq.front().imm; // Need an adder
                Dbuf.write_data = sq.front().vdata;
            }
        }
        else{
            if(!lq.empty()){
                Dbuf.valid = true;
                Dbuf.rob_tag = lq.front().rob_tag;
                Dbuf.mem_read = true, Dbuf.mem_write = false;
                Dbuf.mem_unsigned = lq.front().mem_unsigned, Dbuf.mem_mask = lq.front().mem_mask;
                Dbuf.goal_addr = lq.front().vbase + lq.front().imm; // Need an adder
                lq.pop();
            }
        }
    }
    if(buf.curr.valid){
        if(buf.curr.mem_read){
            lq.push((LoadEntry){true, buf.curr.rob_tag, buf.curr.vj, buf.curr.qj, false,
                buf.curr.mem_unsigned, buf.curr.mem_mask, buf.curr.rd, buf.curr.imm});
        }
        else{
            sq.push((StoreEntry){true, buf.curr.rob_tag, buf.curr.vj, buf.curr.qj, buf.curr.vk,
                buf.curr.qk, false, false, buf.curr.mem_unsigned, buf.curr.mem_mask, buf.curr.imm, false});
        }
    }
}
void LSQ::flush(){
    flushed.next = true;
    return;
}
void LSQ::tick(){
    buf.tick();
    buf.next.valid = false;
    for(int i=0;i<EntrySize_;i++){
        lq.v[i].tick(), sq.v[i].tick();
    }
    lq.l.tick(), lq.r.tick(), lq.cnt.tick();
    sq.l.tick(), sq.r.tick(), sq.cnt.tick();
    stall.tick(), flushed.tick();
    stall.next = flushed.next = false;
    return;
}