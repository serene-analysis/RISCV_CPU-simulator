#pragma once

#include <cassert>
#include <cstdio>
#include <map>
#include <string>

typedef char int_8;
typedef unsigned char uint_8;
typedef short int_16;
typedef unsigned short uint_16;
typedef int int_32;
typedef unsigned int uint_32;

const static bool logout = false;

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
    uint_8 funct3;

    uint_32 rs2, rs1, rd, imm;

    Instruction(){
        rs1 = rs2 = rd = imm = reg_write_en = alu_src_a = alu_src_b = alu_sel = mem_read = mem_write =
            mem_mask = mem_unsigned = wb_sel = is_branch = is_jump = funct3 = 0;
        return;
    }
    
    void out(){
        logout && fprintf(stderr, "rs1 = %u, rs2 = %u, rd = %u, imm = %u\n", rs1, rs2, rd, imm);
        logout && fprintf(stderr, "reg_write_en = %d\n", reg_write_en);
        logout && fprintf(stderr, "alu_src_a = %d, alu_src_b = %d, alu_sel = %u\n", alu_src_a, alu_src_b, uint_32(alu_sel));
        logout && fprintf(stderr, "mem_read = %d, mem_write = %d, mem_mask = %u, mem_unsigned = %d\n",
            mem_read, mem_write, uint_32(mem_mask), mem_unsigned);
        logout && fprintf(stderr, "wb_sel = %u\n", uint_32(wb_sel));
        logout && fprintf(stderr, "is_branch = %d, is_jump = %d, funct3 = %u\n", is_branch, is_jump, uint_32(funct3));
        return;
    }
};

template<typename tp>
struct Unit{
    tp curr;
    tp next;
    Unit() : curr(tp()), next(tp()){}
    Unit(tp cur, tp nxt) : curr(cur), next(nxt) {}
    void tick(){
        curr = next;
        return;
    }
};

template<typename tp, int n>
struct Queue{
    Unit<tp> v[n + 2];
    Unit<int> l{1, 1}, r{1, 1}, cnt{1, 1};
    bool empty(){
        return l.curr == r.curr;
    }
    int nxt(int x){
        return x == n + 1 ? 1 : x + 1;
    }
    bool full(){
        return l.curr == nxt(r.curr);
    }
    int size(){
        if(l.curr <= r.curr){
            return r.curr - l.curr;
        }
        return r.curr + n - l.curr;
    }
    bool nearly_full(){
        return size() >= n - 4;
    }
    tp front(){
        return v[l.curr].curr;
    }
    void pop(){
        l.next = nxt(l.curr);
        return;
    }
    void push(tp x){
        v[r.curr].next = x, r.next = nxt(r.curr), cnt.next++;
        return;
    }
};

struct Converter{
    uint_32 cur = 0;
    std::map<uint_32, uint_8> IMEM;
    void read_instruction();
    uint_32 fetch_instruction(uint_32 addr);
};
struct Decoder{
    Instruction decode(uint_32 inst);
};
struct DMEM;
struct RegFile;

struct IF_IS_Buffer;
struct IF{
    Converter conv;
    Unit<uint_32> PC, foretold_PC, redirected_PC;
    Unit<bool> stall, foretold, redirected;
    void move(IF_IS_Buffer &buf);
    void flush();
    void tick();
};

struct IF_IS_Buffer{
    bool valid = false;
    uint_32 inst;
    uint_32 PC;
};

struct IS_ArithRS_Buffer;
struct IS_BranchRS_Buffer;
struct IS_LSQ_Buffer;
struct RAT;
struct ROB;
struct IS{
    Decoder dec;
    Unit<IF_IS_Buffer> buf;
    Unit<bool> RSstall, ROBstall, flushed;
    void move(IS_ArithRS_Buffer &Abuf, IS_BranchRS_Buffer &Bbuf, IS_LSQ_Buffer &LSbuf,
        RAT &rat, RegFile &regfile, ROB &rob, bool &IFStall, bool &Foretold, uint_32 &Foretold_PC, uint_32 &end_tag);
    void flush();
    void tick();
};

