#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial btSerial(2,3);

char c=' ';

void setup() {
  Serial.begin(9600);
  Serial.println("Learning bluetooth apparently");  
  btSerial.begin(38400);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void loop() {
  if (btSerial.available()) {
    c = btSerial.read();
    Serial.write(c);
  }
 if (Serial.available()) {
   c = Serial.read();
   btSerial.write(c);
 }
}
