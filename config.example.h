/*
 * This is an example config.h file for the DCC-EX serial throttle.
*/

/////////////////////////////////////////////////////////////////////////////////////
//  Ensure only one instance of config.h is loaded.
// 
#ifndef CONFIG_H
#define CONFIG_H

/*
AT
AT+HELP
AT+NAME?
AT+PIN?
AT+BAUD?
AT+VERSION
*/

/////////////////////////////////////////////////////////////////////////////////////
//  Define baudrate and pins.
// 
#define BT_BAUDRATE 9600
#define BT_TX 2
#define BT_RX 3
#define BT_KEY 4

#endif