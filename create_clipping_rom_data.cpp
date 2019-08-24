
typedef unsigned char u8;
typedef short         s16;
typedef unsigned int  u32;

static const int value_in_address_bits[11] = {
    16, 15, 12, 7, 6, 5, 4, 3, 2, 1, 0
};
static const int rom_select_address_bit = 10; // low for lower bits, high for higher bits

static const int sign_of_a_address_bit = 11;
static const int sign_of_b_address_bit =  9;


static const int low_value_out_data_bits[6] = {
    7, 6, 5, 4, 3, 2 
};

static const int high_value_out_data_bits[5] = {
    7, 6, 5, 4, 3, 
};


#define A(m_a) (address & 1UL<<(m_a))

u8 data_for_address(u32 address) {
    
    // Pull down A8, and A13, A14
    if (A(8)||A(13)||A(14)) {
        return 0;
    }
    
    int value = 0;
    
    if (A(sign_of_b_address_bit)     == A(sign_of_a_address_bit) &&
        A(value_in_address_bits[10]) != A(sign_of_a_address_bit)
        ) {
        if (A(sign_of_a_address_bit)) {
            value = -999;
        } else {
            value =  999;
        }
        
    } else {
        for (int i=0;i<10;i++) {
            u32 b = address & (1UL<<value_in_address_bits[i]);
            if(b) {
                value += 1<<i;
            }
        }
        if (address & (1UL<<value_in_address_bits[10])) {
            value -= 1024;
        }
        if (value > 999) {
            value = 999;
        } else if (value < -999) {
            value = -999;
        }
    }
    
    
    unsigned char out = 0;
    if (!A(rom_select_address_bit)) { // lower bits
        for (int i=0; i<6; i++) {
            if (value & 1<<i) {
                out |= (1<<low_value_out_data_bits[i]);
            }
        }
    } else { // higher bits
        for (int i=0; i<5; i++) {
            if (value & 1<<(i+6)) {
                out |= (1<<high_value_out_data_bits[i]);
            }
        }
    }
    
    return out;
    
}

#include <stdio.h>
#include <assert.h>
int main() {
    auto f = fopen("build/clipping_flash_data.bin", "w");
    for(u32 address = 0; address<1UL<<17;address++) {
        auto b = data_for_address(address);
        fputc(b, f);
    }
    auto r1 = fclose(f);
    assert(!r1);
}
