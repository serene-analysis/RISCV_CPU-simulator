#pragma once

#include "utils.h"
#include <iostream>
#include <string>
#include <map>
void Converter::read_instruction(){
    std::string str;
    while(std::cin >> str){
        if(str[0] == '@'){
            cur = std::stoul(str.substr(1), nullptr, 16);
        }
        else{
            uint_8 val = std::stoul(str, nullptr, 16);
            IMEM[cur] = val;
            cur++;
        }
    }
    return;
}
uint_32 Converter::fetch_instruction(uint_32 addr){
    uint_32 v0 = IMEM[addr], v1 = IMEM[addr+1], v2 = IMEM[addr+2], v3 = IMEM[addr+3];
    return v0 | (v1 << 8) | (v2 << 16) | (v3 << 24);
}