struct IS_ArithRS_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    bool alu_src_a, alu_src_b;
    uint_8 alu_sel;
    uint_32 imm, PC;
};
struct ArithRSEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    bool alu_src_a, alu_src_b;
    uint_8 alu_sel;
    uint_32 imm, PC;
};

struct RATEntry{
    bool busy = false;
    uint_32 rob_tag;
};
struct RAT{
    const static int EntrySize_ = 32;
    Unit<RATEntry> ent[EntrySize_];
    void query(const uint_8 &pos, uint_32 &ret);
    void mark(const uint_8 &pos, const uint_32 &value);
    void unlock(const uint_8 &pos, const uint_32 &value);
    void flush();
    void tick();
};

struct ROBEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_8 type; // 1: Arithmetic, 2: Branch, 3: Memory
    uint_8 rd;
    uint_32 value;
    bool predicted_jump, branch_misjumped;
    uint_32 nojump_dest, jump_dest;
    bool done; // can be commited or not
    uint_32 write_addr;
    bool is_read;
    uint_8 remaining_round;
    bool mem_unsigned;
    uint_8 mem_mask;
};
struct ROB_CommitStore_Buffer{
    bool valid = false;
    uint_32 rob_tag;
};

struct ArithRS;
struct BranchRS;
struct LSQ;

struct ArithRS_ALU_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 operand_a, operand_b;
    uint_8 alu_sel;
};

struct ALU_CDB_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 alu_result;
    uint_32 submitted_id;
};
struct ALU{
    const static int Memsize_ = 8;
    Unit<ArithRS_ALU_Buffer> buf;
    Unit<ALU_CDB_Buffer> mem[Memsize_];
    Unit<uint_32> submitted_id;
    Unit<bool> accepted, flushed;
    void move(ALU_CDB_Buffer &Cbuf, bool &ArithRSStall);
    void flush();
    void tick();
};

struct BU_CDB_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    bool mispredicted;
    uint_32 actual_dest;
    uint_32 submitted_id;
};

struct BranchRS_BU_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 vj, vk;
    uint_8 funct3;
    bool is_jump, is_jalr;
    bool predicted_jump;
    uint_32 nojump_dest, jump_dest;
    uint_32 imm, PC;
};
struct BU{
    const static int Memsize_ = 8;
    Unit<BranchRS_BU_Buffer> buf;
    Unit<BU_CDB_Buffer> mem[Memsize_];
    Unit<uint_32> submitted_id;
    Unit<bool> accepted, flushed;
    void move(BU_CDB_Buffer &Cbuf, bool &BranchRSStall);
    void flush();
    void tick();
};
struct CDB;
struct ROB{
    const static int Queuesize_ = 32;
    Queue<ROBEntry, Queuesize_> qu;
    Unit<ROB_CommitStore_Buffer> Rbuf;
    Unit<bool> flushed;
    void allocate(const uint_8 &type, const uint_8 &rd, const uint_32 PC, const bool &predicted_jump,
        const uint_32 &nojump_dest, const uint_32 &jump_dest, bool &ISStall, uint_32 &ret);
    void move(IF &If, IS &Is, ArithRS &Ars, BranchRS &Brs, LSQ &Lsq, ALU &Alu, BU &Bu,
        DMEM &Dmem, CDB &Cdb, RegFile &Rf, RAT &Rat, bool &ended, uint_32 &end_tag);
    void flush();
    void tick();
};
struct CDB_Broadcast_Buffer{
    bool valid = false;
    uint_32 rob_tag, result;
};
struct ArithRS{
    const static int EntrySize_ = 32;
    Unit<ArithRSEntry> ent[EntrySize_];
    Unit<IS_ArithRS_Buffer> buf;
    Unit<bool> stall, flushed;
    void move(ArithRS_ALU_Buffer &ABuf, const CDB_Broadcast_Buffer &CBuf, bool &ISStall);
    void flush();
    void tick();
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
struct BranchRSEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vj, qj, vk, qk;
    uint_8 funct3;
    bool is_jump, is_jalr;
    bool predicted_jump;
    uint_32 nojump_dest, jump_dest;
    uint_32 imm, PC;
};

