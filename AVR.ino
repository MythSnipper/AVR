#include <LiquidCrystal.h>
#include <EEPROM.h>


//lcd screen
LiquidCrystal lcd(12, 8, 14, 15, 16, 17);

//wiring
/*
VSS GND
VDD 5V
VO potentiometer middle
RS 12
RW GND
E 8
D4-7 A0-A3
A(LED+) 5V+330Ω resistor
K(LED-) GND

main button 2
left/up button 4
right/down button 3
piezos 11 10 9
*/

const uint8_t upButtonPin = 4;
const uint8_t downButtonPin = 3;
const uint8_t mainButtonPin = 2;
const uint8_t piezo1Pin = 11;
const uint8_t piezo2Pin = 10;
const uint8_t piezo3Pin = 9;
const uint8_t ledPin = 13;


uint32_t currTime;
uint32_t dt;
bool lastMainState = !LOW;
uint32_t lastOnTime;
uint32_t lastOffTime;
bool firstStart = true; //if true make the first space not print

uint8_t charBuf;
char fileBuf[900];

const uint32_t piezo_freq_1 = 440;
const uint32_t piezo_freq_2 = 660;

const uint8_t wpm = 15;

uint32_t dot_len;
uint32_t dash_len;
uint32_t dot_thres; //threshold, if ms below this it is considered to be a dot, otherwise dash
uint32_t inter_char_len;

//lookup table
const uint8_t morse_code_keys[] PROGMEM = {
    0b00000010, //1 len
    0b00000011,
    0b00000100, //2 len
    0b00000101,
    0b00000110,
    0b00000111,
    0b00001000, //3 len
    0b00001001,
    0b00001010,
    0b00001011,
    0b00001100,
    0b00001101,
    0b00001110,
    0b00001111,
    0b00010000, //4 len
    0b00010001,
    0b00010010,
    0b00010011,
    0b00010100,
    0b00010101,
    0b00010110,
    0b00010111,
    0b00011000,
    0b00011001,
    0b00011010,
    0b00011011,
    0b00011100,
    0b00011101,
    0b00011110,
    0b00011111,

    0b00100000, //5.1 len
    0b00100001,
    0b00100010,
    0b00100011,
    0b00100100,
    0b00100101,
    0b00100110,
    0b00100111,
    0b00101000,
    0b00101001,
    0b00101010,
    0b00101011,
    0b00101100,
    0b00101101,
    0b00101110,
    0b00101111,
    0b00110000, //5.2 len
    0b00110001,
    0b00110010,
    0b00110011,
    0b00110100,
    0b00110101,
    0b00110110,
    0b00110111,
    0b00111000,
    0b00111001,
    0b00111010,
    0b00111011,
    0b00111100,
    0b00111101,
    0b00111110,
    0b00111111,

    0b01000000, //6.1 len
    0b01000001,
    0b01000010,
    0b01000011,
    0b01000100,
    0b01000101,
    0b01000110,
    0b01000111,
    0b01001000,
    0b01001001,
    0b01001010,
    0b01001011,
    0b01001100,
    0b01001101,
    0b01001110,
    0b01001111,
    0b01010000,
    0b01010001,
    0b01010010,
    0b01010011,
    0b01010100,
    0b01010101,
    0b01010110,
    0b01010111,
    0b01011000,
    0b01011001,
    0b01011010,
    0b01011011,
    0b01011100,
    0b01011101,
    0b01011110,
    0b01011111,

    0b01100000, //6.2 len
    0b01100001,
    0b01100010,
    0b01100011,
    0b01100100,
    0b01100101,
    0b01100110,
    0b01100111,
    0b01101000,
    0b01101001,
    0b01101010,
    0b01101011,
    0b01101100,
    0b01101101,
    0b01101110,
    0b01101111,
    0b01110000,
    0b01110001,
    0b01110010,
    0b01110011,
    0b01110100,
    0b01110101,
    0b01110110,
    0b01110111,
    0b01111000,
    0b01111001,
    0b01111010,
    0b01111011,
    0b01111100,
    0b01111101,
    0b01111110,
    0b01111111,



};
const char morse_code_chars[] PROGMEM = {
    'e', //1 len
    't',

    'i', //2 len
    'a',
    'n',
    'm',

    's', //3 len
    'u',
    'r',
    'w',
    'd',
    'k',
    'g',
    'o',

    'h', //4 len
    'v',
    'f',
    0xA, //enter ..-- = 0xA LF
    'l',
    ' ', //space .-.-
    'p',
    'j',
    'b',
    'x',
    'c',
    'y',
    'z',
    'q',
    0x6, //exit/confirm ---. = 0x6 ACK
    0x8, //backspace ---- = 0x8 BS

    '5', //5.1 len
    '4',
    0, //0s are null characters
    '3',
    0,
    0,
    0,
    '2',
    '&',
    0,
    '+',
    0,
    0,
    0,
    0,
    '1',

    '6', //5.2 len
    '=',
    '/',
    0,
    0,
    0,
    '(',
    0,
    '7',
    0,
    0,
    0,
    '8',
    0,
    '9',
    '0',

    0, //6.1 len
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '?',
    0,
    0,
    0,
    0,
    0,
    '"',
    0,
    0,
    '.',
    0,
    0,
    0,
    0,
    '@',
    0,
    0,
    0,
    '\'',
    0,

    0, //6.2 len
    '-',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '!',
    0,
    ')',
    0,
    0,
    0,
    0,
    0,
    ',',
    0,
    0,
    0,
    0,
    ':',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};



