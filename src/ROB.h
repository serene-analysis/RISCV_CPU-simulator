#pragma once

#include "utils.h"
#include "DMEM.h"
#include "RegFile.h"

void ROB::allocate(const uint_8 &type, const uint_8 &rd, const uint_32 PC, const bool &predicted_jump,
    const uint_32 &nojump_dest, const uint_32 &jump_dest, bool &ISStall, uint_32 &ret){
    if(qu.nearly_full()){
        ISStall = true;
    }
    qu.push((ROBEntry){true, uint_32(qu.cnt.curr) + 1, type, rd, PC + 4, predicted_jump, false, 0, false, 0, false, 0, false, 0}); 
    ret = qu.cnt.curr + 1;
    return;
}
void ROB::move(IF_IS &If_is, ArithRS &Ars, BranchRS &Brs, LSQ &Lsq, ALU &Alu, BU &Bu, DMEM &Dmem, CDB &Cdb, RegFile &Regfile, RAT &Rat, bool &ended, uint_32 &end_tag){
    if(flushed.curr){
        qu.l.next = qu.r.curr;
        return;
    }
    if(qu.empty()){
        return;
    }
    ROBEntry fir = qu.front();
    if(fir.done){
        
        if(fir.rob_tag == end_tag){
            ended = true;
            printf("%u\n", Regfile.reg[10].curr & 255u);
            return;
        }
        if(fir.type == 1){
            Regfile.write(fir.rd, fir.value);
            Rat.unlock(fir.rd, fir.rob_tag);
            Cdb.Cbuf.next.valid = true;
            Cdb.Cbuf.next.result = fir.value;
            Cdb.Cbuf.next.rob_tag = fir.rob_tag;
            qu.pop(), committed++;
        }
        else if(fir.type == 2){
            Regfile.write(fir.rd, fir.value);
            Rat.unlock(fir.rd, fir.rob_tag);
            Cdb.Cbuf.next.valid = true;
            Cdb.Cbuf.next.result = fir.value;
            Cdb.Cbuf.next.rob_tag = fir.rob_tag;
            if(fir.branch_misjumped){
                
                If_is.flush(), Ars.flush(), Brs.flush(), Lsq.flush(), Alu.flush(), Bu.flush(), Dmem.flush(), Cdb.flush(), Rat.flush(), flush();
                If_is.redirected.next = true;
                If_is.redirected_PC.next = fir.actual_dest;
            }
            else{
                
            }
            If_is.update_bht(fir.value - 4, (fir.actual_dest != fir.value));
            branches++, branch_mispred += fir.branch_misjumped;
            qu.pop(), committed++;
        }
        else if(fir.type == 3){
            if(fir.is_read){
                Regfile.write(fir.rd, fir.value);
                Rat.unlock(fir.rd, fir.rob_tag);
                Cdb.Cbuf.next.valid = true;
                Cdb.Cbuf.next.result = fir.value;
                Cdb.Cbuf.next.rob_tag = fir.rob_tag;
                qu.pop(), committed++;
            }
            else{
                if(fir.remaining_round != 0){
                    qu.v[qu.nxt(qu.l.curr)].next.remaining_round = fir.remaining_round - 1;
                }
                else{
                    uint_32 no_use = 0;
                    Dmem.DMEM_operation(fir.value, false, true, fir.mem_unsigned, fir.mem_mask, fir.write_addr, no_use);
                    Rbuf.next.valid = true;
                    Rbuf.next.rob_tag = fir.rob_tag;
                    qu.pop(), committed++;
                    
                }
            }
        }
        else{
            assert(false);
        }
    }
}
void ROB::flush(){
    flushed.next = true;
    return;
}
void ROB::tick(){
    for(int i=1;i<=Queuesize_;i++){
        qu.v[i].tick();
    }
    qu.l.tick(), qu.r.tick(), qu.cnt.tick();
    Rbuf.tick();
    Rbuf.next.valid = false;
    flushed.tick();
    flushed.next = false;
    return;
    
}