#pragma once

#include "utils.h"

/*
struct IF_IS_Buffer{
    bool valid = false;
    uint_32 inst;
    uint_32 PC;
};
*/

void IF::move(IF_IS_Buffer &buf){
    buf = IF_IS_Buffer();
    if(redirected.curr){
        PC.next = redirected_PC.curr;
    }
    else if(stall.curr){
        if(foretold.curr){
            PC.next = foretold_PC.curr;
        }
        else{
            PC.next = PC.curr;
        }
    }
    else if(foretold.curr){
        uint_32 inst = conv.fetch_instruction(foretold_PC.curr);
        PC.next = foretold_PC.curr + 4; // Need an adder
        buf.inst = inst;
        buf.PC = foretold_PC.curr;
    }
    else{
        uint_32 inst = conv.fetch_instruction(PC.curr);
        PC.next = PC.curr + 4; // Need an adder
        buf.inst = inst;
        buf.PC = PC.curr;
    }
    return;
}
void IF::flush(){
    redirected.next = true;
    return;
}
void IF::tick(){
    PC.tick(), foretold_PC.tick(), redirected_PC.tick();
    stall.tick(), foretold.tick(), redirected.tick();
    stall.next = foretold.next = redirected.next = false;
    return;
}