#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(6, 5, false);

#define TEST_OUT 11

#define BAUD_FACTOR 3

void setup() {

  pinMode(TEST_OUT, OUTPUT);

  Serial.begin(9600);
  Bluetooth.begin(38400);
  // Bluetooth.begin(9600 << BAUD_FACTOR);
}

int n = 0;

void loop() {

  // Read from HC-05 and send data to the Arduino Serial Monitor
  if (Bluetooth.available()) {
    int val = Bluetooth.read();
    Serial.write(val);
    digitalWrite(TEST_OUT, val);
  }

  // Read from Arduino Serial Monitor and send it to the HC-05
  if (Serial.available()) {
    int val = Serial.read();
    // Serial.write(val);
    Bluetooth.write(val);
  }

  // Bluetooth.println(n);
  // Serial.println(n);
  // delay(500);
  // n++;
}