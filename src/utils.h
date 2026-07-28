#pragma once

typedef unsigned char uint_8;
typedef unsigned int uint_32;

struct Instruction{
    // 1. 寄存器写使能
    bool reg_write_en;   // RegWEn: 是否要将计算结果写入目标寄存器 rd

    // 2. ALU 源操作数选择 (用于决定 ALU 输入来自哪里)
    bool alu_src_a;      // ASel: 0 = RegReadData1 (来自 rs1/RS), 1 = PC
    bool alu_src_b;      // BSel: 0 = RegReadData2 (来自 rs2/RS), 1 = Imm (来自 imm)

    // 3. ALU 功能选择码
    uint_8 alu_sel;     // ALUSel: 告诉 ALU 执行具体的算术/逻辑运算

    // 4. 内存 (DMEM) 访问控制
    bool mem_read;       // MemRead: 是否读内存 (Load 指令)
    bool mem_write;      // MemWrite: 是否写内存 (Store 指令)
    uint_8 mem_mask;    // 访存字节控制: 0 = 1 Byte (b), 1 = 2 Bytes (h), 2 = 4 Bytes (w)
    bool mem_unsigned;   // 访存符号控制: true = 无符号扩展 (lbu/lhu), false = 有符号扩展 (lb/lh)

    // 5. 写回数据源选择 (WBSel)
    uint_8 wb_sel;      // 0 = ALU 结果, 1 = 内存读取值 (Mem), 2 = PC + 4 (用于 JAL/JALR 存返回地址)

    // 6. 分支与跳转控制
    bool is_branch;      // 是否为条件分支指令 (BEQ, BNE, BLT 等)
    bool is_jump;        // 是否为无条件跳转指令 (JAL, JALR)
    bool br_unsigned;    // 分支比较符控制: true = 无符号比较 (BLTU, BGEU), false = 有符号比较

    uint_32 rs2, rs1, rd, imm;
};

enum ALUType{ ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu, ALU_addi, ALU_andi, ALU_ori, ALU_xori, ALU_slli, ALU_srli, ALU_srai,
    ALU_slti, ALU_sltiu, ALU_passb};