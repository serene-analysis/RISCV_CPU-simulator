#include<algorithm>
#include<cstdio>
#include<vector>
#include "converter.h"
#include "decoder.h"
Converter conv;
Decoder dec;
signed main(){
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