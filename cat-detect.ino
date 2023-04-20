#include <LiquidCrystal.h>
#include <TimeLib.h>
#include <Servo.h>

#define BUTTON1 8
#define BUTTON2 9

#define MOTION 7

#define RS 12
#define EN 11
#define D4 2
#define D5 3
#define D6 4
#define D7 5

#define DISPENSER 6

Servo dispenser;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

byte upArrow[8] = {
	0b00000,
	0b00100,
	0b01110,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

typedef struct cat_time {
  long startStamp;
  long endStamp;
} CatTime;

#define CAT_TIMES_START_SIZE 20
int catTimesSize = CAT_TIMES_START_SIZE;
CatTime *catTimes;
int lastCatTimeIndex = -1;

bool currentlyDetected = false;
CatTime currentDetection;

bool buttons[2] = {0, 0};
bool lastButtons[2] = {0, 0};
bool justPressedButtons[2] = {0, 0};

void updateButtons() {
  lastButtons[0] = buttons[0];
  lastButtons[1] = buttons[1];
  buttons[0] = !digitalRead(BUTTON1);
  buttons[1] = !digitalRead(BUTTON2);
  justPressedButtons[0] = buttons[0] && !lastButtons[0]; 
  justPressedButtons[1] = buttons[1] && !lastButtons[1]; 
}

void setup() {

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  dispenser.attach(DISPENSER);

  int clockDigits[5] = {0, 0, 0, 0, 0}; // H1, H0, M1, M0, AM/PM
  int clockMaxes[5] = {1, 9, 5, 9, 1};

  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.print("Hello");

  lcd.createChar(0, upArrow);

  int currTimeDigit = 0;


  while (currTimeDigit < 5) {
    updateButtons();

    if (justPressedButtons[0]) {
      
      int currDigit = clockDigits[currTimeDigit];
      int maxDigit = clockMaxes[currTimeDigit];
      currDigit++;
      if (currDigit > maxDigit) currDigit = 0;
      clockDigits[currTimeDigit] = currDigit;

    }

    if (justPressedButtons[1]) {
      currTimeDigit++;
      if (clockDigits[0] > 0) clockMaxes[1] = 2;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    String timeString = "Time: ";
    timeString += clockDigits[0];
    timeString += clockDigits[1];
    timeString += ":";
    timeString += clockDigits[2];
    timeString += clockDigits[3];
    timeString += " ";
    timeString += clockDigits[4] ? "PM" : "AM";
    lcd.print(timeString);

    lcd.setCursor(currTimeDigit*3/2+6, 1);
    lcd.write((byte)0);
    
    delay(10);
  }

  int hr = clockDigits[0] * 10 + clockDigits[1];
  if (hr == 12) hr = 0;
  hr += (clockDigits[4] ? 12 : 0);
  int min = clockDigits[2] * 10 + clockDigits[3];
  setTime(hr, min, 0, 0, 0, 0);

  catTimes = malloc(catTimesSize * sizeof(CatTime));

}



void loop() {
  
  updateButtons();
  int motion = digitalRead(MOTION);
  
  bool seenRecently = currentlyDetected;
  if (!seenRecently && lastCatTimeIndex >= 0) {
    long currTime = now();
    long lastSeen = catTimes[lastCatTimeIndex].endStamp;
    long timeSince = currTime - lastSeen;
    if (timeSince > 5 * 60 * 1000) { // if last 5 mins
      seenRecently = true;
    }
  }

  lcd.clear();
  // lcd.print("Status");
  if (seenRecently && millis()/2000 % 2) {
    lcd.setCursor(0, 0);
    lcd.print("Press for");
    lcd.setCursor(0, 1);
    lcd.print("Treat");
  } else if (currentlyDetected) {
    lcd.setCursor(0, 0);
    lcd.print("Cat is");
    lcd.setCursor(0, 1);
    lcd.print("present");
  } else if (lastCatTimeIndex >= 0) {
    lcd.setCursor(0, 0);
    lcd.print("Last seen");
    lcd.setCursor(0, 1);
    String timeString = createTimeString(catTimes[lastCatTimeIndex].endStamp);
    lcd.print(timeString);
  }

  delay(10);

  if (!currentlyDetected && motion) {
    currentlyDetected = true;
    currentDetection.startStamp = now();
    currentDetection.endStamp = 0;
  }
  if (currentlyDetected && !motion) {
    currentlyDetected = false;
    currentDetection.endStamp = now();

    lastCatTimeIndex++;
    if (lastCatTimeIndex >= catTimesSize) {
      int newCatTimesSize = 2 * catTimesSize;
      int newCatTimes = realloc(catTimes, newCatTimesSize * sizeof(CatTime));
      if (newCatTimes) {
        // Complete Realloc
        catTimes = newCatTimes;
        catTimesSize = newCatTimesSize;
      } else {
        for (int i = 0; i < catTimesSize - CAT_TIMES_START_SIZE; i++) {
          catTimes[i] = catTimes[i + CAT_TIMES_START_SIZE];
        }
        lastCatTimeIndex -= CAT_TIMES_START_SIZE;        
      }
    }
    catTimes[lastCatTimeIndex] = currentDetection;
  }

  if (seenRecently && (justPressedButtons[0] || justPressedButtons[1])) {
    dispenser.write(0);
    delay(1000);
    dispenser.write(180);
    delay(1000);
    dispenser.write(0);
  }
}

String createTimeString(long stamp) {
  int hours = hour(stamp);
  int minutes = minute(stamp);
  bool pm = hours >= 12;
  if (pm) {
    hours -= 12;
  }
  if (hours == 0) hours = 12;
  String hourStr = String(hours);
  String minuteStr = String(minutes);
  String pmStr = pm ? "PM" : "AM";
  if (hourStr.length() == 1) hourStr = String("0") + hourStr;
  if (minuteStr.length() == 1) minuteStr = String("0") + minuteStr;
  String out = hourStr + ":" + minuteStr + " " + pmStr;
  return out;
}
