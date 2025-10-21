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
*/

const uint8_t buttonPin = 2;
const uint8_t ledPin = 13;



bool lastState = LOW;
uint32_t lastTime;
const uint8_t wpm = 20;
uint32_t dot_len;
uint32_t dash_len;
uint32_t intra_char_len;
uint32_t inter_char_len;
uint32_t inter_word_len;

void setup(){
    //morse code input and output
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);

    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.blink();

    //calculate some things
    dot_len = 120 / wpm;
    dash_len = 3 * dot_len;
    intra_char_len = dot_len;
    inter_char_len = 3 * dot_len;
    inter_word_len = 7 * dot_len;

    delay(2000);

    Serial.println("Hello World!");
    Serial.println("Dot length: "+dot_len);
    Serial.println("Dash length: "+dash_len);
    Serial.println("intra char length: "+intra_char_len);
    Serial.println("inter char length: "+inter_char_len);
    Serial.println("inter word length: "+inter_word_len);
    Serial.println("Waiting for first button press...");
    while(!digitalRead(buttonPin));
}

void loop(){
    bool state = digitalRead(buttonPin);
	
    lcd.print("O");
    delay(400);
    lcd.print("A");
    delay(400);
    lcd.print("H");
    delay(400);
    
}




