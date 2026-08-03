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
    If.conv.fetch_instruction(0);
    for(auto now : If.conv.IMEM){
        dmem.DMEM[now.first] = (Unit<uint_8>){now.second, now.second};
    }
    If.PC.curr = If.PC.next = 0;
    int timestamp = 0;
    bool ended = false;
    int countdown = 1000;
    while(true){
        countdown -= ended;
        if(!countdown){
            printf("%u", rf.reg[10].curr & 255u);
            return 0;
        }
        rob.move(If, is, ars, brs, lsq, alu, bu, dmem, cdb, rf, rat);
        lsq.move(dmem.buf.next, cdb.Cbuf.curr, rob.Rbuf.curr, is.RSstall.next);
        if(!ended){
            is.move(ars.buf.next, brs.buf.next, lsq.buf.next, rat, rf, rob, If.stall.next, If.foretold.next, If.foretold_PC.next, ended);
        }
        if(!ended){
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