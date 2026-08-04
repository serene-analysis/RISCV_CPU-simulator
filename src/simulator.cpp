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
#include "IF_IS.h"
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
    IF_IS if_is;
    LSQ lsq;
    RAT rat;
    RegFile rf;
    ROB rob;
    if_is.conv.read_instruction();
    for(auto now : if_is.conv.IMEM){
        dmem.DMEM[now.first] = (Unit<uint_8>){now.second, now.second};
    }
    if_is.PC.curr = if_is.PC.next = 0;
    int timestamp = 0;
    bool ended = false, always = true;
    uint_32 end_tag = 0;
    while(!ended && (timestamp < 200000 || always)){
        // DEBUG: fprintf(stderr, "timestamp = %d\n", ++timestamp);
        rob.move(if_is, ars, brs, lsq, alu, bu, dmem, cdb, rf, rat, ended, end_tag);
        lsq.move(dmem.buf.next, cdb.Cbuf.curr, rob.Rbuf.curr, if_is.RSstall.next);
        if(ended == false){
            if_is.move(ars.buf.next, brs.buf.next, lsq.buf.next, rat, rf, rob, end_tag);
        }
        dmem.move(cdb.LSbuf.next, lsq.stall.next);
        cdb.move(rob, alu.accepted.next, bu.accepted.next, dmem.accepted.next, alu.submitted_id.next, bu.submitted_id.next, dmem.submitted_id.next);
        bu.move(cdb.Bbuf.next, brs.stall.next);
        brs.move(bu.buf.next, cdb.Cbuf.curr, if_is.RSstall.next);
        ars.move(alu.buf.next, cdb.Cbuf.curr, if_is.RSstall.next);
        alu.move(cdb.Abuf.next, ars.stall.next);

        rob.tick();
        rf.tick();
        rat.tick();
        lsq.tick();
        if_is.tick();
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
src 文件夹下实现了一个 RISCV CPU 的 C++ 模拟器，使用的是五级流水 Tomasulo，上层的 README.md 有具体的要求，data 下有样例和测试数据（.dump 文件，.c 文件的末尾有注释标明的标准答案）。
这份代码已经是正确的了，但跑得太慢，我试图加入分支预测，但当我去掉 IF_IS.h 里面对无条件跳转的判断时，我连样例都过不去了。为什么呢？
*/