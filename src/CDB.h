#pragma once

#include "utils.h"

struct CDB{
    Unit<ALU_CDB_Buffer> Abuf;
    Unit<BranchRS_BU_Buffer> Bbuf;
    Unit<LSQ_CDB_Buffer> Lbuf;
    Unit<CDB_Broadcast_Buffer> Cbuf;
    void move(){
        
    }
};