struct BranchRS{
    const static int EntrySize_ = 32;
    Unit<BranchRSEntry> ent[EntrySize_];
    Unit<IS_BranchRS_Buffer> buf;
    Unit<bool> stall, flushed;
    void move(BranchRS_BU_Buffer &Bbuf, CDB_Broadcast_Buffer &Cbuf, bool &ISStall);
    void flush();
    void tick();
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
struct LoadEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vbase, qbase;
    bool base_ready;
    bool mem_unsigned;
    uint_8 mem_mask;
    uint_32 rd;
    uint_32 imm;
};
struct StoreEntry{
    bool busy = false;
    uint_32 rob_tag;
    uint_32 vbase, qbase, vdata, qdata;
    bool base_ready, data_ready;
    bool mem_unsigned;
    uint_8 mem_mask;
    uint_32 imm;
    bool submitted;
};

struct LSQ_DMEM_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 rd, write_data, goal_addr;
    bool mem_read, mem_write, mem_unsigned;
    uint_8 mem_mask;
};

struct LSQ{
    const static int EntrySize_ = 32;
    Unit<IS_LSQ_Buffer> buf;
    Queue<LoadEntry, EntrySize_> lq;
    Queue<StoreEntry, EntrySize_> sq;
    Unit<uint_32> submitted_id;
    Unit<bool> accepted, stall, flushed;
    void move(LSQ_DMEM_Buffer &Dbuf, CDB_Broadcast_Buffer &Cbuf, const ROB_CommitStore_Buffer &Rbuf, bool &ISStall);
    void flush();
    void tick();
};

struct DMEMEntry{
    bool valid = false;
    uint_32 rob_tag;
    uint_32 rd, load_result, remaining_round;
    uint_32 write_addr, write_data;
    bool mem_read, mem_write, mem_unsigned;
    uint_8 mem_mask;
};

struct DMEM_CDB_Buffer{
    bool valid = false;
    uint_32 rob_tag;
    bool is_read;
    uint_32 rd, load_result;
    uint_32 write_addr, write_data;
    uint_32 submitted_id;
    bool mem_unsigned;
    uint_8 mem_mask;
};

struct CDB{
    Unit<ALU_CDB_Buffer> Abuf;
    Unit<BU_CDB_Buffer> Bbuf;
    Unit<DMEM_CDB_Buffer> LSbuf;
    Unit<CDB_Broadcast_Buffer> Cbuf;
    Unit<uint_32> last_id, last_type;
    Unit<bool> flushed;
    void move(ROB &rob, bool &UseA, bool &UseB, bool &UseLS, uint_32 &IdA, uint_32 &IdB, uint_32 &IdLS);
    void flush();
    void tick();
};

enum ALUType{ ALU_add, ALU_sub, ALU_and, ALU_or, ALU_xor, ALU_sll, ALU_srl, ALU_sra,
    ALU_slt, ALU_sltu, ALU_addi, ALU_andi, ALU_ori, ALU_xori, ALU_slli, ALU_srli, ALU_srai,
    ALU_slti, ALU_sltiu, ALU_passb};

uint_32 ADD(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    /*bool carry = false;
    for(int i=0;i<32;i++){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        bool got = (lv & carry) | (rv & carry) | (lv & rv);
        ret |= (lv ^ rv ^ carry) << i;
        carry = got;
    }*/
    ret = now + oth;
    return ret;
}

uint_32 SUB(uint_32 now, uint_32 oth){
    /*for(int i=0;i<32;i++){
        oth ^= (1u << i);
    }
    oth = ADD(oth, 1);
    return ADD(now, oth);*/
    return now - oth;
}

uint_32 AND(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    /*for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) & ((oth >> i) & 1)) << i;
    }*/
    ret = now & oth;
    return ret;
}

