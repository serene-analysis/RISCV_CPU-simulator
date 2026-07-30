#pragma once

#include "utils.h"
/*enum ALUType{ ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu, ALU_addi, ALU_andi, ALU_ori, ALU_xori, ALU_slli, ALU_srli, ALU_srai,
    ALU_slti, ALU_sltiu, ALU_passb};*/

struct ALU{
    uint_32 ALU_operation(uint_32 rs1, uint_32 rs2, bool src_a, bool src_b, uint_8 type, uint_32 imm, uint_32 PC){
        uint_32 va = MUX(rs1, PC, src_a), vb = MUX(rs2, imm, src_b);
        logout && fprintf(stderr, "ALU_operation: va = %u, vb = %u, type = %u\n", va, vb, uint_32(type));
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
        logout && fprintf(stderr, "ret = %u\n", ret);
        return ret;
    }
};