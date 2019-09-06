

typedef unsigned char u8;
typedef short         s16;
typedef unsigned int  u32;
typedef unsigned long long u64;


#define ABIT(m_n) static const int m_n ## _ ## ABIT
#define DBIT(m_n) static const int m_n ## _ ## DBIT

ABIT(OPCODE)[8] = { 0,1,2,3,4,5,6,7 };
ABIT(LAST_Q)[3] = { 8, 9, 11};
ABIT(WRITE_READY_Q) = 14; 


ABIT(READY_LEFT)  = 16; 
ABIT(READY_RIGHT) = 15; 
ABIT(READY_UP)    = 12; 
ABIT(READY_DOWN)  = 13; 

#define UNUSED_COUNT 1
ABIT(UNUSED)[UNUSED_COUNT] = { 10 };



DBIT(H_Q)[3]   =  { 0, 1, 2};
DBIT(IDLEal)        = 3;
DBIT(BUF_OUTal)     = 4;
DBIT(BUF_SETal)     = 5;
DBIT(LAST_SETal)    = 6;
DBIT(WRITE_READY_D) = 7;



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
    
    enum IO_MODE { NONE, WRITE, READ } io_mode = NONE;
    bool buf_set = 0;
    
    if (VPORT_OUT != VP_NIL ) {
        if (!A(WRITE_READY_Q)) {
            buf_set = 1;
        } else {
            D(BUF_OUTal, 1);
            io_mode = WRITE;
        }
    }
    if (VPORT_IN != VP_NIL ) {
        if (!A(WRITE_READY_Q)) {
            io_mode = READ;
        } 
    }
    
    char active_vport = VP_NIL;
    if      (io_mode == WRITE) active_vport = VPORT_OUT;
    else if (io_mode == READ ) active_vport = VPORT_IN;
    
    unsigned char h = VP_NIL;
    
    bool port_block = 0;
    
    switch (active_vport) {
#define VPORT_GEN(m_d) case VP_ ## m_d: { if (A(READY_ ## m_d)) { h = LP_ ## m_d; } else { port_block=1; }  }; break
        VPORT_GEN(LEFT); VPORT_GEN(RIGHT); VPORT_GEN(UP); VPORT_GEN(DOWN);
#undef  VPORT_GEN 
        
        case VP_ANY: {
#define _M(m_p)  if (A(READY_ ## m_p)) { h = VP_ ## m_p;  D(LAST_SETal, 1); } else 
            if (io_mode == READ) { _M(UP) _M(LEFT) _M(RIGHT) _M(DOWN) { port_block=1; } }
            else                 { _M(LEFT) _M(RIGHT) _M(UP) _M(DOWN) { port_block=1; } }
#undef _M
        } break;
        
        case VP_LAST: {
            switch (last) {
                case LP_NIL: { if (io_mode==WRITE) {port_block=1;} } break; 
#define _M(m_p) case LP_ ## m_p: { if (A(READY_ ## m_p)) {h=LP_ ## m_p; } else { port_block=1; } } break
                _M(LEFT); _M(RIGHT); _M(UP); _M(DOWN);
#undef  _M
            }
        }
    };
    
    
    
    if (buf_set || port_block) D(IDLEal, 1);
    
    
    
    for (int i=0;i<UNUSED_COUNT;i++) { if (Ai(UNUSED, i)) data = 0; }
    
    
#define Db(m_d)  ((u64)1)<<(m_d ## _ ## DBIT)
    data ^= (Db(IDLEal)  | 
             Db(BUF_OUTal)  |
             Db(BUF_SETal)   |
             Db(LAST_SETal));
    
    return data;
    
}

#include <stdio.h>
#include <assert.h>
int main() {
    auto f = fopen("build/control1_lut.bin", "w");
    for(u32 address = 0; address<1UL<<17;address++) {
        auto b = data_for_address(address);
        fputc(b, f);
    }
    auto r1 = fclose(f);
    assert(!r1);
}




