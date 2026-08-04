#pragma once

#include "utils.h"

void RAT::query(const uint_8 &pos, uint_32 &ret){
    ret = ent[pos].curr.rob_tag;
    return;
}
void RAT::mark(const uint_8 &pos, const uint_32 &value){
    if(pos){
        ent[pos].next.busy = true;
        ent[pos].next.rob_tag = value;
    }
    return;
}
void RAT::unlock(const uint_8 &pos, const uint_32 &value){
    if(ent[pos].curr.rob_tag == value){
        if(ent[pos].next.rob_tag == value || ent[pos].next.rob_tag == 0){
            ent[pos].next.busy = false;
            ent[pos].next.rob_tag = 0;
        }
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