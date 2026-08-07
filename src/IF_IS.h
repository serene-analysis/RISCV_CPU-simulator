#pragma once

#include "utils.h"
#include "decoder.h"
#include "RegFile.h"

void IF_IS::update_bht(const uint_32 &pc, const bool &taken){
    uint_32 idx = (pc >> 2) & 255;
    uint_8 cnt = bht[idx].curr;
    if(taken){
        cnt = (cnt >= 3) ? 3 : cnt + 1;
    }
    else{
        cnt = (cnt == 0) ? 0 : cnt - 1;
    }
    bht[idx].next = cnt;
    return;
}

void IF_IS::move(IS_ArithRS_Buffer &Abuf, IS_BranchRS_Buffer &Bbuf, IS_LSQ_Buffer &LSbuf, RAT &rat, RegFile &regfile, ROB &rob, uint_32 &end_tag){
    bool bvalid = false;
    uint_32 binst = 0, bPC = 0;
    if(redirected.curr){
        PC.next = redirected_PC.curr;
    }
    else if(RSstall.curr || ROBstall.curr){
        PC.next = PC.curr;
    }
    else{
        uint_32 inst = conv.fetch_instruction(PC.curr);
        PC.next = PC.curr + 4; 
        bvalid = (inst != 0);
        binst = inst;
        bPC = PC.curr;
    }

    Abuf = IS_ArithRS_Buffer(), Bbuf = IS_BranchRS_Buffer(), LSbuf = IS_LSQ_Buffer();
    if(flushed.curr){
        bvalid = false;
        assert(rat.flushed.curr);
        for(int i=0;i<rat.EntrySize_;i++){
            rat.ent[i].next.busy = false;
            rat.ent[i].next.rob_tag = 0;
        }
        return;
    }
    if(RSstall.curr || ROBstall.curr){
        if(rob.qu.nearly_full()){
            ROBstall.next = true;
        }
    }
    if(bvalid){
        Instruction inst = dec.decode(binst);
        if(inst.mem_read || inst.mem_write){
            LSbuf.valid = true;
            LSbuf.mem_read = inst.mem_read, LSbuf.mem_write = inst.mem_write;
            LSbuf.mem_unsigned = inst.mem_unsigned, LSbuf.mem_mask = inst.mem_mask;
            LSbuf.PC = bPC, LSbuf.imm = inst.imm, LSbuf.rd = inst.rd;
            if(inst.mem_read){
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, bPC, false, 0, 0, ROBstall.next, ntag), LSbuf.rob_tag = ntag;
                if(binst == 0x0ff00513){
                    end_tag = ntag;
                }
                rat.query(inst.rs1, nqj), LSbuf.qj = nqj;
                if(!nqj){
                    regfile.read(inst.rs1, LSbuf.vj);
                }
                LSbuf.vk = inst.imm, LSbuf.qk = 0;
                rat.mark(inst.rd, ntag);
            }
            else{
                assert(inst.mem_write);
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, bPC, false, 0, 0, ROBstall.next, ntag), LSbuf.rob_tag = ntag;
                if(binst == 0x0ff00513){
                    end_tag = ntag;
                }
                rat.query(inst.rs1, nqj), rat.query(inst.rs2, nqk), LSbuf.qj = nqj, LSbuf.qk = nqk;
                if(!nqj){
                    regfile.read(inst.rs1, LSbuf.vj);
                }
                if(!nqk){
                    regfile.read(inst.rs2, LSbuf.vk);
                }
            }
        }
        else if(inst.is_branch || inst.is_jump){
            Bbuf.valid = true;
            Bbuf.is_jump = inst.is_jump;
            Bbuf.is_jalr = (inst.is_jump && inst.alu_src_a == 0);
            Bbuf.funct3 = inst.funct3;
            Bbuf.predicted_jump = false;
            if(inst.is_jump && inst.alu_src_a == 1){ 
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, bPC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                if(binst == 0x0ff00513){
                    end_tag = ntag;
                }
                Bbuf.vj = bPC, Bbuf.vk = inst.imm;
                Bbuf.qj = Bbuf.qk = 0;
                rat.mark(inst.rd, ntag);
                Bbuf.nojump_dest = bPC + 4;
                Bbuf.jump_dest = bPC + inst.imm;
                Bbuf.PC = bPC;
                Bbuf.predicted_jump = true;
                PC.next = bPC + inst.imm; 
            }
            else if(inst.is_jump){ 
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, bPC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                if(binst == 0x0ff00513){
                    end_tag = ntag;
                }
                rat.query(inst.rs1, nqj), Bbuf.qj = nqj;
                uint_32 memvj = 0;
                if(!nqj){
                    regfile.read(inst.rs1, memvj);
                    Bbuf.vj = memvj;
                }
                Bbuf.vk = inst.imm, Bbuf.qk = 0;
                rat.mark(inst.rd, ntag);
                Bbuf.nojump_dest = bPC + 4;
                Bbuf.jump_dest = 0;
                Bbuf.PC = bPC;
                if(!nqj){
                    Bbuf.predicted_jump = true;
                    PC.next = memvj + inst.imm; 
                }
            }
            else{
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, 0, bPC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                if(binst == 0x0ff00513){
                    end_tag = ntag;
                }
                Bbuf.predicted_jump = (bht[(bPC >> 2) & 255].curr >= 2);
                if(Bbuf.predicted_jump){
                    PC.next = bPC + inst.imm;
                }
                rat.query(inst.rs1, nqj), rat.query(inst.rs2, nqk), Bbuf.qj = nqj, Bbuf.qk = nqk;
                if(!nqj){
                    regfile.read(inst.rs1, Bbuf.vj);
                }
                if(!nqk){
                    regfile.read(inst.rs2, Bbuf.vk);
                }
                Bbuf.nojump_dest = bPC + 4;
                Bbuf.jump_dest = bPC + inst.imm;
                Bbuf.PC = bPC;
            }
        }
        else{
            Abuf.valid = true;
            Abuf.alu_sel = inst.alu_sel;
            Abuf.alu_src_a = inst.alu_src_a, Abuf.alu_src_b = inst.alu_src_b;
            Abuf.imm = inst.imm, Abuf.PC = bPC;
            uint_32 ntag = 0, nqj = 0, nqk = 0;
            rob.allocate(1, inst.rd, bPC, false, 0, 0, ROBstall.next, ntag), Abuf.rob_tag = ntag;
            if(binst == 0x0ff00513){
                end_tag = ntag;
            }
            if(inst.alu_src_a == 0){
                rat.query(inst.rs1, nqj), Abuf.qj = nqj;
                if(!nqj){
                    regfile.read(inst.rs1, Abuf.vj);
                }
            }
            else{
                if(inst.alu_sel == ALU_passb){
                    Abuf.vj = Abuf.qj = 0;
                }
                else{
                    Abuf.vj = bPC, Abuf.qj = 0;
                }
            }
            if(inst.alu_src_b == 0){
                rat.query(inst.rs2, nqk), Abuf.qk = nqk;
                if(!nqk){
                    regfile.read(inst.rs2, Abuf.vk);
                }
            }
            else{
                Abuf.vk = inst.imm, Abuf.qk = 0;
            }
            rat.mark(inst.rd, ntag);
        }
    }
}
void IF_IS::flush(){
    flushed.next = true;
    return;
}
void IF_IS::tick(){
    PC.tick(), redirected_PC.tick();
    for(int i=0;i<256;i++){
        bht[i].tick();
    }
    RSstall.tick(), ROBstall.tick(), flushed.tick(), redirected.tick();
    RSstall.next = ROBstall.next = flushed.next = redirected.next = false;
    return;
}