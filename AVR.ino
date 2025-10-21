#include <LiquidCrystal.h>

//the screen
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//wiring
/*
VSS GND
VDD 5V
VO potentiometer middle
RS 12
RW GND
E 11
D4-7 5-2
A(LED+) 5V+330Ω resistor
K(LED-) GND
*/

void setup(){
	pinMode(13, OUTPUT);
    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.print("Hello, World!");
    lcd.blink()
}

int sleepTime = 500;
int oah = 22;
void loop(){
    Serial.println("Hello World!");

	digitalWrite(13, HIGH);
	delay(sleepTime);
	digitalWrite(13, LOW);
	delay(sleepTime);

    if(sleepTime > oah){
        sleepTime-=60;
    }

    if(sleepTime < oah){
        sleepTime = oah;
    }

}




