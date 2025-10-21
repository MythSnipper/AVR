#include <LiquidCrystal.h>

//the screen
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
//wiring
/*
VSS GND
VDD 5V
VO potentiometer middle
RS 12
RW GND
E 11
D4-7 10-7
A(LED+) 5V+330Ω resistor
K(LED-) GND

button 2
piezo 3
*/

const uint8_t buttonPin = 2;
const uint8_t piezoPin = 3;
const uint8_t ledPin = 13;



bool lastState = !LOW;
uint32_t lastTime;

const uint32_t piezo_frequency = 220;

const uint8_t wpm = 10;

uint32_t dot_len;
uint32_t dash_len;
uint32_t dot_thres; //threshold, if ms below this it is considered to be a dot, otherwise dash
uint32_t inter_char_len;
uint32_t inter_word_len;

void setup(){
    //morse code input and output
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    pinMode(piezoPin, OUTPUT);

    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.blink();

    //calculate some things
    dot_len = 1200 / wpm;
    dash_len = 3 * dot_len;
    dot_thres = 1.5 * dot_len;
    inter_char_len = 3 * dot_len;
    inter_word_len = 7 * dot_len;


    Serial.println("Hello World!");
    Serial.print("Dot length: ");Serial.println(dot_len);
    Serial.print("Dash length: ");Serial.println(dash_len);
    Serial.print("inter char length: ");Serial.println(inter_char_len);
    Serial.print("inter word length: ");Serial.println(inter_word_len);
    
    tone(piezoPin, piezo_frequency);
    delay(30);
    noTone(piezoPin);
    delay(100);
    tone(piezoPin, piezo_frequency);
    delay(30);
    noTone(piezoPin);
    Serial.println("Start");

    lastTime = millis();
}

void loop(){
    bool state = !digitalRead(buttonPin);
	digitalWrite(ledPin, state);

    if(state){
        tone(piezoPin, piezo_frequency);
    }
    else{
        noTone(piezoPin);
    }
    if(state != lastState){
        uint32_t dt = millis() - lastTime;
        lastTime = millis();
        //Serial.print(((lastState)?"HIGH":"LOW"));Serial.print(" ");Serial.println(dt, DEC);Serial.print("ms ");
        if(lastState){ //time segment was button on high
            if(dt > 10){
                if(dt < dot_thres){
                    Serial.print(".");
                }
                else{
                    Serial.print("-");
                }
            }
            else{
                Serial.println("ignored");
            }
        }
        else{
            if(dt > inter_word_len){
                Serial.print("/");
            }
            else if(dt > inter_char_len){
                Serial.print(" ");
            }
        }
    }



    lastState = state;
}




