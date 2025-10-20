#include <LiquidCrystal.h>

//the screen
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup(){
	pinMode(13, OUTPUT);
    Serial.begin(115200);

    lcd.begin(16, 2);
    lcd.print("Hello, World!");
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




