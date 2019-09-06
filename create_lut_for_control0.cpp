
typedef unsigned char u8;
typedef short         s16;
typedef unsigned int  u32;
typedef unsigned long long u64;


#define ABIT(m_n) static const int m_n ## _ ## ABIT
#define DBIT(m_n) static const int m_n ## _ ## DBIT

ABIT(OPCODE)[8] = { 0,1,2,3,4,5,6,7 };
ABIT(LAST_Q)[3] = { 8,9,11 };
ABIT(WRITE_READY_Q) = 14; 

#define UNUSED_COUNT 5
ABIT(UNUSED)[UNUSED_COUNT] = { 10, 12, 13, 15, 16 };

DBIT(ALU_WRACCal)   = 3;
DBIT(ALU_RDACCal)   = 2;
DBIT(BAK_SAVEal)    = 1;
DBIT(BAK_RESTOREal) = 0;

DBIT(G_LEFT)   = 7;
DBIT(G_RIGHT)  = 6;
DBIT(G_UP)     = 5;
DBIT(G_DOWN)   = 4;


#define A(m_a)       (!!(address & 1UL<<(m_a ## _ ## ABIT)))
#define Ai(m_a, m_i) (!!(address & 1UL<<(m_a ## _ ## ABIT[m_i])))
#define D(m_d, m_v)  (data |= ((u64)(!!(m_v)))<<(m_d ## _ ## DBIT))

u8 data_for_address(u32 address) {
    
    u8 data = 0;
    
    u8 opcode = 0;
    for (int i=0;i<8;i++) {
        opcode |= (Ai(OPCODE, i))<<i;
    }
    
    enum LAST_PORT {
        LP_NIL   = 0b000,
        LP_LEFT  = 0b100,
        LP_RIGHT = 0b101,
        LP_UP    = 0b110,
        LP_DOWN  = 0b111,
    };
    u8 last = 0;
    for (int i=0;i<3;i++) {
        last |= (Ai(LAST_Q, i))<<i;
    }
    
    enum VPORT {
        VP_ACC   = 0b000,
        VP_LEFT  = 0b001,
        VP_RIGHT = 0b010,
        VP_UP    = 0b011,
        VP_DOWN  = 0b100,
        VP_ANY   = 0b101,
        VP_LAST  = 0b110,
        VP_NIL   = 0b111,
    };
    
    
    char VPORT_IN  = 0b111;
    if (0b01010000 == (opcode & 0b11111000) ||  // JRO A
        0b01011000 == (opcode & 0b11111000) ||  // ADD A
        0b01100000 == (opcode & 0b11111000) ||  // SUB A
        0b11000000 == (opcode & 0b11000000) ){  // MOV A B
        VPORT_IN = opcode & 0b00000111;
    }
    char VPORT_OUT = 0b111;
    if (0b10000000 == (opcode & 0b11000000) ||  // MOV [X] B
        0b11000000 == (opcode & 0b11000000) ){  // MOV A B
        VPORT_OUT = (opcode & 0b00111000) >> 3;
    }
    
    if (A(WRITE_READY_Q)) {
        switch(VPORT_OUT) { default: break;
            case VP_LEFT : {  D(G_LEFT , 1); } break;
            case VP_RIGHT: {  D(G_RIGHT, 1); } break;
            case VP_UP   : {  D(G_UP   , 1); } break;
            case VP_DOWN : {  D(G_DOWN , 1); } break;
            case VP_LAST : {
                switch(last) {
                    case LP_LEFT : {  D(G_LEFT , 1); } break;
                    case LP_RIGHT: {  D(G_RIGHT, 1); } break;
                    case LP_UP   : {  D(G_UP   , 1); } break;
                    case LP_DOWN : {  D(G_DOWN , 1); } break;
                    case LP_NIL  : {  } break;
                }
            } break;
        }
    }
    
    if (VPORT_OUT == VP_ACC) D(ALU_WRACCal, 1);
    if (0b01011000 == (opcode&0b11111000) || // ADD A 
        0b01100000 == (opcode&0b11111000) || // SUB A
        0b01101000 == (opcode&0b11111000) || // ADD [X]
        0b01110000 == (opcode&0b11111000) ){ // SUB [X]
        D(ALU_WRACCal, 1);
        D(ALU_RDACCal, 1);
    }
    
    if (0b00111100 == opcode) { // SWP
        D(ALU_WRACCal, 1); 
        D(BAK_SAVEal, 1); 
        D(BAK_RESTOREal, 1); 
    }
    if (0b00111101 == opcode) D(BAK_SAVEal, 1); // SAV
    if (0b00111110 == opcode) D(ALU_WRACCal, 1); // NEG
    
    
    for (int i=0;i<UNUSED_COUNT;i++) { if (Ai(UNUSED, i)) data = 0; }
    
    
#define Db(m_d)  ((u64)1)<<(m_d ## _ ## DBIT)
    data ^= (Db(ALU_WRACCal)  | 
             Db(ALU_RDACCal)  |
             Db(BAK_SAVEal)   |
             Db(BAK_RESTOREal));
    
    return data;
    
}

#include <stdio.h>
#include <assert.h>
int main() {
    auto f = fopen("build/control0_lut.bin", "w");
    for(u32 address = 0; address<1UL<<17;address++) {
        auto b = data_for_address(address);
        fputc(b, f);
    }
    auto r1 = fclose(f);
    assert(!r1);
}


