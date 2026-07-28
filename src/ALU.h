#pragma once

#include "utils.h"
/*enum ALUType{ ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu, ALU_addi, ALU_andi, ALU_ori, ALU_xori, ALU_slli, ALU_srli, ALU_srai,
    ALU_slti, ALU_sltiu, ALU_passb};*/

uint_32 ADD(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    bool carry = false;
    for(int i=0;i<32;i++){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        bool got = (lv & carry) | (rv & carry) | (lv & rv);
        ret |= (lv ^ rv ^ carry) << i;
        carry = got;
    }
    return ret;
}

uint_32 SUB(uint_32 now, uint_32 oth){
    for(int i=0;i<32;i++){
        oth ^= (1u << i);
    }
    oth = ADD(oth, 1);
    return ADD(now, oth);
}

uint_32 AND(uint_32 now, uint_32 oth){
    uint_32 ret;
    for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) & ((oth >> i) & 1)) << i;
    }
    return ret;
}

uint_32 OR(uint_32 now, uint_32 oth){
    uint_32 ret;
    for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) | ((oth >> i) & 1)) << i;
    }
    return ret;
}

uint_32 XOR(uint_32 now, uint_32 oth){
    uint_32 ret;
    for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) ^ ((oth >> i) & 1)) << i;
    }
    return ret;
}

uint_32 MUX(uint_32 v0, uint_32 v1, bool type){
    uint_32 lv = !type, rv = type;
    for(int i=1;i<32;i++){
        lv |= ((lv & 1) << i), rv |= ((rv & 1) << i);
    }
    return OR(AND(lv, v0), AND(rv, v1));
}

bool ISZERO(uint_32 now){
    uint_32 ret = 1;
    for(int i=0;i<32;i++){
        bool nv = (now >> i) & 1;
        ret &= !(nv & nv);
    }
    return ret;
}

uint_32 SLL(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=d;i<32;i++){
            int nv = MUX(0, (now >> (i - d)) & 1, equ);
            ret |= (nv << i);
        }
    }
    return ret;
}

uint_32 SRL(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=0;i<32-d;i++){
            int nv = MUX(0, (now >> (i + d)) & 1, equ);
            ret |= (nv << i);
        }
    }
    return ret;
}

uint_32 SRA(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=0;i<32-d;i++){
            int nv = MUX(0, (now >> (i + d)) & 1, equ);
            ret |= (nv << i);
        }
        for(int i=32-d;i<32;i++){
            int nv = MUX(0, (now >> 31) & 1, equ);
            ret |= (nv << i);
        }
    }
    return ret;
}

bool SLTU(uint_32 now, uint_32 oth){ // (now < oth), unsigned
    bool yes = false, no = false;
    for(int i=31;i>=0;i--){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        yes = yes | ((!no) & (lv != rv) & ISZERO(lv));
        no = no | ((!yes) & (lv != rv) & ISZERO(rv));
    }
    return yes;
}

bool SLT(uint_32 now, uint_32 oth){
    bool luv = ((now >> 31) & 1), ruv = ((oth >> 31) & 1);
    bool dif = (luv != ruv);
    bool yes = MUX(false, ISZERO(ruv), dif), no = MUX(false, ISZERO(luv), dif), rev = (!dif) & (!ISZERO(luv));
    for(int i=31;i>=0;i--){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        yes = yes | ((!no) & (lv != rv) & ISZERO(lv));
        no = no | ((!yes) & (lv != rv) & ISZERO(rv));
    }
    return MUX(yes, !yes, rev);// can't put no because when now == oth, no == false
}

bool EQUAL(uint_32 now, uint_32 oth){
    return ISZERO(SUB(now, oth));
}

/*ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu,*/

struct ALU{
    uint_32 ALU_operation(Instruction inst, uint_32 PC){
        bool src_a = inst.alu_src_a, src_b = inst.alu_src_b;
        uint_8 type = inst.alu_sel;
        uint_32 va = MUX(inst.rs1, PC, src_a), vb = MUX(inst.rs2, inst.imm, src_b);
        uint_32 ret = MUX(0, ADD(va, vb), EQUAL(type, ALU_add) | EQUAL(type, ALU_addi)) |
            MUX(0, SUB(va, vb), EQUAL(type, ALU_sub)) | 
            MUX(0, AND(va, vb), EQUAL(type, ALU_and) | EQUAL(type, ALU_andi)) |
            MUX(0, OR(va, vb), EQUAL(type, ALU_or) | EQUAL(type, ALU_ori)) |
            MUX(0, XOR(va, vb), EQUAL(type, ALU_xor) | EQUAL(type, ALU_xori)) |
            MUX(0, SLL(va, vb), EQUAL(type, ALU_sll) | EQUAL(type, ALU_slli)) |
            MUX(0, SRL(va, vb), EQUAL(type, ALU_srl) | EQUAL(type, ALU_srli)) |
            MUX(0, SRA(va, vb), EQUAL(type, ALU_sra) | EQUAL(type, ALU_srai)) |
            MUX(0, SLT(va, vb), EQUAL(type, ALU_slt) | EQUAL(type, ALU_slti)) |
            MUX(0, SLTU(va, vb), EQUAL(type, ALU_sltu) | EQUAL(type, ALU_sltiu));
        return ret;
    }
};