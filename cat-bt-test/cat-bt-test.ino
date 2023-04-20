#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(3, 2, false);

#define BAUD_FACTOR 0

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600 << BAUD_FACTOR);
}

int n = 0;

void loop() {

    // Read from HC-05 and send data to the Arduino Serial Monitor
  if (Bluetooth.available())
    Serial.write(Bluetooth.read());

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