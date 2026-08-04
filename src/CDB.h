#pragma once

#include "utils.h"

void CDB::move(ROB &rob, bool &UseA, bool &UseB, bool &UseLS, uint_32 &IdA, uint_32 &IdB, uint_32 &IdLS){
    if(flushed.curr){
        Cbuf.next.valid = false;
        return;
    }
    // FIX(Plan A): full round-robin - every cycle ANY valid source is served, in a rotating priority,
    // so a cycle is never wasted on an empty unit's turn and the LSbuf can't monopolize the CDB.
    static int turn = 1;
    turn++;
    if(turn == 4){
        turn = 1;
    }
    auto tryLS = [&]() -> bool {
        if(!LSbuf.curr.valid) return false;
        UseLS = true, IdLS = LSbuf.curr.submitted_id;
        last_id.next = LSbuf.curr.submitted_id, last_type.next = 3;
        uint_32 pos = LSbuf.curr.rob_tag;
        pos &= (rob.Queuesize_ - 1); // come from Queue size
        if(pos == 0){
            pos = rob.Queuesize_;
        }
        if(rob.qu.v[pos].curr.done){
            return false;
        }
        rob.qu.v[pos].next.rob_tag = LSbuf.curr.rob_tag;
        rob.qu.v[pos].next.type = 3;
        rob.qu.v[pos].next.is_read = LSbuf.curr.is_read;
        rob.qu.v[pos].next.write_addr = LSbuf.curr.write_addr;
        rob.qu.v[pos].next.rd = LSbuf.curr.rd;
        rob.qu.v[pos].next.value = (LSbuf.curr.is_read ? LSbuf.curr.load_result : LSbuf.curr.write_data);
        rob.qu.v[pos].next.remaining_round = (LSbuf.curr.is_read ? 0 : 3);
        rob.qu.v[pos].next.mem_unsigned = LSbuf.curr.mem_unsigned;
        rob.qu.v[pos].next.mem_mask = LSbuf.curr.mem_mask;
        rob.qu.v[pos].next.done = true;
        return true;
    };
    auto tryB = [&]() -> bool {
        if(!Bbuf.curr.valid) return false;
        UseB = true, IdB = Bbuf.curr.submitted_id;
        last_id.next = Bbuf.curr.submitted_id, last_type.next = 2;
        uint_32 pos = Bbuf.curr.rob_tag;
        pos &= (rob.Queuesize_ - 1); // come from Queue size
        if(pos == 0){
            pos = rob.Queuesize_;
        }
        if(rob.qu.v[pos].curr.done){
            return false;
        }
        rob.qu.v[pos].next.rob_tag = Bbuf.curr.rob_tag;
        rob.qu.v[pos].next.type = 2; // rd and value for jal and jalr is written before
        rob.qu.v[pos].next.actual_dest = Bbuf.curr.actual_dest;
        rob.qu.v[pos].next.branch_misjumped = Bbuf.curr.mispredicted;
        rob.qu.v[pos].next.done = true;
        return true;
    };
    auto tryA = [&]() -> bool {
        if(!Abuf.curr.valid) return false;
        UseA = true, IdA = Abuf.curr.submitted_id;
        last_id.next = Abuf.curr.submitted_id, last_type.next = 1;
        uint_32 pos = Abuf.curr.rob_tag;
        pos &= (rob.Queuesize_ - 1); // come from Queue size
        if(pos == 0){
            pos = rob.Queuesize_;
        }
        if(rob.qu.v[pos].curr.done){
            return false;
        }
        rob.qu.v[pos].next.rob_tag = Abuf.curr.rob_tag;
        rob.qu.v[pos].next.type = 1;
        rob.qu.v[pos].next.value = Abuf.curr.alu_result;
        rob.qu.v[pos].next.done = true;
        return true;
    };
    if(turn == 1){
        tryA() || tryB() || tryLS();
    }
    else if(turn == 2){
        tryB() || tryLS() || tryA();
    }
    else{
        tryLS() || tryA() || tryB();
    }
    return;
}
void CDB::flush(){
    flushed.curr = true;
    return;
}
void CDB::tick(){
    Abuf.tick(), Bbuf.tick(), LSbuf.tick(), Cbuf.tick();
    Abuf.next.valid = false, Bbuf.next.valid = false, LSbuf.next.valid = false; // Don't clear the broadcast, it should last
    last_id.tick(), last_type.tick(), flushed.tick();
    last_id.next = last_type.next = 0;
    flushed.next = false;
    return;
}