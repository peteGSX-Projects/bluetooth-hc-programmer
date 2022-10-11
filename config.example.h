/*
 * This is an example config.h file for the Bluetooth HC-05/06 programmer.
*/

/////////////////////////////////////////////////////////////////////////////////////
//  Ensure only one instance of config.h is loaded.
// 
#ifndef CONFIG_H
#define CONFIG_H

/////////////////////////////////////////////////////////////////////////////////////
//  Define module type being programmed.
// 
#define TYPE HC05
// #define TYPE HC06

/////////////////////////////////////////////////////////////////////////////////////
//  Uncomment for interactive programming.
//  Note that interactive programming is pointless for the HC-06.
//  If left commented, all programming parameters below must be defined.
// 
// #define INTERACTIVE

/////////////////////////////////////////////////////////////////////////////////////
//  Define programming parameters.
// 
#define NAME BT_NAME
#define PIN 123456

/////////////////////////////////////////////////////////////////////////////////////
//  Define baudrate and pins.
// 
#define BT_HC06_BAUDRATE 9600
#define BT_HC05_BAUDRATE 38400
#define BT_TX 2
#define BT_RX 3
#define BT_KEY 4

#endif