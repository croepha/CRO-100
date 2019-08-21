

#define FLASH_PIN0 21

#define FLASH_CE (FLASH_PIN0+22)
#define FLASH_OE (FLASH_PIN0+24)
#define FLASH_WE (FLASH_PIN0+31)

static const int ADDRESS_PIN[] = {
    [ 0] = FLASH_PIN0+12,
    [ 1] = FLASH_PIN0+11,
    [ 2] = FLASH_PIN0+10,
    [ 3] = FLASH_PIN0+ 9,
    [ 4] = FLASH_PIN0+ 8,
    [ 5] = FLASH_PIN0+ 7,
    [ 6] = FLASH_PIN0+ 6,
    [ 7] = FLASH_PIN0+ 5,
    [ 8] = FLASH_PIN0+27,
    [ 9] = FLASH_PIN0+26,
    [10] = FLASH_PIN0+23,
    [11] = FLASH_PIN0+25,
    [12] = FLASH_PIN0+ 4,
    [13] = FLASH_PIN0+28,
    [14] = FLASH_PIN0+29,
    [15] = FLASH_PIN0+ 3,
    [16] = FLASH_PIN0+ 2,
};

static const int DATA_PIN[] = {
    [ 0] = FLASH_PIN0+13,
    [ 1] = FLASH_PIN0+14,
    [ 2] = FLASH_PIN0+15,
    [ 3] = FLASH_PIN0+17,
    [ 4] = FLASH_PIN0+18,
    [ 5] = FLASH_PIN0+19,
    [ 6] = FLASH_PIN0+20,
    [ 7] = FLASH_PIN0+21,
};

void set_address_pins(unsigned  long address) {
    for(unsigned long long i=0;i<17;i++) {
        if (address & (1<<i)) {
            digitalWrite(ADDRESS_PIN[i], HIGH);
        } else {
            digitalWrite(ADDRESS_PIN[i], LOW);      
        }
    }  
}

unsigned char read_data_pins() {
    unsigned char ret = 0;
    for(int i=0;i<8; i++) {
        if (digitalRead(DATA_PIN[i]) == HIGH) {
            ret = ret ^ (1<<i);
        }
    }
    return ret;  
}

void set_data_pins(unsigned char data) {
    for(int i=0;i<8;i++) {
        if (data & (1<<i)) {
            digitalWrite(DATA_PIN[i], HIGH);
        } else {
            digitalWrite(DATA_PIN[i], LOW);      
        }
    }  
}


void send_software_command_sequence(unsigned long* address, unsigned char* data, int count) {
    
    digitalWrite(FLASH_CE, LOW);
    digitalWrite(FLASH_OE, HIGH);
    
    //delay(1);
    for(int i=0;i<8;i++) {
        pinMode(DATA_PIN[i], OUTPUT);
    }
    
    //delay(1);
    
    for (int i=0;i<count;i++) {
        digitalWrite(FLASH_WE, HIGH);
        set_address_pins(address[i]);
        digitalWrite(FLASH_WE, LOW);
        set_data_pins(data[i]);
    }
    
    digitalWrite(FLASH_WE, HIGH);
    
    for(int i=0;i<8;i++) {
        pinMode(DATA_PIN[i], INPUT);
    }
    //delay(1);
    
    digitalWrite(FLASH_OE, LOW); 
    //delay(1);
    
    digitalWrite(FLASH_CE, HIGH);
    
}


unsigned char flash_read_byte(unsigned long address) {
    digitalWrite(FLASH_CE, LOW);
    set_address_pins(address); //delay(1);
    return read_data_pins();
}
void flash_enter_software_id() {
    unsigned long  address[] = {0x5555, 0x2AAA, 0x5555};
    unsigned char data[] = {0xAA, 0x55, 0x90};
    send_software_command_sequence(address, data, 3);
}
void flash_exit_software_id() {
    unsigned long address[] = {0x5555, 0x2AAA, 0x5555};
    unsigned char data[] = {0xAA, 0x55, 0xF0};
    send_software_command_sequence(address, data, 3);
}


void flash_chip_erase() {
    unsigned long address[] = {0x5555, 0x2AAA, 0x5555, 0x5555, 0x2AAA, 0x5555};
    unsigned char data[] = {0xAA, 0x55, 0x80, 0xAA, 0x55, 0x10};
    
    send_software_command_sequence(address, data, 6);
    delay(200);
}



