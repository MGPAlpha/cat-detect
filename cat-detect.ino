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

#define DISP_OPEN 0
#define DISP_CLOSED 30

#define USE_BT


#ifdef USE_BT

// #include <SoftwareSerial.h>
// #define BT_RX A0
// #define BT_TX A1
#define BT_STATE 13

// SoftwareSerial Bluetooth(BT_TX, BT_RX);

#endif


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

#define EMPTY_FRAME {0, 0, 0, 0, 0, 0, 0, 0}

typedef struct anim_frame {
  byte left[8];
  byte right[8];
} AnimFrame;

typedef struct cat_animation {
  AnimFrame* frames;
  int frameCount;
  bool reversed;
} CatAnimation;

#define MAKE_CAT_ANIM(frames, reversed) ((CatAnimation){(frames), sizeof(frames), (reversed)})

AnimFrame walkFrames[] = {
  (AnimFrame){
    EMPTY_FRAME,
    {B00000, B00001, B00001, B00001, B00000, B00000, B00001, B00001}
  },
  (AnimFrame){
    EMPTY_FRAME,
    {B00000, B00010, B00011, B00011, B00001, B00001, B00001, B00010}
  },
  (AnimFrame){
    EMPTY_FRAME,
    {B00000, B00101, B00111, B00111, B00011, B00011, B00011, B00010}
  },
  (AnimFrame){
    EMPTY_FRAME,
    {B00000, B01010, B01110, B01110, B00111, B00111, B00110, B00101}
  },
  (AnimFrame){
    EMPTY_FRAME,
    {B00000, B10100, B11100, B11100, B01111, B01111, B01100, B10100}
  },
  (AnimFrame){
    {B00000, B00001, B00001, B00001, B00000, B00000, B00000, B00001},
    {B00000, B01000, B11000, B11000, B11111, B11111, B11001, B01010}
  },
  (AnimFrame){
    {B00000, B00010, B00011, B00011, B00001, B00001, B00010, B00010},
    {B00000, B10000, B10000, B10000, B11111, B11111, B10011, B10100}
  },
  (AnimFrame){
    {B00000, B00101, B00111, B00111, B00011, B00011, B00011, B00100},
    {B00000, B00000, B00010, B00001, B11111, B11110, B00110, B10101}
  },
  (AnimFrame){
    {B00000, B01010, B01110, B01110, B00111, B00111, B00110, B00101},
    {B00000, B00000, B00100, B00010, B11110, B11100, B01100, B01010}
  },
  (AnimFrame){
    {B00000, B01010, B01110, B01110, B00111, B00111, B00110, B01010},
    {B00000, B00000, B00100, B00010, B11110, B11100, B01100, B10100}
  }
};

CatAnimation walkInAnim = MAKE_CAT_ANIM(walkFrames, false);
CatAnimation walkOutAnim = MAKE_CAT_ANIM(walkFrames, true);

AnimFrame idleFrames[] = {
  (AnimFrame){
    {B00000, B01010, B01110, B01110, B00111, B00111, B00110, B00110},
    {B00000, B00000, B00100, B00010, B11110, B11100, B01100, B01100}
  },
  (AnimFrame){
    {B00000, B00000, B01010, B01110, B01110, B00111, B00111, B01010},
    {B00000, B00000, B00000, B00010, B00010, B11110, B11100, B01100}
  }
};

CatAnimation idleAnim = MAKE_CAT_ANIM(idleFrames, false);

AnimFrame emptyFrames[] = {
  (AnimFrame){
    EMPTY_FRAME,
    EMPTY_FRAME
  },
  (AnimFrame){
    EMPTY_FRAME,
    EMPTY_FRAME
  }
};

CatAnimation emptyAnim = MAKE_CAT_ANIM(emptyFrames, false);

CatAnimation *currentAnim = &emptyAnim;
int currAnimFrame = 0;
#define ANIM_FRAME_DELAY 500
long animTimer = 0; 

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

  #ifdef USE_BT
  // Bluetooth.begin(38400);
  pinMode(BT_STATE, INPUT);
  #else
  pinMode(MOTION, INPUT);
  #endif

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  dispenser.attach(DISPENSER);

  dispenser.write(DISP_CLOSED);

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

  #ifdef USE_BT

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("Bluetooth");
  delay(750);

  int btState = LOW;
  while (!btState) {
    btState = digitalRead(BT_STATE);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bluetooth");
  lcd.setCursor(0, 1);
  lcd.print("Connected");  
  delay(750);
  
  #endif

  Serial.begin(38400);

}

int motion = LOW;

void loop() {
  
  updateButtons();

  #ifdef USE_BT
  if (Serial.available()) {    
    motion = Serial.read();
  }
  #else
  motion = digitalRead(MOTION);
  #endif

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

  updateAnim(millis());

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
    Serial.println("attempting to dispense");
    dispenser.write(DISP_CLOSED);
    delay(1000);
    dispenser.write(DISP_OPEN);
    delay(1000);
    dispenser.write(DISP_CLOSED);
  }
}

long lastAnimUpdate = 0;
void updateAnim(long t) {
  if (!lastAnimUpdate) lastAnimUpdate = t;
  long delta = t - lastAnimUpdate;
  animTimer += delta;

  CatAnimation *transitionAnimation = NULL;
  if (currentlyDetected && (currentAnim == &emptyAnim || currentAnim == &walkOutAnim)) {
    transitionAnimation = &walkInAnim;
  } else if (!currentlyDetected && (currentAnim == &idleAnim || currentAnim == &walkInAnim)) {
    transitionAnimation = &walkOutAnim;
  } else if (currentAnim == &walkInAnim && currAnimFrame == walkInAnim.frameCount-1 && animTimer > ANIM_FRAME_DELAY) {
    transitionAnimation = &idleAnim;
  } else if (currentAnim == &walkOutAnim && currAnimFrame == walkInAnim.frameCount-1 && animTimer > ANIM_FRAME_DELAY) {
    transitionAnimation = &emptyAnim;
  }
  if (transitionAnimation) {
    if (transitionAnimation == &walkInAnim && currentAnim == &walkOutAnim || transitionAnimation == &walkOutAnim && currentAnim == &walkInAnim) {
      currAnimFrame = currentAnim->frameCount - 1 - currAnimFrame;
    } else {
      currAnimFrame = 0;
    }

    currentAnim = transitionAnimation;
  }

  if (animTimer > ANIM_FRAME_DELAY) {
    animTimer -= ANIM_FRAME_DELAY;
    currAnimFrame++;
    if (currAnimFrame >= currentAnim->frameCount) {
      currAnimFrame = 0;
    }
  }

  int frameToIndex = currentAnim->reversed ? (currentAnim->frameCount - 1 - currAnimFrame) : currAnimFrame;

  lcd.createChar(0, currentAnim->frames[frameToIndex].left);
  lcd.createChar(1, currentAnim->frames[frameToIndex].right);

  lcd.setCursor(14, 1);
  lcd.write(byte(0));
  lcd.setCursor(15, 1);
  lcd.write(byte(1));
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
