#include<algorithm>
#include<cstdio>
#include<vector>
#include "converter.h"
#include "decoder.h"
#include "ALU.h"
#include "DMEM.h"
#include "RegFile.h"
signed main(){
    #ifndef ONLINE_JUDGE
    setbuf(stderr, NULL);
    freopen("err.out","w",stderr);
    #endif
    Converter conv;
    Decoder dec;
    ALU alu;
    DMEM dmem;
    RegFile regfile;
    conv.read_instruction();
    dmem.DMEM = conv.IMEM;
    uint_32 PC = 0;
    int timestamp = 0;
    bool onl = false;
    #ifdef ONLINE_JUDGE
    onl = true;
    #endif
    #ifndef ONLINE_JUDGE
    onl = true;
    #endif
    while(true && (++timestamp < 20 || onl)){
        logout && fprintf(stderr, "\n\ntimestamp = %d \n\n\n", timestamp);
        uint_32 inst = conv.fetch_instruction(PC);
        logout && fprintf(stderr, "PC = %u = 0x%x, inst = %u\n", PC, PC, inst);
        if(inst == 0x0ff00513){
            printf("%u\n", regfile.read(10) & 255u);
            break;
        }
        Instruction info = dec.decode(inst);
        info.out();
        uint_32 vrs1 = regfile.read(info.rs1), vrs2 = regfile.read(info.rs2);
        uint_32 alu_result = alu.ALU_operation(vrs1, vrs2, info.alu_src_a, info.alu_src_b, info.alu_sel, info.imm, PC);
        logout && fprintf(stderr, "alu_result = %u\n", alu_result);
        uint_32 dmem_result = dmem.DMEM_operation(vrs2,
                info.mem_read, info.mem_write, info.mem_unsigned, info.mem_mask, alu_result);
        logout && fprintf(stderr, "dmem_result = %u\n", dmem_result);
        uint_32 write_back = 0;
        switch(info.wb_sel){
            case 0: write_back = alu_result; break;
            case 1: write_back = dmem_result; break; // In fact we need another ADDER
            case 2: write_back = ADD(PC, 4); break;
        }
        regfile.write(info.rd, info.reg_write_en, write_back);
        bool condition_met = false;
        if(info.is_branch){
            logout && fprintf(stderr, "vrs1 = %u, vrs2 = %u\n", vrs1, vrs2);
            switch(info.funct3){
                case 0: condition_met = EQUAL(vrs1, vrs2); break;
                case 1: condition_met = !EQUAL(vrs1, vrs2); break;
                case 4: condition_met = SLT(vrs1, vrs2); break;
                case 5: condition_met = !SLT(vrs1, vrs2); break;
                case 6: condition_met = SLTU(vrs1, vrs2); break;
                case 7: condition_met = !SLTU(vrs1, vrs2); break;
                default:{
                    fflush(stderr);
                    throw false;
                }
            }
        }
        logout && fprintf(stderr, "condition_met = %d\n", condition_met);
        uint_32 cleared = (alu_result >> 1) << 1;
        PC = MUX(ADD(PC, 4), MUX(cleared, alu_result, info.alu_src_a), info.is_jump | (info.is_branch & condition_met));
    }
    return 0;
}