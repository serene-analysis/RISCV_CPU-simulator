#include<algorithm>
#include<cstdio>
#include<vector>
#include "ALU.h"
#include "ArithRS.h"
#include "BranchRS.h"
#include "BU.h"
#include "CDB.h"
#include "converter.h"
#include "decoder.h"
#include "DMEM.h"
#include "IF.h"
#include "IS.h"
#include "LSQ.h"
#include "RAT.h"
#include "RegFile.h"
#include "ROB.h"
#include "utils.h"
signed main(){
    #ifndef ONLINE_JUDGE
    setbuf(stderr, NULL);
    //freopen("err.out","w",stderr);
    #endif
    ALU alu;
    ArithRS ars;
    BranchRS brs;
    BU bu;
    CDB cdb;
    DMEM dmem;
    IF If;
    IS is;
    LSQ lsq;
    RAT rat;
    RegFile rf;
    ROB rob;
    If.conv.read_instruction();
    for(auto now : If.conv.IMEM){
        dmem.DMEM[now.first] = (Unit<uint_8>){now.second, now.second};
    }
    If.PC.curr = If.PC.next = 0;
    int timestamp = 0;
    bool ended = false, always = true;
    uint_32 end_tag = 0;
    while(!ended && (timestamp < 200000 || always)){
        // DEBUG: fprintf(stderr, "timestamp = %d\n", ++timestamp);
        rob.move(If, is, ars, brs, lsq, alu, bu, dmem, cdb, rf, rat, ended, end_tag);
        lsq.move(dmem.buf.next, cdb.Cbuf.curr, rob.Rbuf.curr, is.RSstall.next);
        if(ended == false){
            is.move(ars.buf.next, brs.buf.next, lsq.buf.next, rat, rf, rob, If.stall.next, If.foretold.next, If.foretold_PC.next, end_tag);
        }
        if(ended == false){
            If.move(is.buf.next);
        }
        dmem.move(cdb.LSbuf.next, lsq.stall.next);
        cdb.move(rob, alu.accepted.next, bu.accepted.next, dmem.accepted.next, alu.submitted_id.next, bu.submitted_id.next, dmem.submitted_id.next);
        bu.move(cdb.Bbuf.next, brs.stall.next);
        brs.move(bu.buf.next, cdb.Cbuf.curr, is.RSstall.next);
        ars.move(alu.buf.next, cdb.Cbuf.curr, is.RSstall.next);
        alu.move(cdb.Abuf.next, ars.stall.next);

        rob.tick();
        rf.tick();
        rat.tick();
        lsq.tick();
        is.tick();
        If.tick();
        dmem.tick();
        cdb.tick();
        bu.tick();
        brs.tick();
        ars.tick();
        alu.tick();
    }
    return 0;
}
/*
src 文件夹下实现了一个 RISCV CPU 的 C++ 模拟器，使用的是五级流水 Tomasulo，请你为这份代码 debug。上层的 README.md 有具体的要求，data 下有样例和测试数据（.dump 文件，.c 文件的末尾有注释标明的标准答案）。先不管所有附加要求，先把输出调对，然后再考虑别的。在所有作出修改的地方用注释标明。如果有拿不准为什么这么写的，要向我提问。
我之前写过一个串行的暴力实现，所以 Decoder 部分、ALU 的具体判断部分无需检查。
*/