uint_32 OR(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    /*for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) | ((oth >> i) & 1)) << i;
    }*/
    ret = now | oth;    
    return ret;
}

uint_32 XOR(uint_32 now, uint_32 oth){
    uint_32 ret = 0;
    /*for(int i=0;i<32;i++){
        ret |= (((now >> i) & 1) ^ ((oth >> i) & 1)) << i;
    }*/
    ret = now ^ oth;
    return ret;
}

uint_32 MUX(uint_32 v0, uint_32 v1, bool type){
    /*if(type == 1){
        logout && fprintf(stderr, "MUX: %u, %u, %d\n", v0, v1, type);
    }*/
    /*uint_32 lv = !type, rv = type;
    for(int i=1;i<32;i++){
        lv |= ((lv & 1) << i), rv |= ((rv & 1) << i);
    }
    return OR(AND(lv, v0), AND(rv, v1));*/
    return type ? v1 : v0;
}

bool ISZERO(uint_32 now){
    /*uint_32 ret = 1;
    for(int i=0;i<32;i++){
        bool nv = (now >> i) & 1;
        ret &= !(nv & nv);
    }
    return ret;*/
    return now == 0;
}

uint_32 SLL(uint_32 now, uint_32 oth){
    oth &= 31;
    uint_32 ret = 0;
    /*for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=d;i<32;i++){
            uint_32 nv = MUX(0, (now >> (i - d)) & 1, equ);
            ret |= (nv << i);
        }
    }*/
    ret = now << oth;
    return ret;
}

uint_32 SRL(uint_32 now, uint_32 oth){
    oth &= 31;
    uint_32 ret = 0;
    /*for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=0;i<32-d;i++){
            uint_32 nv = MUX(0, (now >> (i + d)) & 1, equ);
            ret |= (nv << i);
        }
    }*/
    ret = now >> oth;
    return ret;
}

uint_32 SRA(uint_32 now, uint_32 oth){
    oth &= 31;
    uint_32 ret = 0;
    /*for(uint_32 d=0;d<32;d++){
        bool equ = ISZERO(SUB(oth, d));
        for(int i=0;i<32-d;i++){
            uint_32 nv = MUX(0, (now >> (i + d)) & 1, equ);
            ret |= (nv << i);
        }
        for(int i=32-d;i<32;i++){
            uint_32 nv = MUX(0, (now >> 31) & 1, equ);
            ret |= (nv << i);
        }
    }*/
    ret = static_cast<uint_32>(static_cast<int_32>(now) >> oth);
    return ret;
}

bool SLTU(uint_32 now, uint_32 oth){ // (now < oth), unsigned
    /*bool yes = false, no = false;
    for(int i=31;i>=0;i--){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        yes = yes | ((!no) & (lv != rv) & ISZERO(lv));
        no = no | ((!yes) & (lv != rv) & ISZERO(rv));
    }
    return yes;*/
    return now < oth;
}

bool SLT(uint_32 now, uint_32 oth){
    /*bool luv = ((now >> 31) & 1), ruv = ((oth >> 31) & 1);
    bool dif = (luv != ruv);
    bool yes = MUX(false, ISZERO(ruv), dif), no = MUX(false, ISZERO(luv), dif), rev = (!dif) & (!ISZERO(luv));
    for(int i=31;i>=0;i--){
        bool lv = (now >> i) & 1, rv = (oth >> i) & 1;
        yes = yes | ((!no) & (lv != rv) & ISZERO(lv));
        no = no | ((!yes) & (lv != rv) & ISZERO(rv));
    }
    return MUX(yes, !yes, rev);// can't put no because when now == oth, no == false*/
    return static_cast<int_32>(now) < static_cast<int_32>(oth);
}

bool EQUAL(uint_32 now, uint_32 oth){
    return ISZERO(SUB(now, oth));
}

uint_32 sign_extend(uint_32 imm, int N) {
    int_32 val = (int_32)imm;
    int shift = 32 - N;
    return uint_32((val << shift) >> shift);
}