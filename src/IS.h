#pragma once

#include "utils.h"
#include "decoder.h"
#include "RegFile.h"

/*
struct IF_IS_Buffer{
    bool valid = false;
    uint_32 inst;
    uint_32 PC;
};
struct IS_ArithRS_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    bool alu_src_a, alu_src_b;
    uint_8 alu_sel;
    uint_32 imm, PC;
};
struct IS_BranchRS_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    uint_8 funct3;
    bool is_jump, is_jalr;
    bool predicted_jump;
    uint_32 nojump_dest, jump_dest;
    uint_32 imm, PC;
};
struct IS_LSQ_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    bool mem_read, mem_write, mem_unsigned;
    uint_8 mem_mask;
    uint_32 rd;
    uint_32 PC, imm;
};
*/

void IS::move(IS_ArithRS_Buffer &Abuf, IS_BranchRS_Buffer &Bbuf, IS_LSQ_Buffer &LSbuf,
    RAT &rat, RegFile &regfile, ROB &rob, bool &IFStall, bool &Foretold, uint_32 &Foretold_PC, uint_32 &end_tag){
    Abuf = IS_ArithRS_Buffer(), Bbuf = IS_BranchRS_Buffer(), LSbuf = IS_LSQ_Buffer();
    fprintf(stderr, "IS : flushed = %d, RSstall = %d, ROBstall = %d\n", flushed.curr, RSstall.curr, ROBstall.curr);
    if(flushed.curr){
        buf.next.valid = false;
        assert(rat.flushed.curr);
        for(int i=0;i<rat.EntrySize_;i++){
            rat.ent[i].next.busy = false;
            rat.ent[i].next.rob_tag = 0;
        }
        return;
    }
    if(RSstall.curr || ROBstall.curr){
        IFStall = true;
        if(rob.qu.nearly_full()){
            ROBstall.next = true;
        }
    }
    if(buf.curr.valid){
        Instruction inst = dec.decode(buf.curr.inst);
        fprintf(stderr, "valid, inst = %u = 0x%x\n", buf.curr.inst, buf.curr.inst), inst.out();
        if(inst.mem_read || inst.mem_write){
            fprintf(stderr, "Memory\n");
            LSbuf.valid = true;
            LSbuf.mem_read = inst.mem_read, LSbuf.mem_write = inst.mem_write;
            LSbuf.mem_unsigned = inst.mem_unsigned, LSbuf.mem_mask = inst.mem_mask;
            LSbuf.PC = buf.curr.PC, LSbuf.imm = inst.imm, LSbuf.rd = inst.rd;
            if(inst.mem_read){
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), LSbuf.rob_tag = ntag;
                fprintf(stderr, "rob_tag = %u\n", ntag);
                if(buf.curr.inst == 0x0ff00513){
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
                rob.allocate(1, inst.rd, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), LSbuf.rob_tag = ntag;
                fprintf(stderr, "rob_tag = %u\n", ntag);
                if(buf.curr.inst == 0x0ff00513){
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
            fprintf(stderr, "Branch\n");
            Bbuf.valid = true;
            Bbuf.is_jump = inst.is_jump;
            Bbuf.is_jalr = (inst.is_jump && inst.alu_src_a == 0);
            Bbuf.funct3 = inst.funct3;
            Bbuf.predicted_jump = false;
            if(inst.is_jump && inst.alu_src_a == 1){ // jal
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                fprintf(stderr, "rob_tag = %u\n", ntag);
                if(buf.curr.inst == 0x0ff00513){
                    end_tag = ntag;
                }
                Bbuf.vj = buf.curr.PC, Bbuf.vk = inst.imm;
                Bbuf.qj = Bbuf.qk = 0;
                rat.mark(inst.rd, ntag);
                Bbuf.nojump_dest = buf.curr.PC + 4;
                Bbuf.jump_dest = buf.curr.PC + inst.imm;
                Bbuf.PC = buf.curr.PC;
            }
            else if(inst.is_jump){ // jalr
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, inst.rd, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                fprintf(stderr, "rob_tag = %u\n", ntag);
                if(buf.curr.inst == 0x0ff00513){
                    end_tag = ntag;
                }
                rat.query(inst.rs1, nqj), Bbuf.qj = nqj;
                if(!nqj){
                    regfile.read(inst.rs1, Bbuf.vj);
                }
                Bbuf.vk = inst.imm, Bbuf.qk = 0;
                rat.mark(inst.rd, ntag);
                Bbuf.nojump_dest = buf.curr.PC + 4;
                Bbuf.jump_dest = 0;
                Bbuf.PC = buf.curr.PC;
            }
            else{
                uint_32 ntag = 0, nqj = 0, nqk = 0;
                rob.allocate(1, 0, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), Bbuf.rob_tag = ntag;
                fprintf(stderr, "rob_tag = %u\n", ntag);
                if(buf.curr.inst == 0x0ff00513){
                    end_tag = ntag;
                }
                rat.query(inst.rs1, nqj), rat.query(inst.rs2, nqk), Bbuf.qj = nqj, Bbuf.qk = nqk;
                if(!nqj){
                    regfile.read(inst.rs1, Bbuf.vj);
                }
                if(!nqk){
                    regfile.read(inst.rs2, Bbuf.vk);
                }
                Bbuf.nojump_dest = buf.curr.PC + 4;
                Bbuf.jump_dest = buf.curr.PC + inst.imm;
                Bbuf.PC = buf.curr.PC;
            }
        }
        else{
            fprintf(stderr, "Arithmetic\n");
            Abuf.valid = true;
            Abuf.alu_sel = inst.alu_sel;
            Abuf.alu_src_a = inst.alu_src_a, Abuf.alu_src_b = inst.alu_src_b;
            Abuf.imm = inst.imm, Abuf.PC = buf.curr.PC;
            uint_32 ntag = 0, nqj = 0, nqk = 0;
            rob.allocate(1, inst.rd, buf.curr.PC, false, 0, 0, ROBstall.next, ntag), Abuf.rob_tag = ntag;
            fprintf(stderr, "rob_tag = %u\n", ntag);
            if(buf.curr.inst == 0x0ff00513){
                end_tag = ntag;
                fprintf(stderr, "\n\nend_tag appeared!\n\n\n");
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
                    Abuf.vj = buf.curr.PC, Abuf.qj = 0;
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
void IS::flush(){
    flushed.next = true;
    return;
}
void IS::tick(){
    buf.tick();
    //fprintf(stderr, "IS::buf.tick(), buf.valid = %d\n", buf.curr.valid);
    buf.next.valid = false;
    RSstall.tick(), ROBstall.tick(), flushed.tick();
    RSstall.next = ROBstall.next = flushed.next = false;
    return;
}