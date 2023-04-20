#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(4, 3);

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(38400);
}

void loop() {
    // Read from HC-05 and send data to the Arduino Serial Monitor
  if (Bluetooth.available())
    Serial.write(Bluetooth.read());

    // Read from Arduino Serial Monitor and send it to the HC-05
  if (Serial.available()) {
    int val = Serial.read();
    Serial.write(val);
    Bluetooth.write(val); 
  }
}