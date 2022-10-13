/*
 *  © 2022 Peter Cole
 *
 *  This file is for programming HC-05/6 Bluetooth modules.
 * 
 *  Credit to multiple different examples on the Internet for this code, it
 *  didn't orginate entirely from my head.
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

/***********************************************************************************
If we haven't got a custom config.h, use the example
***********************************************************************************/
#if __has_include ( "config.h")
  #include "config.h"
#else
  #warning config.h not found. Using defaults from config.example.h
  #include "config.example.h"
#endif

// Initialise software serial interface
SoftwareSerial btSerial(BT_TX,BT_RX);

// Placeholder char
char c=' ';

void setup() {
  // Start USB serial connection to the programmer
  Serial.begin(9600);
  Serial.println("HC-05/06 programmer");
  // Start Bluetooth serial connection
  btSerial.begin(BT_BAUDRATE);
  pinMode(4, OUTPUT);
  // Put HC-05 in AT mode if connected
  digitalWrite(4, HIGH);
}

void loop() {
  // Write out any responses from Bluetooth to serial console
  if (btSerial.available()) {
    c = btSerial.read();
    Serial.write(c);
  }
  // Write out any user entered commands from the serial console to Bluetooth serial
  if (Serial.available()) {
    c = Serial.read();
    btSerial.write(c);
  }
}
