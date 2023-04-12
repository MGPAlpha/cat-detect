#include <LiquidCrystal.h>

#define BUTTON1 8
#define BUTTON2 9

#define MOTION 7

#define RS 12
#define EN 11
#define D4 2
#define D5 3
#define D6 4
#define D7 5

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.print("Hello");

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  String text = "";
  text += "B1 ";
  text += digitalRead(BUTTON1);
  text += " B2 ";
  text += digitalRead(BUTTON2);
  text += " M ";
  text += digitalRead(MOTION);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status");
  lcd.setCursor(0, 1);
  lcd.print(text);
  delay(10);
}
