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
left/up button 7
right/down button 4
piezos 11 10 9

RGBLED: red 6 green 3 blue 5
*/

typedef struct{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;
typedef struct{
    uint8_t x;
    uint8_t y;
} Vector2;

//button configurations
const uint8_t upButtonPin = 7;
const uint8_t downButtonPin = 4;
const uint8_t mainButtonPin = 2;

//piezo configurations
const uint8_t piezo1Pin = 11;
const uint8_t piezo2Pin = 10;
const uint8_t piezo3Pin = 9;

const uint16_t piezoFreqs[3] = {440, 660, 466};

//LED configurations
RGB ledColor{255, 255, 0};
RGB ledColor2{0, 255, 255};
RGB ledColor3{255, 0, 0};
const RGB ledPins{6, 3, 5};

//runtime use
uint32_t currTime;
uint32_t dt;
bool lastMainState = !LOW;
uint32_t lastOnTime;
uint32_t lastOffTime;
uint32_t lastDisplayRefresh;
bool firstStart = true; //if true make the first space not print

uint32_t dot_len;
uint32_t dash_len;
uint32_t dot_thres; //threshold, if ms below this it is considered to be a dot, otherwise dash
uint32_t inter_char_len;

uint8_t charBuf;
char fileBuf[901] = {0}; //900 max for a file
char displayBuf[2][17] = {{' '}}; //screen size + null terminator
Vector2 displayPos = {0, 0};

//modifiable settings
const uint8_t wpm = 20;

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


void morse_code_output_on(uint8_t piezo, uint32_t piezo_freq, RGB color){
    analogWrite(ledPins.red, 255-color.red);
    analogWrite(ledPins.green, 255-color.green);
    analogWrite(ledPins.blue, 255-color.blue);
    tone(piezo, piezo_freq);
}
void morse_code_output_off(uint8_t piezo){
    analogWrite(ledPins.red, 255);
    analogWrite(ledPins.green, 255);
    analogWrite(ledPins.blue, 255);
    noTone(piezo);
}
void morse_code_output_warning(uint8_t piezo, uint32_t piezo_freq, RGB color){
    morse_code_output_on(piezo, piezo_freq, color);
    delay(50);
    morse_code_output_off(piezo);
    delay(10);
    morse_code_output_on(piezo, piezo_freq, color);
    delay(350);
    morse_code_output_off(piezo);
    delay(10);
}
void play_ringtone(){
/*
    morse_code_output_on(piezo1Pin, 311, ledColor);
    delay(100);
    morse_code_output_off(piezo1Pin);
    delay(5);
    morse_code_output_on(piezo2Pin, 391, ledColor);
    delay(100);
    morse_code_output_off(piezo2Pin);
    delay(5);
    morse_code_output_on(piezo3Pin, 466, ledColor);
    delay(100);
    morse_code_output_off(piezo3Pin);
    delay(5);
    morse_code_output_on(piezo1Pin, 622, ledColor);
    delay(100);
    morse_code_output_off(piezo1Pin);
    delay(5);
*/

    morse_code_output_on(piezo3Pin, 622, ledColor);
    int a = millis();
    const int e = 3150;
    while(millis() - a < 60){
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
    morse_code_output_on(piezo3Pin, 622, ledColor2);
    a = millis();
    while(millis() - a < 200){
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

void LCDRefresh(){
    for(int y = 0; y < 2; y++){
        lcd.setCursor(0, y);
        lcd.print(displayBuf[y]);
    }
    lcd.setCursor(displayPos.x, displayPos.y);

}

/*
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
*/


void LCDPutChar(char niko){
    char* target = &displayBuf[displayPos.y][displayPos.x];
    bool advance = true;
    bool advanceForwards = true;
    bool bksp = false;
    bool line_feed = false;
    //print as displaypos
    if(niko >= ' '){
        *target = niko;
    }
    else if(niko == 0xA){ //LF ..--
        line_feed = true;
    }
    else if(niko == 0x6){ //ACK ---.
        *target = 'A';
    }
    else if(niko == 0x8){ //BS .-.-
        //backspace
        bksp = true;
        advanceForwards = false;
    }
    else{
        *target = 'N';
    }


    //increment displaypos
    if(!advance){
        return;
    }
    if(advanceForwards){
        if(displayPos.x == 15 || line_feed){ //if reached the end of a line
            displayPos.x = 0;
            //advance y, scroll if needed
            if(displayPos.y == 0){
                displayPos.y = 1;
            }
            else if(displayPos.y == 1){
                //scroll screen
                bool line2_empty = true; //if line2 stays empty, move cursor to 0, 0 for fresh writing
                for(int i=0;i<sizeof(displayBuf[0])/sizeof(displayBuf[0][0])-1;i++){
                    if(displayBuf[1][i] != ' '){
                        line2_empty = false;
                    }
                    displayBuf[0][i] = displayBuf[1][i];
                    displayBuf[1][i] = ' ';
                }
                if(line2_empty){
                    displayPos.y = 0;
                }
            }
        }
        else{
            displayPos.x++;
        }
    }
    else{ //go back
        if(displayPos.x == 0){ //if reached the start of a line
            //decrement y only if y is on the second line
            if(displayPos.y == 1){
                //go to end of first line
                displayPos.y = 0;
                displayPos.x = 15;

            }
        }
        else{
            displayPos.x--;
        }
    }

    if(bksp){
        displayBuf[displayPos.y][displayPos.x] = ' '; //set to space
    }
}

void setup(){
    //morse code input and output
    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    pinMode(mainButtonPin, INPUT_PULLUP);
    pinMode(ledPins.red, OUTPUT);
    pinMode(ledPins.green, OUTPUT);
    pinMode(ledPins.blue, OUTPUT);
    pinMode(piezo1Pin, OUTPUT);
    pinMode(piezo2Pin, OUTPUT);
    pinMode(piezo3Pin, OUTPUT);

    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.blink();

    //calculate some things
    dot_len = 1200 / wpm;
    dash_len = 3 * dot_len;
    dot_thres = 1.8 * dot_len;
    inter_char_len = 4 * dot_len;

    charBuf = 1;
    firstStart = true;

    //just put the null terminators in the bag lil bro
    for(int i=0;i<2;i++){
        displayBuf[i][16] = 0;
    }

    Serial.println("Nuck arduinOS");
    Serial.print("WPM: ");Serial.println(wpm);


    play_ringtone();

    lastOnTime = millis();
    lastOffTime = millis();
    lastDisplayRefresh = millis();
}

void loop(){

    bool mainState = !digitalRead(mainButtonPin);
    bool upState = !digitalRead(upButtonPin);
    bool downState = !digitalRead(downButtonPin);
    currTime = millis();

    if(mainState != lastMainState){
        if(lastMainState){ //triggered falling edge
            //start timing the non button press
            lastOffTime = millis();

            morse_code_output_off(piezo1Pin);


            dt = currTime - lastOnTime; //time the button is on
            if(dt > 3){
                if(dt < dot_thres){
                    //only add if the buffer is not full, otherwise reset buffer
                    if(charBuf & 0b1000000){ //full
                        charBuf = 1; //reset
                        Serial.println("RST");
                        firstStart = true;
                        morse_code_output_warning(piezo1Pin, piezoFreqs[2], ledColor3);
                    }
                    else{
                        Serial.print("."); //add 0 to the char buffer
                        charBuf <<= 1;
                        charBuf |= 0;
                    }
                }
                else{
                    //only add if the buffer is not full, otherwise reset buffer
                    if(charBuf & 0b1000000){ //full
                        charBuf = 1; //reset
                        Serial.println("RST");
                        firstStart = true;
                        morse_code_output_warning(piezo1Pin, piezoFreqs[2], ledColor3);
                    }
                    else{
                        Serial.print("-"); //add 1 to the char buffer
                        charBuf <<= 1;
                        charBuf |= 1;
                    }
                }
            }
        }
        else{ //triggered rising edge
            //start timing the button press
            lastOnTime = currTime;


            morse_code_output_on(piezo1Pin, piezoFreqs[0], ledColor);
        }
    }

    if(upState){

        //only add if the buffer is not full, otherwise reset buffer
        if(charBuf & 0b1000000){ //full
            charBuf = 1; //reset
            Serial.println("RST");
            firstStart = true;
            morse_code_output_warning(piezo1Pin, piezoFreqs[2], ledColor3);
        }
        else{
            Serial.print(".");
            charBuf <<= 1;
            charBuf |= 0;
        }


        morse_code_output_on(piezo2Pin, piezoFreqs[0], ledColor2);
        delay(dot_len);

        //counts as falling edge
        //start timing the non button press
        lastOffTime = millis();

        morse_code_output_off(piezo2Pin);
        delay(dot_len);
        
    }
    if(downState){

        //only add if the buffer is not full, otherwise reset buffer
        if(charBuf & 0b1000000){ //full
            charBuf = 1; //reset
            Serial.println("RST");
            firstStart = true;
            morse_code_output_warning(piezo1Pin, piezoFreqs[2], ledColor3);
        }
        else{
            Serial.print("-");
            charBuf <<= 1;
            charBuf |= 1;
        }
        
        morse_code_output_on(piezo3Pin, piezoFreqs[1], ledColor2);
        delay(dash_len);

        //counts as falling edge
        //start timing the non button press
        lastOffTime = millis();

        morse_code_output_off(piezo3Pin);
        delay(dash_len);

    }


    //print space if needed
    if(!upState && !downState && !mainState){
        if(!firstStart){
            dt = currTime - lastOffTime; //how much time the button is off
            if(dt > inter_char_len && dt < inter_char_len * 2){
                Serial.print(" ");

                //get character from lookup table
                for(int i=0;i<sizeof(morse_code_keys)/sizeof(morse_code_keys[0]);i++){
                    if(pgm_read_byte(&morse_code_keys[i]) == charBuf){
                        char decoded = pgm_read_byte(&morse_code_chars[i]);
                        LCDPutChar(decoded);
                        LCDRefresh();
                    }
                }

                //reset character buffer as well as off time so space doesn't get printed infinitely
                charBuf = 1;
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



