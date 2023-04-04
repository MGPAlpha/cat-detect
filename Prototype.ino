#include <Servo.h>

//Servo instatiation 
Servo myServo;

//Extra pin setup
int IRSensor = 3;
int button = 4;

//variable for storing IR Sensor data
int irVal;
int buttonVal;

void setup()   
{ // put your setup code here, to run once:
  pinMode(IRSensor, INPUT);
  pinMode(button, INPUT);

  myServo.attach(6);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  irVal = digitalRead(IRSensor);
  buttonVal = digitalRead(button);

  if (irVal) {
    Serial.println("Movement!");
    if (button == HIGH) {
      myServo.write(90);
    }
  } else {
    Serial.println("No Movement!");
    myServo.write(0);
  }
}
