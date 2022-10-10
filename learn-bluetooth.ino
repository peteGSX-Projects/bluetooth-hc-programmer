/*
 *  © 2022 Peter Cole
 *
 *  This file is for programming HC-05/6 Bluetooth modules.
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this code.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial btSerial(2,3);

char c=' ';

void setup() {
  Serial.begin(9600);
  Serial.println("Learning bluetooth apparently");  
  // btSerial.begin(9600);
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