void flash_write_byte(unsigned int addr, unsigned char _byte) {
    unsigned long address[] = {0x5555, 0x2AAA, 0x5555, addr};
    unsigned char data[] = {0xAA, 0x55, 0xA0, _byte};
    send_software_command_sequence(address, data, 4);
    //delay(1);
}




void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    
    for (int i=0; i<54; i++) {
        pinMode(i, INPUT);
    }
    
    pinMode(FLASH_CE, OUTPUT);
    pinMode(FLASH_OE, OUTPUT);
    pinMode(FLASH_WE, OUTPUT);
    for(int i=0;i<17;i++) {
        pinMode(ADDRESS_PIN[i], OUTPUT);
    }
    
    pinMode(LED_BUILTIN, OUTPUT);
    
    
    Serial.begin(2000000);
    
    Serial.print("TEST\n");
    
    //  for (int i=0; i<32; i++) {
    //    pinMode(i, INPUT);
    //  }
    
    
    
    digitalWrite(FLASH_CE, HIGH);
    digitalWrite(FLASH_OE, LOW);
    digitalWrite(FLASH_WE, HIGH);
    delay(1);
    
    
    
    
    flash_enter_software_id();
    Serial.print("Data: 0x");
    Serial.print(flash_read_byte(0x0000), HEX);
    Serial.print("\n");
    
    Serial.print("Data: 0x");
    Serial.print(flash_read_byte(0x0001), HEX);
    Serial.print("\n");
    flash_exit_software_id();
    
    
    
    Serial.print("Erasing\n");
    flash_chip_erase();
    Serial.print("Erasing done\n");
    
    
    Serial.print("Write A short:\n");
    for (unsigned long i=0; i < 1UL<<10; i++) {
        unsigned char b = 29+i*13;
        flash_write_byte(i, b);
        
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    Serial.print("\n");
    
    Serial.print("Read A short:\n");
    for (unsigned long i=0; i < 1UL<<10; i++) {
        unsigned char b = 29+i*13;
        auto _ = flash_read_byte(i);
        if (_ != b) {
            Serial.print("bad write??");
            Serial.print(i, HEX);
            Serial.print("\n");
        }
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    Serial.print("\n");
    
    Serial.print("Erasing\n");
    flash_chip_erase();
    Serial.print("Erasing done\n");
    
    
    Serial.print("Write B short:\n");
    for (unsigned long i=0; i < 1UL<<10; i++) {
        unsigned char b = 31+i*89;
        flash_write_byte(i, b);
        
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    
    Serial.print("\n");
    
    Serial.print("Read B short:\n");
    for (unsigned long i=0; i < 1UL<<10; i++) {
        unsigned char b = 31+i*89;
        auto _ = flash_read_byte(i);
        if (_ != b) {
            Serial.print("bad write??");
            Serial.print(i, HEX);
            Serial.print("\n");
        }
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    
    Serial.print("\n");
    
    Serial.print("Write: A long\n");
    for (unsigned long i=1UL<<10; i < 1UL<<17; i++) {
        unsigned char b = 29+i*13;
        flash_write_byte(i, b);
        
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    
    Serial.print("\n");
    
    Serial.print("Read: A long\n");
    for (unsigned long i=1UL<<10; i < 1UL<<17; i++) {
        unsigned char b = 29+i*13;
        auto _ = flash_read_byte(i);
        if (_ != b) {
            Serial.print("bad write??");
            Serial.print(i, HEX);
            Serial.print("\n");
        }
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    
    Serial.print("\n");
    
    Serial.print("Erasing\n");
    flash_chip_erase();
    Serial.print("Erasing done\n");
    
    
    Serial.print("Write B long:\n");
    for (unsigned long i=1UL<<10; i < 1UL<<17; i++) {
        unsigned char b = 31+i*89;
        flash_write_byte(i, b);
        
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    Serial.print("\n");
    
    Serial.print("Read B long:\n");
    for (unsigned long i=1UL<<10; i < 1UL<<17; i++) {
        unsigned char b = 31+i*89;
        auto _ = flash_read_byte(i);
        if (_ != b) {
            Serial.print("bad write??");
            Serial.print(i, HEX);
            Serial.print("\n");
        }
        if (!(i % 0x1000)) {
            Serial.print(i, HEX);
        }
    }
    Serial.print("\n");
    Serial.print("Done\n");
    
}

//     digitalWrite(i, HIGH);

// the loop function runs over and over again forever
void loop() {
    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000); 
    digitalWrite(LED_BUILTIN, LOW);   
    delay(1000);              
}