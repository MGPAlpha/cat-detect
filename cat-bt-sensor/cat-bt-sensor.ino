#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(6, 5, false);

#define BAUD_FACTOR 0

#define SENSOR 12

void setup() {

  pinMode(SENSOR, INPUT_PULLUP);
  Serial.begin(9600);
  Bluetooth.begin(9600 << BAUD_FACTOR);
}

int n = 0;

void loop() {

  int sensorValue = digitalRead(SENSOR);
  
  Bluetooth.write(sensorValue);

  // Bluetooth.println(n);
  // Serial.println(n);
  // delay(500);
  // n++;
}