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
bool lastUpState = !LOW;
bool lastDownState = !LOW;
uint32_t lastOnTime;
uint32_t lastOffTime;
bool firstStart = true; //if true make the first space not print

char charBuf[20];
char fileBuf[900];

const uint32_t piezo_freq_1 = 440;
const uint32_t piezo_freq_2 = 660;

const uint8_t wpm = 15;

uint32_t dot_len;
uint32_t dash_len;
uint32_t dot_thres; //threshold, if ms below this it is considered to be a dot, otherwise dash
uint32_t inter_char_len;

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
    inter_char_len = 7 * dot_len;

    firstStart = true;
    //zero out the buffers
    for(uint32_t i=0;i<sizeof(charBuf)/sizeof(char);i++){
        charBuf[i] = '\0';
    }
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
    //play_ringtone();
}

void loop(){
    for(uint32_t i=0;i<sizeof(charBuf)/sizeof(char);i++){
        charBuf[i] = '\0';
    }
    for(uint32_t i=0;i<sizeof(fileBuf)/sizeof(char);i++){
        fileBuf[i] = '\0';
    }
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
                    Serial.print(".");
                }
                else{
                    Serial.print("-");
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
        morse_code_output_off(piezo2Pin);
        delay(dash_len);

        //counts as falling edge
        //start timing the non button press
        lastOffTime = millis();
    }


    //print space if needed
    if(!upState && !downState && !mainState){
        if(!firstStart){
            dt = currTime - lastOffTime; //how much time the button is off
            if(dt > inter_char_len && dt < inter_char_len * 2){
                Serial.print(" ");
                lastOffTime = -inter_char_len * 3;
            }
        }
    }
    else{
        //if something is pressed remove first start
        firstStart = false;
    }








    lastMainState = mainState;
    lastUpState = upState;
    lastDownState = downState;


}



