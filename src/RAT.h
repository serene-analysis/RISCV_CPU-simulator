#pragma once

#include "utils.h"

void RAT::query(const uint_8 &pos, uint_32 &ret){
    ret = ent[pos].curr.rob_tag;
    return;
}
void RAT::mark(const uint_8 &pos, const uint_32 &value){
    if(pos){
        ent[pos].next.locked = true;
        ent[pos].next.lock_id = value;
        //ent[pos].next.busy = true;
        //ent[pos].next.rob_tag = value;
    }
    return;
}
void RAT::unlock(const uint_8 &pos, const uint_32 &value){
    if(ent[pos].curr.rob_tag == value){
        //if(ent[pos].next.rob_tag == value || ent[pos].next.rob_tag == 0){
        //    ent[pos].next.busy = false;
        //    ent[pos].next.rob_tag = 0;
        //}
        ent[pos].next.unlocked = true;
        ent[pos].next.unlock_id = value;
    }
    return;
}
void RAT::flush(){
    flushed.next = true;
    return;
}
void RAT::tick(){
    for(int i=0;i<EntrySize_;i++){
        if(!ent[i].next.locked && !ent[i].next.unlocked){
            
        }
        else if(ent[i].next.locked && !ent[i].next.unlocked){
            ent[i].next.busy = true;
            ent[i].next.rob_tag = ent[i].next.lock_id;
        }
        else if(!ent[i].next.locked && ent[i].next.unlocked){
            ent[i].next.busy = false;
            ent[i].next.rob_tag = 0;
        }
        else{
            if(ent[i].next.lock_id == ent[i].next.unlock_id){
                ent[i].next.busy = false;
                ent[i].next.rob_tag = 0;
            }
            else{
                ent[i].next.busy = true;
                ent[i].next.rob_tag = ent[i].next.lock_id;
            }
        }
        ent[i].tick();
        ent[i].next.locked = ent[i].next.unlocked = false;
        ent[i].next.lock_id = ent[i].next.unlock_id = 0;
    }
    flushed.tick(), flushed.next = false;
    return;
}