

typedef unsigned char u8;
typedef short         s16;
typedef unsigned int  u32;
typedef unsigned long long u64;


#define ABIT(m_n) static const int m_n ## _ ## ABIT
#define DBIT(m_n) static const int m_n ## _ ## DBIT


ABIT(OPCODE)[8]     = { 0,1,2,3,4,5,6,7 };
ABIT(WRITE_READY_Q) = 14;

ABIT(PC_SUM_SIGN)   = 15;
ABIT(PC_SGTE)       = 16;
ABIT(PC_CEQE)       = 12;

ABIT(BUS_SIGN)      = 13; 
ABIT(ACC_EQZ)       =  8;
ABIT(ACC_SIGN)      =  9;

#define UNUSED_COUNT 2
ABIT(UNUSED)[UNUSED_COUNT] = { 10, 11 };



DBIT(PC_LDal)    = 0;
DBIT(PC_LDENDal) = 1;
DBIT(PC_LDBUSal) = 2;
DBIT(PC_LDSUMal) = 7;

DBIT(G_ANY)     = 6;
DBIT(ALU_SUB)   = 5;
DBIT(MEM_OUTal) = 4;
DBIT(ACC_OUTal) = 3;


#define A(m_a)       (!!(address & 1UL<<(m_a ## _ ## ABIT)))
#define Ai(m_a, m_i) (!!(address & 1UL<<(m_a ## _ ## ABIT[m_i])))
#define D(m_d, m_v)  (data |= ((u64)(!!(m_v)))<<(m_d ## _ ## DBIT))

u8 data_for_address(u32 address) {
    
    u8 data = 0;
    
    u8 opcode = 0;
    for (int i=0;i<8;i++) {
        opcode |= (Ai(OPCODE, i))<<i;
    }
    
    
    if (0b01010000 == (opcode & 0b11111000) || // JRO A
        0b01011000 == (opcode & 0b11111000) || // ADD A
        0b01100000 == (opcode & 0b11111000) || // SUB A
        0b11000000 == (opcode & 0b11000000) ){ // MOV A B
        D(ACC_OUTal, 1);
    }
    
    
    if (0b00111110 == opcode                || // NEG
        0b01100000 == (opcode & 0b11111000) || // SUB A
        0b01110000 == (opcode & 0b11111000) ){ // SUB [X]
        D(ALU_SUB, 1);
    }
    
    if (0b01011011 == opcode                || // JMP M
        0b01011100 == opcode                || // JEZ M
        0b01011101 == opcode                || // JNZ M
        0b01011110 == opcode                || // JGZ M
        0b01001111 == opcode                || // JLZ M
        0b01101000 == (opcode & 0b11111000) || // ADD [X]
        0b01110000 == (opcode & 0b11111000) || // SUB [X] 
        0b01111000 == (opcode & 0b11111000) || // JRO [X]  
        0b10000000 == (opcode & 0b11000000) ){ // MOV [X] B 
        D(MEM_OUTal, 1);
    }
    
    bool ACTUAL_JMP = 0;
    bool JMP_REL = 0;
    
    if (0b01011011 == opcode)                ACTUAL_JMP=1; // JMP M 
    if (0b01011100 == opcode &&  A(ACC_EQZ)) ACTUAL_JMP=1; // JEZ M 
    if (0b01011101 == opcode && !A(ACC_EQZ)) ACTUAL_JMP=1; // JNZ M
    if (0b01011110 == opcode && !A(ACC_EQZ) && !A(ACC_SIGN) ) ACTUAL_JMP=1; // JGZ M 
    if (0b01001111 == opcode && !A(ACC_EQZ) &&  A(ACC_SIGN) ) ACTUAL_JMP=1; // JLZ M 
    if (0b01010000 == (opcode & 0b11111000)) { ACTUAL_JMP=1; JMP_REL=1;}; //  JRO IN
    if (0b11110000 == (opcode & 0b11111000)) { ACTUAL_JMP=1; JMP_REL=1;}; //  JRO [X]  
    
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
    
    
    // get JMP, JPM_REL from opcode
    // ACTUAL_JMP = JMP && ( test EQZ or NEG )
    
    if (ACTUAL_JMP) {
        if(JMP_REL) {
            if (A(BUS_SIGN) && A(PC_SUM_SIGN)) {
                // Negative case
                D(PC_LDal, 1); // jump to zero
            } else if (A(PC_SUM_SIGN)) {
                D(PC_LDal, 1); 
                D(PC_LDENDal, 1); // jump to end
            } else {
                D(PC_LDal, 1); 
                D(PC_LDSUMal, 1); // jump to BUS + PC
            }
        } else {
            D(PC_LDal, 1);
            D(PC_LDBUSal, 1);
        }
    } else {
        if (A(PC_CEQE)) {
            D(PC_LDal, 1); // jump to zero
        }
    }
    
    if (A(WRITE_READY_Q)) {
        if (VPORT_OUT == VP_ANY) {
            D(G_ANY, 1);
        }
    }
    
    if (VPORT_OUT == VP_ACC) {
        D(ACC_OUTal, 1);
    }
    if (0b00111110 == opcode) { // NEG
        D(ACC_OUTal, 1);
    }
    
    
    for (int i=0;i<UNUSED_COUNT;i++) { if (Ai(UNUSED, i)) data = 0; }
    
#define Db(m_d)  ((u64)1)<<(m_d ## _ ## DBIT)
    data ^= (Db(PC_LDal) |
             Db(PC_LDENDal) |
             Db(PC_LDBUSal) |
             Db(PC_LDSUMal) |
             Db(MEM_OUTal) |
             Db(ACC_OUTal) );
    
    return data;
    
}

#include <stdio.h>
#include <assert.h>
int main() {
    auto f = fopen("build/control2_lut.bin", "w");
    for(u32 address = 0; address<1UL<<17;address++) {
        auto b = data_for_address(address);
        fputc(b, f);
    }
    auto r1 = fclose(f);
    assert(!r1);
}




