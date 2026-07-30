#pragma once

#include "utils.h"

struct Decoder{
    Instruction decode(uint_32 inst){
        uint_32 opcode = inst & 127, rd = (inst >> 7) & 31,
            funct3 = (inst >> 12) & 7, rs1 = (inst >> 15) & 31,
            rs2 = (inst >> 20) & 31, funct7 = inst >> 25;
        uint_32 IsR = (opcode == 51), IsItype = (opcode == 19) || (opcode == 3) || (opcode == 103) || (opcode == 115),
            IsIS = (opcode == 19) && ((funct3 == 1 || funct3 == 5)), IsI = (IsItype ^ IsIS),
            IsS = (opcode == 35), IsB = (opcode == 99), IsJ = (opcode == 111), IsU = (opcode == 23) || (opcode == 55);
        uint_32 immI = (funct7 << 5) | rs2, immIS = rs2,
            immS = (funct7 << 5) | rd, immU = (inst >> 12) << 12;
        uint_32 immB = ((inst >> 31) << 12) | (((inst >> 7) & 1) << 11) |
            (((inst >> 25) & 63) << 5) | (((inst >> 8) & 15) << 1);
        uint_32 immJ = ((inst >> 31) << 20) | (((inst >> 12) & 255) << 12) |
            (((inst >> 20) & 1) << 11) | (((inst >> 21) & 1023) << 1);
        immI = sign_extend(immI, 12), immS = sign_extend(immS, 12);
        immB = sign_extend(immI, 13), immJ = sign_extend(immJ, 21);
        uint_32 imm = (immI & SUB(0, IsI)) | (immIS & SUB(0, IsIS)) |
            (immS & SUB(0, IsS)) | (immB & SUB(0, IsB)) |
            (immU & SUB(0, IsU)) | (immJ & SUB(0, IsJ));
        fprintf(stderr, "immU = %u, IsU = %d, andval = %u\n", immU, IsU, SUB(0, IsU));
        fprintf(stderr, "immJ = %u, IsJ = %d, andval = %u\n", immJ, IsJ, SUB(0, IsJ));
        Instruction ret;
        ret.rs2 = rs2, ret.rs1 = rs1, ret.rd = rd, ret.imm = imm;
        switch(opcode){
            case 51:{ // R
                fprintf(stderr, "type R\n");
                ret.reg_write_en = true;
                ret.alu_src_a = 0;
                ret.alu_src_b = 0;
                ret.wb_sel = 0;
                switch(funct3){
                    case 0: ret.alu_sel = (funct7 & 32) ? ALU_sub : ALU_add; break;
                    case 1: ret.alu_sel = ALU_sll; break;
                    case 2: ret.alu_sel = ALU_slt; break;
                    case 3: ret.alu_sel = ALU_sltu; break;
                    case 4: ret.alu_sel = ALU_xor; break;
                    case 5: ret.alu_sel = (funct7 & 32) ? ALU_sra : ALU_srl; break;
                    case 6: ret.alu_sel = ALU_or; break;
                    case 7: ret.alu_sel = ALU_and; break;
                }
                break;
            }
            case 19:{ // I/I* arithmetic
                fprintf(stderr, "type I arithmetic\n");
                ret.reg_write_en = true;
                ret.alu_src_a = 0;
                ret.alu_src_b = 1;
                ret.wb_sel = 0;
                switch(funct3){
                    case 0: ret.alu_sel = ALU_addi; break;
                    case 1: ret.alu_sel = ALU_slli; break;
                    case 2: ret.alu_sel = ALU_slti; break;
                    case 3: ret.alu_sel = ALU_sltiu; break;
                    case 4: ret.alu_sel = ALU_xori; break;
                    case 5: ret.alu_sel = (funct7 & 32) ? ALU_srai : ALU_srli; break;
                    case 6: ret.alu_sel = ALU_ori; break;
                    case 7: ret.alu_sel = ALU_andi; break;
                }
                break;
            }
            case 3:{ // I load
                fprintf(stderr, "type I load\n");
                ret.reg_write_en = true;
                ret.alu_src_a = 0;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.mem_read = true;
                ret.wb_sel = 1;
                ret.mem_mask = funct3 & 3;
                ret.mem_unsigned = (funct7 & 4);
                break;
            }
            case 35:{ // S
                fprintf(stderr, "type S\n");
                ret.reg_write_en = false;
                ret.alu_src_a = 0;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.mem_write = true;
                ret.wb_sel = 0;
                ret.mem_mask = funct3 & 3;
                break;
            }
            case 99:{ // B
                fprintf(stderr, "type B\n");
                ret.reg_write_en = false;
                ret.is_branch = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.funct3 = funct3;
                break;
            }
            case 111:{ // Jal
                fprintf(stderr, "type Jal\n");
                ret.reg_write_en = true;
                ret.is_jump = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 2;
                break;
            }
            case 103:{ // Jalr
                fprintf(stderr, "type Jalr\n");
                ret.reg_write_en = true;
                ret.is_jump = true;
                ret.alu_src_a = 0;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 2;
                break;
            }
            case 23:{ // U add
                fprintf(stderr, "type U add\n");
                ret.reg_write_en = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 0;
                break;
            }
            case 55:{ // U load
                fprintf(stderr, "type U load\n");
                ret.reg_write_en = true;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_passb;
                ret.wb_sel = 0;
                break;
            }
            case 115:{ // I environment
                fprintf(stderr, "type I environment\n");
                break;
            }
            default: throw false;
        }
        return ret;
    }
};