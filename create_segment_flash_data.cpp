

typedef unsigned char u8;
typedef short         s16;
typedef unsigned int  u32;

static const int value_bits_to_address_bits[11] = {
    0, 1, 2, 3, 4, 5, 6, 7, 12, 15, 16
};

static const int digit_to_address_bits[4] = {
    11, 9, 8, 13
};


#define SEGMENT_DATA_PIN(m_s, m_b)  static const unsigned char  SEG_ ## m_s = (1<<m_b);
SEGMENT_DATA_PIN(A, 0);
SEGMENT_DATA_PIN(B, 2);
SEGMENT_DATA_PIN(C, 3);
SEGMENT_DATA_PIN(D, 5);
SEGMENT_DATA_PIN(E, 6);
SEGMENT_DATA_PIN(F, 4);
SEGMENT_DATA_PIN(G, 1);
SEGMENT_DATA_PIN(P, 7);

unsigned char segments_for_digit_value[10] = {
    [0] = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,
    [1] = SEG_B|SEG_C,
    [2] = SEG_A|SEG_B|SEG_G|SEG_D|SEG_E,
    [3] = SEG_B|SEG_C|SEG_A|SEG_G|SEG_D,
    [4] = SEG_F|SEG_G|SEG_B|SEG_C,
    [5] = SEG_A|SEG_F|SEG_G|SEG_C|SEG_D,
    [6] = SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
    [7] = SEG_A|SEG_B|SEG_C,
    [8] = SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
    [9] = SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G,
};

u8 _data_for_address(u32 address) {
    
    // Pull down A10 and A14....
    if ((address & 1<<10) || (address & 1<<14) ) {
        return 0;
    }
    
    u8  digit_bits = 0;
    for (int i=0;i<4;i++) {
        u32 b = address & (1UL<<digit_to_address_bits[i]);
        if (b) {
            digit_bits |= 1<<i;
        }
    }
    
    int value = 0;
    for (int i=0;i<10;i++) {
        u32 b = address & (1UL<<value_bits_to_address_bits[i]);
        if(b) {
            value += 1<<i;
        }
    }
    if (address & (1UL<<value_bits_to_address_bits[10])) {
        value -= 1024;
    }
    
    if (value > 999 || value < -999) {
        return SEG_G;
    }
    
    s16 abs_value = value;
    if (value <0) abs_value = -value;
    
    switch(digit_bits) { 
        default: return 0;
        case 0b1000: { 
            if (value < 0) {
                return SEG_G;
            } else {
                return 0;
            }
        } break;
        case 0b0100: {
            if (abs_value <  100) {
                return 0;
            } else {
                return segments_for_digit_value[abs_value/100];
            }
        } break;
        case 0b0010: {
            if (abs_value <  10) {
                return 0;
            } else {
                abs_value = abs_value % 100;
                return segments_for_digit_value[abs_value/10];
            }
        } break;
        case 0b0001: {
            return segments_for_digit_value[abs_value%10];
        } break;
    }
    
}


unsigned char data_for_address(unsigned long address) {
    return ~_data_for_address(address);
}

#include <stdio.h>
#include <assert.h>
int main() {
    
    auto f = fopen("build/segment_flash_data.bin", "w");
    
    for(u32 address = 0; address<1UL<<17;address++) {
        auto b = data_for_address(address);
        fputc(b, f);
    }
    
    auto r1 = fclose(f);
    assert(!r1);
    
}

