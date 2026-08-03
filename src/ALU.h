#pragma once

#include "utils.h"
/*enum ALUType{ ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu, ALU_addi, ALU_andi, ALU_ori, ALU_xori, ALU_slli, ALU_srli, ALU_srai,
    ALU_slti, ALU_sltiu, ALU_passb};*/

void ALU::move(ALU_CDB_Buffer &Cbuf, bool &ArithRSStall){
    if(flushed.curr){
        for(int i=0;i<Memsize_;i++){
            mem[i].next.valid = false;
        }
        return;
    }
    uint_32 got = 0, epos = Memsize_, bpos = Memsize_;
    if(accepted.curr){
        mem[submitted_id.curr].curr.valid = false;
        got--;
    }
    for(int i=0;i<Memsize_;i++){
        if(mem[i].curr.valid){
            bpos = i;
            got++;
        }
        else if(!accepted.curr || i != submitted_id.curr){
            epos = i;
        }
    }
    fprintf(stderr, "ALU : got = %d\n", got);
    if(got >= Memsize_ - 4){
        ArithRSStall = true;
    }
    if(buf.curr.valid){
        uint_32 va = buf.curr.operand_a, vb = buf.curr.operand_b;
        uint_8 type = buf.curr.alu_sel;
        fprintf(stderr, "ALU_operation: va = %u, vb = %u, type = %u\n", va, vb, uint_32(type));
        uint_32 ret = MUX(0, ADD(va, vb), EQUAL(type, ALU_add) | EQUAL(type, ALU_addi)) |
            MUX(0, SUB(va, vb), EQUAL(type, ALU_sub)) | 
            MUX(0, AND(va, vb), EQUAL(type, ALU_and) | EQUAL(type, ALU_andi)) |
            MUX(0, OR(va, vb), EQUAL(type, ALU_or) | EQUAL(type, ALU_ori)) |
            MUX(0, XOR(va, vb), EQUAL(type, ALU_xor) | EQUAL(type, ALU_xori)) |
            MUX(0, SLL(va, vb), EQUAL(type, ALU_sll) | EQUAL(type, ALU_slli)) |
            MUX(0, SRL(va, vb), EQUAL(type, ALU_srl) | EQUAL(type, ALU_srli)) |
            MUX(0, SRA(va, vb), EQUAL(type, ALU_sra) | EQUAL(type, ALU_srai)) |
            MUX(0, SLT(va, vb), EQUAL(type, ALU_slt) | EQUAL(type, ALU_slti)) |
            MUX(0, SLTU(va, vb), EQUAL(type, ALU_sltu) | EQUAL(type, ALU_sltiu)) |
            MUX(0, vb, EQUAL(type, ALU_passb));
        assert(epos != Memsize_);
        mem[epos].next.valid = true;
        mem[epos].next.rob_tag = buf.curr.rob_tag;
        mem[epos].next.alu_result = ret;
    }
    if(bpos){
        Cbuf.valid = true;
        Cbuf.rob_tag = mem[bpos].curr.rob_tag;
        Cbuf.alu_result = mem[bpos].curr.alu_result;
        Cbuf.submitted_id = bpos;
    }
    //logout && fprintf(stderr, "ret = %u\n", ret);
    return;
}
void ALU::flush(){
    flushed.next = true;
}
void ALU::tick(){
    buf.tick();
    buf.next.valid = false;
    for(int i=0;i<Memsize_;i++){
        mem[i].tick();
    }
    submitted_id.tick();
    accepted.tick(), flushed.tick();
    accepted.next = flushed.next = false;
    return;
}