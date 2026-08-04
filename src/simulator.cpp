#include <algorithm>
#include <cstdio>
#include <vector>
#include <random>
#include <chrono>
std::mt19937 tian(std::chrono::high_resolution_clock::now().time_since_epoch().count());
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
    int move_order[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int tick_order[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    while(!ended && (timestamp < 200000 || always)){
        if(timestamp % 100 == 0){
            std::shuffle(move_order + 1, move_order + 9 + 1, tian);
            std::shuffle(tick_order + 1, tick_order + 11 + 1, tian);
        }
        
        for(int i=1;i<=9;i++){
            if(move_order[i] == 1) rob.move(if_is, ars, brs, lsq, alu, bu, dmem, cdb, rf, rat, ended, end_tag);
            if(move_order[i] == 2) lsq.move(dmem.buf.next, cdb.Cbuf.curr, rob.Rbuf.curr, if_is.RSstall.next);
            if(move_order[i] == 3) if_is.move(ars.buf.next, brs.buf.next, lsq.buf.next, rat, rf, rob, end_tag);
            if(move_order[i] == 4) dmem.move(cdb.LSbuf.next, lsq.stall.next);
            if(move_order[i] == 5) cdb.move(rob, alu.accepted.next, bu.accepted.next, dmem.accepted.next, alu.submitted_id.next, bu.submitted_id.next, dmem.submitted_id.next);
            if(move_order[i] == 6) bu.move(cdb.Bbuf.next, brs.stall.next);
            if(move_order[i] == 7) brs.move(bu.buf.next, cdb.Cbuf.curr, if_is.RSstall.next);
            if(move_order[i] == 8) ars.move(alu.buf.next, cdb.Cbuf.curr, if_is.RSstall.next);
            if(move_order[i] == 9) alu.move(cdb.Abuf.next, ars.stall.next);
        }

        for(int i=1;i<=11;i++){
            if(tick_order[i] == 1) rob.tick();
            if(tick_order[i] == 2) rf.tick();
            if(tick_order[i] == 3) rat.tick();
            if(tick_order[i] == 4) lsq.tick();
            if(tick_order[i] == 5) if_is.tick();
            if(tick_order[i] == 6) dmem.tick();
            if(tick_order[i] == 7) cdb.tick();
            if(tick_order[i] == 8) bu.tick();
            if(tick_order[i] == 9) brs.tick();
            if(tick_order[i] == 10) ars.tick();
            if(tick_order[i] == 11) alu.tick();
        }
        timestamp++;
    }
    fprintf(stderr, "cycles=%d instructions=%u branches=%u branch_accuracy=%.2f%%\n",
        timestamp, rob.committed, rob.branches,
        rob.branches ? 100.0 * (rob.branches - rob.branch_mispred) / rob.branches : 100.0);
    return 0;
}
