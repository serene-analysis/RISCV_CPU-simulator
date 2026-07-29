#include<algorithm>
#include<cstdio>
#include<vector>
#include "converter.h"
#include "decoder.h"
Converter conv;
Decoder dec;
void decoder_test(){
    // 1. ADD: add x5, x6, x7 (rd=5, rs1=6, rs2=7)
    // inst = 0x007302b3
    {
        uint_32 inst = 0x007302b3;
        // Expected Output:
        // rd = 5, rs1 = 6, rs2 = 7, imm = 0
        // ctrl.reg_write_en = true
        // ctrl.alu_src_a = 0, ctrl.alu_src_b = 0
        // ctrl.alu_sel = ALU_ADD, ctrl.wb_sel = 0
    }

    // 2. SUB: sub x5, x6, x7 (rd=5, rs1=6, rs2=7, funct7=0x20)
    // inst = 0x407302b3
    {
        uint_32 inst = 0x407302b3;
        // Expected Output:
        // rd = 5, rs1 = 6, rs2 = 7, imm = 0
        // ctrl.reg_write_en = true
        // ctrl.alu_sel = ALU_SUB, ctrl.wb_sel = 0
    }

    // 3. SLL: sll x5, x6, x7
    // inst = 0x007312b3
    {
        uint_32 inst = 0x007312b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_SLL
    }

    // 4. SLT: slt x5, x6, x7 (Signed)
    // inst = 0x007322b3
    {
        uint_32 inst = 0x007322b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_SLT
    }

    // 5. SLTU: sltu x5, x6, x7 (Unsigned)
    // inst = 0x007332b3
    {
        uint_32 inst = 0x007332b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_SLTU
    }

    // 6. XOR: xor x5, x6, x7
    // inst = 0x007342b3
    {
        uint_32 inst = 0x007342b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_XOR
    }

    // 7. SRL: srl x5, x6, x7 (Logical Right Shift)
    // inst = 0x007352b3
    {
        uint_32 inst = 0x007352b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_SRL
    }

    // 8. SRA: sra x5, x6, x7 (Arithmetic Right Shift, funct7=0x20)
    // inst = 0x407352b3
    {
        uint_32 inst = 0x407352b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_SRA
    }

    // 9. OR: or x5, x6, x7
    // inst = 0x007362b3
    {
        uint_32 inst = 0x007362b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_OR
    }

    // 10. AND: and x5, x6, x7
    // inst = 0x007372b3
    {
        uint_32 inst = 0x007372b3;
        // Expected Output: rd = 5, rs1 = 6, rs2 = 7; ctrl.alu_sel = ALU_AND
    }

    // 11. ADDI: addi x5, x6, -10 (imm12 = 0xFF6)
    // inst = 0xff630293
    {
        uint_32 inst = 0xff630293;
        // Expected Output:
        // rd = 5, rs1 = 6, imm = -10 (Sign Extended)
        // ctrl.reg_write_en = true, ctrl.alu_src_b = 1, ctrl.alu_sel = ALU_ADD
    }

    // 12. SLTI: slti x5, x6, 15
    // inst = 0x00f32293
    {
        uint_32 inst = 0x00f32293;
        // Expected Output: rd = 5, rs1 = 6, imm = 15; ctrl.alu_sel = ALU_SLT
    }

    // 13. SLTIU: sltiu x5, x6, 15
    // inst = 0x00f33293
    {
        uint_32 inst = 0x00f33293;
        // Expected Output: rd = 5, rs1 = 6, imm = 15; ctrl.alu_sel = ALU_SLTU
    }

    // 14. XORI: xori x5, x6, 15
    // inst = 0x00f34293
    {
        uint_32 inst = 0x00f34293;
        // Expected Output: rd = 5, rs1 = 6, imm = 15; ctrl.alu_sel = ALU_XOR
    }

    // 15. ORI: ori x5, x6, 15
    // inst = 0x00f36293
    {
        uint_32 inst = 0x00f36293;
        // Expected Output: rd = 5, rs1 = 6, imm = 15; ctrl.alu_sel = ALU_OR
    }

    // 16. ANDI: andi x5, x6, 15
    // inst = 0x00f37293
    {
        uint_32 inst = 0x00f37293;
        // Expected Output: rd = 5, rs1 = 6, imm = 15; ctrl.alu_sel = ALU_AND
    }

    // 17. SLLI (I* Type): slli x5, x6, 4 (shamt = 4)
    // inst = 0x00431293
    {
        uint_32 inst = 0x00431293;
        // Expected Output: rd = 5, rs1 = 6, imm = 4 (or 4 after mask); ctrl.alu_sel = ALU_SLL
    }

    // 18. SRLI (I* Type): srli x5, x6, 4 (shamt = 4)
    // inst = 0x00435293
    {
        uint_32 inst = 0x00435293;
        // Expected Output: rd = 5, rs1 = 6, imm = 4; ctrl.alu_sel = ALU_SRL
    }

    // 19. SRAI (I* Type): srai x5, x6, 4 (shamt = 4, funct7=0x20)
    // inst = 0x40435293
    {
        uint_32 inst = 0x40435293;
        // Expected Output: rd = 5, rs1 = 6, imm = 4; ctrl.alu_sel = ALU_SRA
    }

    // 20. LB: lb x5, -4(x6)
    // inst = 0xffc30283
    {
        uint_32 inst = 0xffc30283;
        // Expected Output:
        // rd = 5, rs1 = 6, imm = -4
        // ctrl.reg_write_en = true, ctrl.mem_read = true, ctrl.mem_write = false
        // ctrl.wb_sel = 1 (Mem), ctrl.mem_mask = 0 (Byte), ctrl.mem_unsigned = false
    }

    // 21. LH: lh x5, 8(x6)
    // inst = 0x00831283
    {
        uint_32 inst = 0x00831283;
        // Expected Output: rd = 5, rs1 = 6, imm = 8; ctrl.mem_mask = 1 (Half), ctrl.mem_unsigned = false
    }

    // 22. LW: lw x5, 12(x6)
    // inst = 0x00c32283
    {
        uint_32 inst = 0x00c32283;
        // Expected Output: rd = 5, rs1 = 6, imm = 12; ctrl.mem_mask = 2 (Word), ctrl.mem_unsigned = false
    }

    // 23. LBU: lbu x5, 4(x6)
    // inst = 0x00434283
    {
        uint_32 inst = 0x00434283;
        // Expected Output: rd = 5, rs1 = 6, imm = 4; ctrl.mem_mask = 0, ctrl.mem_unsigned = true
    }

    // 24. LHU: lhu x5, 4(x6)
    // inst = 0x00435283
    {
        uint_32 inst = 0x00435283;
        // Expected Output: rd = 5, rs1 = 6, imm = 4; ctrl.mem_mask = 1, ctrl.mem_unsigned = true
    }

    // 25. SB (S-Type): sb x7, 8(x6)  (imm = 8)
    // inst = 0x00730423
    {
        uint_32 inst = 0x00730423;
        // Expected Output:
        // rs1 = 6, rs2 = 7, imm = 8
        // ctrl.reg_write_en = false, ctrl.mem_read = false, ctrl.mem_write = true
        // ctrl.mem_mask = 0 (Byte)
    }

    // 26. SH (S-Type): sh x7, 8(x6)
    // inst = 0x00731423
    {
        uint_32 inst = 0x00731423;
        // Expected Output: rs1 = 6, rs2 = 7, imm = 8; ctrl.mem_write = true, ctrl.mem_mask = 1 (Half)
    }

    // 27. SW (S-Type): sw x7, -8(x6) (imm = -8 -> imm[11:5]=0x7f, imm[4:0]=0x18)
    // inst = 0xfe732c23
    {
        uint_32 inst = 0xfe732c23;
        // Expected Output: rs1 = 6, rs2 = 7, imm = -8; ctrl.mem_write = true, ctrl.mem_mask = 2 (Word)
    }

    // 28. BEQ: beq x5, x6, -16 (imm = -16)
    // inst = 0xfe6288e3
    {
        uint_32 inst = 0xfe6288e3;
        // Expected Output:
        // rs1 = 5, rs2 = 6, imm = -16
        // ctrl.reg_write_en = false, ctrl.is_branch = true, ctrl.br_unsigned = false
    }

    // 29. BNE: bne x5, x6, 16 (imm = +16)
    // inst = 0x00629863
    {
        uint_32 inst = 0x00629863;
        // Expected Output: rs1 = 5, rs2 = 6, imm = 16; ctrl.is_branch = true
    }

    // 30. BLT: blt x5, x6, 16
    // inst = 0x0062c863
    {
        uint_32 inst = 0x0062c863;
        // Expected Output: rs1 = 5, rs2 = 6, imm = 16; ctrl.br_unsigned = false
    }

    // 31. BGE: bge x5, x6, 16
    // inst = 0x0062d863
    {
        uint_32 inst = 0x0062d863;
        // Expected Output: rs1 = 5, rs2 = 6, imm = 16; ctrl.br_unsigned = false
    }

    // 32. BLTU: bltu x5, x6, 16
    // inst = 0x0062e863
    {
        uint_32 inst = 0x0062e863;
        // Expected Output: rs1 = 5, rs2 = 6, imm = 16; ctrl.br_unsigned = true
    }

    // 33. BGEU: bgeu x5, x6, 16
    // inst = 0x0062f863
    {
        uint_32 inst = 0x0062f863;
        // Expected Output: rs1 = 5, rs2 = 6, imm = 16; ctrl.br_unsigned = true
    }

    // 34. JAL (J-Type): jal x1, -2048 (imm = -2048)
    // inst = 0xfff000ef
    {
        uint_32 inst = 0xfff000ef;
        // Expected Output:
        // rd = 1 (ra), imm = -2048
        // ctrl.reg_write_en = true, ctrl.is_jump = true
        // ctrl.wb_sel = 2 (PC + 4 写回 rd)
    }

    // 35. JALR (I-Type): jalr x1, x5, 4 (imm = 4)
    // inst = 0x004280e7
    {
        uint_32 inst = 0x004280e7;
        // Expected Output:
        // rd = 1, rs1 = 5, imm = 4
        // ctrl.reg_write_en = true, ctrl.is_jump = true, ctrl.alu_src_a = 0 (rs1)
        // ctrl.wb_sel = 2 (PC + 4 写回 rd)
    }

    // 36. LUI (U-Type): lui x5, 0x12345 (imm = 0x12345000)
    // inst = 0x123452b7
    {
        uint_32 inst = 0x123452b7;
        // Expected Output:
        // rd = 5, imm = 0x12345000 (或无符号表示)
        // ctrl.reg_write_en = true, ctrl.alu_src_b = 1, ctrl.alu_sel = ALU_PASS_B, ctrl.wb_sel = 0
    }

    // 37. AUIPC (U-Type): auipc x5, 0x12345 (imm = 0x12345000)
    // inst = 0x12345217
    {
        uint_32 inst = 0x12345217;
        // Expected Output:
        // rd = 5, imm = 0x12345000
        // ctrl.reg_write_en = true, ctrl.alu_src_a = 1 (PC), ctrl.alu_src_b = 1 (Imm)
        // ctrl.alu_sel = ALU_ADD, ctrl.wb_sel = 0
    }
}

signed main(){
    decoder_test();
    return 0;
    conv.read_instruction();
    uint_32 PC = 0;
    while(true){
        uint_32 inst = conv.fetch_instruction(PC);
        if(inst == 0x0ff00513){
            break;
        }
        Instruction info = dec.decode(inst);

        PC += 4;
    }
    return 0;
}