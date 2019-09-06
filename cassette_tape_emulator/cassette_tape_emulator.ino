


#define PIN_PGRMCLK   3
#define PIN_PGRMDATA  2
#define PIN_CLOCK     4
#define PIN_PGRMEN0   6
#define PIN_PGRMEN1_  5
#define PIN_RESET     7


#define DEBUG_DELAY 
//delay(100); digitalWrite(PIN_CLOCK, HIGH);digitalWrite(PIN_CLOCK, LOW);


void setup () {
    
    Serial.begin(2000000);
    
    
    while (Serial.available() > 0) {
        Serial.read();
    }
    Serial.print("START\n");
    
    
    
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_PGRMCLK, OUTPUT);
    pinMode(PIN_PGRMDATA, OUTPUT);
    pinMode(PIN_PGRMEN0, OUTPUT);
    pinMode(PIN_PGRMEN1_, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    
    // TODO: Pause system clock...
    
    digitalWrite(PIN_PGRMEN0,  HIGH);
    delay(1);
    
    digitalWrite(PIN_PGRMEN1_, LOW);
    delay(1);
    
    digitalWrite(PIN_RESET, LOW);
    delay(1);
    digitalWrite(PIN_RESET, HIGH);
    delay(1);
    
    unsigned char data[16] = {
        0b10000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        
        0b10100000,
        0b01010000,
        0b00101000,
        0b00010100,
        0b10001000,
        0b01000100,
        
        0b11111111,
        0b00000000,
        0b11111111,
        0b00000000,
        
    };
    
    digitalWrite(PIN_PGRMCLK, LOW);
    digitalWrite(PIN_CLOCK  , LOW);
    
    Serial.print("2\n");
    
    
    for (int data_i=0; data_i<16; data_i++) {
        
        for (int bit_i=0; bit_i<8; bit_i++) {
            
            if (data[data_i] & (1UL<<(7-bit_i))) {
                digitalWrite(PIN_PGRMDATA, HIGH);
            } else {
                digitalWrite(PIN_PGRMDATA, LOW);
            }
            digitalWrite(PIN_PGRMCLK, LOW);
            DEBUG_DELAY;
            digitalWrite(PIN_PGRMCLK, HIGH);
            DEBUG_DELAY;
            
        }
        
        
        
        
        digitalWrite(PIN_CLOCK, HIGH);
        DEBUG_DELAY;
        digitalWrite(PIN_CLOCK, LOW);
        DEBUG_DELAY;
        
        Serial.print("Wrote a ");
        Serial.print(data[data_i]);
        Serial.print("\n");
        
        
    }
    
    Serial.print("3\n");
    
    
    digitalWrite(PIN_PGRMEN1_, HIGH);
    DEBUG_DELAY;
    
    digitalWrite(PIN_PGRMEN0,  LOW);
    DEBUG_DELAY;
    
    
    
    Serial.print("4\n");
    
    
}


void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000); 
    digitalWrite(LED_BUILTIN, LOW);   
    delay(1000);              
}
