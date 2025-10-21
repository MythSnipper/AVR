#include <LiquidCrystal.h>


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
piezos 11 10 9 5
*/

const uint8_t upButtonPin = 4;
const uint8_t downButtonPin = 3;
const uint8_t mainButtonPin = 2;
const uint8_t piezo1Pin = 11;
const uint8_t piezo2Pin = 10;
const uint8_t piezo3Pin = 9;
const uint8_t piezo4Pin = 5;
const uint8_t ledPin = 13;


uint32_t currTime;
uint32_t dt;
bool lastMainState = !LOW;
bool lastUpState = !LOW;
bool lastDownState = !LOW;
uint32_t lastMainTime;
uint32_t lastUpTime;
uint32_t lastDownTime;

char charBuf[20];
char fileBuf[900];

const uint32_t piezo_freq_1 = 440;
const uint32_t piezo_freq_2 = 660;

const uint8_t wpm = 15;

uint32_t dot_len;
uint32_t dash_len;
uint32_t dot_thres; //threshold, if ms below this it is considered to be a dot, otherwise dash
uint32_t inter_char_len;
uint32_t inter_word_len;

void morse_code_output_on(uint8_t piezo, uint32_t piezo_freq){
    digitalWrite(ledPin, HIGH);
    tone(piezo, piezo_freq);
}
void morse_code_output_off(uint8_t piezo){
    digitalWrite(ledPin, LOW);
    noTone(piezo);
}
void play_ringtone(){


    morse_code_output_on(piezo1Pin, 311);
    delay(150);
    morse_code_output_off(piezo1Pin);
    delay(10);
    morse_code_output_on(piezo2Pin, 391);
    delay(150);
    morse_code_output_off(piezo2Pin);
    delay(10);
    morse_code_output_on(piezo3Pin, 466);
    delay(150);
    morse_code_output_off(piezo3Pin);
    delay(10);
    morse_code_output_on(piezo4Pin, 622);
    delay(150);
    morse_code_output_off(piezo4Pin);
    delay(10);

    morse_code_output_on(piezo1Pin, 311);
    morse_code_output_on(piezo2Pin, 391);
    morse_code_output_on(piezo3Pin, 466);
    morse_code_output_on(piezo4Pin, 622);
    delay(600);


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
    pinMode(piezo4Pin, OUTPUT);

    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.blink();

    //calculate some things
    dot_len = 1200 / wpm;
    dash_len = 3 * dot_len;
    dot_thres = 1.8 * dot_len;
    inter_char_len = 3 * dot_len;
    inter_word_len = 7 * dot_len;


    Serial.println("Hello World!");
    Serial.print("Dot length: ");Serial.println(dot_len);
    Serial.print("Dot threshold: ");Serial.println(dot_thres);
    Serial.print("Dash length: ");Serial.println(dash_len);
    Serial.print("inter char length: ");Serial.println(inter_char_len);
    Serial.print("inter word length: ");Serial.println(inter_word_len);
    


    Serial.println("Start");

    play_ringtone();

    lastMainTime = millis();
    lastUpTime = lastMainTime;
    lastDownTime = lastMainTime;

    lcd.print("Hello World!");
}









void loop(){
    bool mainState = !digitalRead(mainButtonPin);
    bool upState = !digitalRead(upButtonPin);
    bool downState = !digitalRead(downButtonPin);


    if(mainState){
        morse_code_output_on(piezo1Pin, piezo_freq_1);
    }
    else{
        morse_code_output_off(piezo1Pin);
    }

    if(mainState != lastMainState){
        currTime = millis();
        if(lastMainState){ //triggered falling edge
            dt = currTime - lastMainTime; //time the button is on
            if(dt > 3){
                if(dt < dot_thres){
                    Serial.print(".");
                }
                else{
                    Serial.print("-");
                }
            }
        }
        else{ //when rising edge, start timing the button press
            lastMainTime = currTime; //lastMainTime is on rising edge, currTime
        }
    }

    if(upState){
        morse_code_output_on(piezo1Pin, piezo_freq_1);
        delay(dot_len);
        morse_code_output_off(piezo1Pin);
        delay(dot_len);
    }
    if(downState){
        morse_code_output_on(piezo1Pin, piezo_freq_2);
        delay(dash_len);
        morse_code_output_off(piezo1Pin);
        delay(dash_len);
    }

    lastMainState = mainState;
    lastUpState = upState;
    lastDownState = downState;


}



