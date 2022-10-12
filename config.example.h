/*
 * This is an example config.h file for the Bluetooth HC-05/06 programmer.
*/

/////////////////////////////////////////////////////////////////////////////////////
//  Ensure only one instance of config.h is loaded.
// 
#ifndef CONFIG_H
#define CONFIG_H

/////////////////////////////////////////////////////////////////////////////////////
//  Define baudrate and pins.
//  HC-05 default is 38400 in AT mode
//  HC-06 default is 9600
//  EX-CommandStation requires 115200
// 
// #define BT_BAUDRATE 9600
#define BT_BAUDRATE 38400
// #define BT_BAUDRATE 115200
#define BT_TX 2
#define BT_RX 3
#define BT_KEY 4

#endif