void morse_code_output_on(uint8_t piezo, uint32_t piezo_freq){
    digitalWrite(ledPin, HIGH);
    tone(piezo, piezo_freq);
}
void morse_code_output_off(uint8_t piezo){
    digitalWrite(ledPin, LOW);
    noTone(piezo);
}
void play_ringtone(){
/*
    morse_code_output_on(piezo1Pin, 311);
    delay(100);
    morse_code_output_off(piezo1Pin);
    delay(5);
    morse_code_output_on(piezo2Pin, 391);
    delay(100);
    morse_code_output_off(piezo2Pin);
    delay(5);
    morse_code_output_on(piezo3Pin, 466);
    delay(100);
    morse_code_output_off(piezo3Pin);
    delay(5);
    morse_code_output_on(piezo1Pin, 622);
    delay(100);
    morse_code_output_off(piezo1Pin);
    delay(5);
*/

    morse_code_output_on(piezo3Pin, 622);
    int a = millis();
    const int e = 3150;
    while(millis() - a < 100){
        digitalWrite(piezo1Pin, HIGH);
        digitalWrite(piezo2Pin, HIGH);
        delayMicroseconds(0.333*e);
        digitalWrite(piezo1Pin, LOW);
        delayMicroseconds(0.167*e);
        digitalWrite(piezo2Pin, LOW);
        delayMicroseconds(0.166*e);
        digitalWrite(piezo1Pin, HIGH);
        delayMicroseconds(0.334*e);

        digitalWrite(piezo1Pin, LOW);
        digitalWrite(piezo2Pin, HIGH);
        delayMicroseconds(0.333*e);
        digitalWrite(piezo1Pin, HIGH);
        delayMicroseconds(0.167*e);
        digitalWrite(piezo2Pin, LOW);
        delayMicroseconds(0.166*e);
        digitalWrite(piezo1Pin, LOW);
        delayMicroseconds(0.334*e);
        
    }
    morse_code_output_off(piezo3Pin);
    delay(50);
    morse_code_output_on(piezo3Pin, 622);
    a = millis();
    while(millis() - a < 400){
        digitalWrite(piezo1Pin, HIGH);
        digitalWrite(piezo2Pin, HIGH);
        delayMicroseconds(0.333*e);
        digitalWrite(piezo1Pin, LOW);
        delayMicroseconds(0.167*e);
        digitalWrite(piezo2Pin, LOW);
        delayMicroseconds(0.166*e);
        digitalWrite(piezo1Pin, HIGH);
        delayMicroseconds(0.334*e);

        digitalWrite(piezo1Pin, LOW);
        digitalWrite(piezo2Pin, HIGH);
        delayMicroseconds(0.333*e);
        digitalWrite(piezo1Pin, HIGH);
        delayMicroseconds(0.167*e);
        digitalWrite(piezo2Pin, LOW);
        delayMicroseconds(0.166*e);
        digitalWrite(piezo1Pin, LOW);
        delayMicroseconds(0.334*e);
        
    }
    morse_code_output_off(piezo3Pin);
}

