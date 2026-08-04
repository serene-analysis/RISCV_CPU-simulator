#pragma once

#include "utils.h"

void RAT::query(const uint_8 &pos, uint_32 &ret){
    ret = ent[pos].curr.rob_tag;
    if(pos == 10) fprintf(stderr, "[RATquery] a0 = %u\n", ret); // DEBUG
    return;
}
void RAT::mark(const uint_8 &pos, const uint_32 &value){
    ent[pos].next.busy = true;
    ent[pos].next.rob_tag = value;
    if(pos == 10) fprintf(stderr, "[RATmark] a0 -> %u\n", value); // DEBUG
    return;
}
void RAT::unlock(const uint_8 &pos, const uint_32 &value){
    if(pos == 10) fprintf(stderr, "[RATunlock] a0 tag=%u busy=%d\n", value, ent[pos].curr.busy); // DEBUG
    if(ent[pos].curr.rob_tag == value){
        ent[pos].next.busy = false;
        ent[pos].next.rob_tag = 0;
    }
    return;
}
void RAT::flush(){
    flushed.next = true;
    return;
}
void RAT::tick(){
    for(int i=0;i<EntrySize_;i++){
        ent[i].tick();
    }
    flushed.tick(), flushed.next = false;
    return;
}