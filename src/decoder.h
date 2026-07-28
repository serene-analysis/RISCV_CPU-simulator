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
            immS = (funct7 << 5) | rd, immU = (inst >> 12);
        uint_32 immB = ((inst >> 31) << 12) | (((inst >> 7) & 1) << 11) |
            (((inst >> 25) & 63) << 5) | (((inst >> 8) & 15) << 1);
        uint_32 immJ = ((inst >> 31) << 20) | (((inst >> 12) & 255) << 12) |
            (((inst >> 20) & 1) << 11) | (((inst >> 21) & 1023) << 1);
        uint_32 imm = (immI & ((IsI << 31) >> 31)) | (immIS & ((IsIS << 31) >> 31)) |
            (immS & ((IsS << 31) >> 31)) | (immB & ((IsB << 31) >> 31)) |
            (immU & ((IsU << 31) >> 31)) | (immJ & ((IsJ << 31) >> 31));
        Instruction ret;
        ret.rs2 = rs2, ret.rs1 = rs1, ret.rd = rd, ret.imm = imm;
        switch(opcode){
            case 51:{ // R
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
                ret.reg_write_en = false;
                ret.is_branch = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.br_unsigned = (funct3 == 6 || funct3 == 7);
                break;
            }
            case 111:{ // Jal
                ret.reg_write_en = false;
                ret.is_jump = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 2;
                break;
            }
            case 103:{ // Jalr
                ret.reg_write_en = false;
                ret.is_jump = true;
                ret.alu_src_a = 0;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 2;
                break;
            }
            case 23:{ // U add
                ret.reg_write_en = true;
                ret.alu_src_a = 1;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_add;
                ret.wb_sel = 0;
                break;
            }
            case 55:{ // U load
                ret.reg_write_en = true;
                ret.alu_src_b = 1;
                ret.alu_sel = ALU_passb;
                ret.wb_sel = 0;
                break;
            }
            case 115:{ // I environment
                break;
            }
            default: throw false;
        }
        return ret;
    }
};