void setup(){
    //morse code input and output
    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    pinMode(mainButtonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    pinMode(piezo1Pin, OUTPUT);
    pinMode(piezo2Pin, OUTPUT);
    pinMode(piezo3Pin, OUTPUT);

    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.blink();
    lcd.print("Hello World!");

    //calculate some things
    dot_len = 1200 / wpm;
    dash_len = 3 * dot_len;
    dot_thres = 1.8 * dot_len;
    inter_char_len = 4 * dot_len;

    charBuf = 1;
    firstStart = true;
    //zero out the buffer(s)
    for(uint32_t i=0;i<sizeof(fileBuf)/sizeof(char);i++){
        fileBuf[i] = '\0';
    }

    Serial.println("Hello World!");
    Serial.print(F("Dot length: "));Serial.println(dot_len);
    Serial.print(F("Dash length: "));Serial.println(dash_len);
    Serial.print(F("Dot threshold: "));Serial.println(dot_thres);
    Serial.print(F("inter char length: "));Serial.println(inter_char_len);

    lastOnTime = millis();
    lastOffTime = millis();
    play_ringtone();
}

void loop(){


    bool mainState = !digitalRead(mainButtonPin);
    bool upState = !digitalRead(upButtonPin);
    bool downState = !digitalRead(downButtonPin);
    currTime = millis();

    if(mainState){
        morse_code_output_on(piezo1Pin, piezo_freq_1);
    }
    else{
        morse_code_output_off(piezo1Pin);
    }

    if(mainState != lastMainState){
        if(lastMainState){ //triggered falling edge
            dt = currTime - lastOnTime; //time the button is on
            if(dt > 3){
                if(dt < dot_thres){
                    Serial.print("."); //add 0 to the char buffer
                    //only add if the buffer is not full, otherwise reset buffer
                    if(charBuf & 0b10000000){ //full with 7 chars
                        charBuf = 1; //reset
                        Serial.println("RESET");
                    }
                    else{
                        charBuf <<= 1;
                        charBuf |= 0;
                    }
                }
                else{
                    Serial.print("-");
                    //only add if the buffer is not full, otherwise reset buffer
                    if(charBuf & 0b10000000){ //full with 7 chars
                        charBuf = 1; //reset
                        Serial.println("RESET");
                    }
                    else{
                        charBuf <<= 1;
                        charBuf |= 0;
                    }
                }
            }
            //start timing the non button press
            lastOffTime = millis();
        }
        else{ //triggered rising edge
            //start timing the button press
            lastOnTime = currTime;
        }
    }

    if(upState){

        Serial.print(".");
        morse_code_output_on(piezo1Pin, piezo_freq_1);
        delay(dot_len);
        morse_code_output_off(piezo1Pin);
        delay(dot_len);

        //counts as falling edge
        //start timing the non button press
        lastOffTime = millis();
    }
    if(downState){

        Serial.print("-");
        morse_code_output_on(piezo2Pin, piezo_freq_2);
        delay(dash_len);


        //counts as falling edge
        //start timing the non button press
        lastOffTime = millis();


        morse_code_output_off(piezo2Pin);
        delay(dash_len);

    }


    //print space if needed
    if(!upState && !downState && !mainState){
        if(!firstStart){
            dt = currTime - lastOffTime; //how much time the button is off
            if(dt > inter_char_len && dt < inter_char_len * 2){
                Serial.print(" ");

                charBuf = 1; //reset
                //get character from lookup table
                uint32_t index;
                for(int i=0;i<sizeof(morse_code_keys);i++){
                    if(pgm_read_byte(&morse_code_keys[i]) == charBuf){
                        char decoded = pgm_read_byte(&morse_code_chars[i]);
                        if(decoded >= ' '){
                            Serial.println(decoded);
                        }
                        else if(decoded == 0xA){
                            Serial.println("LF");
                        }
                        else if(decoded == 0x6){
                            Serial.println("ACK");
                        }
                        else if(decoded == 0x8){
                            Serial.println("BS");
                        }
                        else if(decoded == 0){
                            Serial.println("NULL");
                        }
                        else{
                            Serial.println("EGG");
                        }
                    }
                }

                lastOffTime = -inter_char_len * 3;
            }
        }
    }
    else{
        //if something is pressed remove first start
        firstStart = false;
    }



    lastMainState = mainState;
}



