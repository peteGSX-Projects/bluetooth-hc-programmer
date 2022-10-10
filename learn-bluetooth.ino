//this sketch uses Serial port as interface to User Entry
//and Serial1 as interface to HC-06

#include <SoftwareSerial.h>

SoftwareSerial Serial1(2,3);

const int constMainMenuChoiceScanForHC06 = 1; 
const int constMainMenuChoiceSetHC06BaudRate = 2;
const int constMainMenuChoiceSetHC06Name = 3;
const int constMainMenuChoiceSetHC06PairingPIN = 4;
const int constMainMenuChoiceSetHC06Parity = 5;
const int constMainMenuChoiceGetHC06Version = 6;
const int constMainMenuChoiceTurnHC06LED_OnOff = 7;
const int constMainMenuChoiceSetHC06SlaveMasterMode = 8;

const char constMainMenuChoiceMin = constMainMenuChoiceScanForHC06;
const char constMainMenuChoiceMax = constMainMenuChoiceSetHC06SlaveMasterMode;
const unsigned int constMaxCharSetHC06Name = 20;

const unsigned int const_uintMenuChoiceToExitToMain = 0; //for the use of setting module baud rate (??and parity)

const unsigned int const_uintBaudRateIndexValueForUnknownHC06 = 0;

const char const_char_CR = 0x0D;
const char const_char_LF = 0x0A; 

//3/16/2021, The observed max was about 530ms
const unsigned long constTimeOutOnHC06ResponseWithoutEntryMs = 750;

//3/16/2021, about 7ms per character entry @1200, let's more than triple it
const unsigned long constTimeOutOnHC06ResponseWithEntryMs = 50;
 
//to be used to determine if there is no longer any user entry
//@9600, each character takes about 1ms, let's make it 10ms
const unsigned long const_ulong_timeout_fromLastUserEntry_millis = 10;

//for both Main Menu and setHC06BaudRate choice entry
char char_valid_entry;
unsigned int uint_valid_entry;

unsigned long ulongInterfacingBaudRate;
//unsigned long ulongHC06BaudRate = const_ulongBaudRateValueForUnknownHC06;
bool bool_return_to_MainMenu;

//for sprintf(...) to format value into string
//used on formatting of baud rate at set module baud rate
//plus, for comparing the baud rate response of set Stored baud rate 
//currently, maximum use at 11 (sprintf(arChar_sprintf, "%c. %7ld", tmp_c, fulong_get_BaudRateByIndex(tmp_int_index-1));)
char arChar_sprintf[25];

//this covers User Entry as well as HC-06 response
const unsigned int max_uintReceivedSerialInputDataCount = 23;
char archarReceivedSerialInputData[max_uintReceivedSerialInputDataCount];//max is 22 for name entry + CR/LF + trailing 0x00(might not be needed)
unsigned int uintReceivedSerialInputDataCount = 0;
unsigned int uintReceivedSerialInputDataIndex = 0;

const unsigned int max_uintCommandToHC06Count = 28;
//2/25/21, max being AT+NAME(up to 20 characters) plus trailing 0x00
char archarCommandToHC06[max_uintCommandToHC06Count];

//3/3/2021, removed for safety and easy explanation
//char *ptrCharSerialInputData, *ptrCharSerialOut; //from Arduino's perspective

unsigned long ulong_timeout_count_start_millis;
unsigned long ulong_command_start_millis;

//Hardware Serial's maximun speed is 115200??? (how about 230400??
const unsigned int const_uint_maxHC06BaudRateIndex = 9; //for both scanning and menu, total baud rate - 1 (before 8: 0~7, now: 9: 1~8)
const unsigned int const_uint_minHC06ScanBaudRateIndex = 1;  
const unsigned int const_uint_minHC06SetBaudRateIndex = 0;  
unsigned long arulHC06BaudRate[const_uint_maxHC06BaudRateIndex] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
unsigned int uintHC06BaudRateIndex = const_uintBaudRateIndexValueForUnknownHC06;
unsigned int uintInterfacingHC06_BaudRateIndex;

const unsigned int const_uint_maxSetParityChoiceIndex = 3; 
const unsigned int const_uint_minSetParityChoiceIndex = 1; 
const unsigned int const_uintSubmenuChoiceSetParityToNone = 1;
const unsigned int const_uintSubmenuChoiceSetParityToEven = 2;
const unsigned int const_uintSubmenuChoiceSetParityToOdd = 3;
const unsigned int const_uintParityValueForUnknownHC06 = 0;

unsigned int uintInterfacingHC06_Parity; 
unsigned int uintHC06ScannedParity = const_uintParityValueForUnknownHC06;
unsigned int uintHC06ParityUponPowerCycling = const_uintParityValueForUnknownHC06;

const unsigned int const_uint_maxTurnLED_OnOffChoiceIndex = 2; 
const unsigned int const_uintSubmenuChoiceTurnOffLED = 1;
const unsigned int const_uintSubmenuChoiceTurnOnLED = 2;

const unsigned int const_uint_maxSetSlaveMasterModeChoiceIndex = 2; 
const unsigned int const_uintSubmenuChoiceSetSlaveMode = 1;
const unsigned int const_uintSubmenuChoiceSetMasterMode = 2;


unsigned long fulong_get_HC06BaudRate() {
  return arulHC06BaudRate[uintHC06BaudRateIndex];
}

unsigned int fuint_get_InterfacingHC06_BaudRateIndex() {
  return uintInterfacingHC06_BaudRateIndex;
}

void set_uintInterfacingHC06_BaudRateIndex(unsigned int passed_uint_index) {
  uintInterfacingHC06_BaudRateIndex = passed_uint_index;
}

//once the Scan for HC-06 was executed successfully
void copy_InterfacingHC06BaudRateIndex_to_HC06BaudRateIndex() {
  uintHC06BaudRateIndex = uintInterfacingHC06_BaudRateIndex;
}

unsigned long fulong_get_InterfacingHC06_BaudRate() {
  return arulHC06BaudRate[uintInterfacingHC06_BaudRateIndex];
}

//for user desired baud rate
unsigned long fulong_get_BaudRateFromHC06Array(unsigned int passed_uint_index) {
  return arulHC06BaudRate[passed_uint_index];
}

void set_uintHC06BaudRateIndex(unsigned int passed_uintIndex) {
 uintHC06BaudRateIndex = passed_uintIndex; 
}
void set_HC06BaudRateIndex_toUnknown() {
 set_uintHC06BaudRateIndex(const_uintBaudRateIndexValueForUnknownHC06); 
}
unsigned int fuint_get_HC06BaudRateIndex() {
  return uintHC06BaudRateIndex;
}
bool fbool_HC06BaudRate_isUnknown() {
  return (fuint_get_HC06BaudRateIndex() == const_uintBaudRateIndexValueForUnknownHC06);
}

void reset_InterfacingHC06_BaudRateIndex() {
 uintInterfacingHC06_BaudRateIndex = const_uint_minHC06ScanBaudRateIndex; 
}
bool fbool_InterfacingHC06_BaudRateIndex_reaches_Max() {
 return (uintInterfacingHC06_BaudRateIndex == const_uint_maxHC06BaudRateIndex);
}
void advance_InterfacingHC06_BaudRateIndex() {
  uintInterfacingHC06_BaudRateIndex ++;
}

unsigned int fuint_get_HC06ScannedParity() {
  return uintHC06ScannedParity;
}
void set_uintHC06ScannedParity(unsigned int passed_uintParity) {
  uintHC06ScannedParity = passed_uintParity;
}
bool fbool_HC06ScannedParity_isUnknown() {
  return (uintHC06ScannedParity == const_uintParityValueForUnknownHC06);
}
void set_HC06ScannedParity_toUnknown() {
  uintHC06ScannedParity = const_uintParityValueForUnknownHC06;
}

unsigned int fuint_get_HC06ParityUponPowerCycling() {
  return uintHC06ParityUponPowerCycling;
}
void set_uintHC06ParityUponPowerCycling(unsigned int passed_uintParity) {
  uintHC06ParityUponPowerCycling = passed_uintParity;
}
bool fbool_HC06ParityUponPowerCycling_isUnknown() {
  return (uintHC06ParityUponPowerCycling == const_uintParityValueForUnknownHC06);
}

void set_InterfacingHC06_Parity(unsigned int passed_uintParity) {
  uintInterfacingHC06_Parity = passed_uintParity;
}
unsigned int fuint_get_InterfacingHC06_Parity() {
  return uintInterfacingHC06_Parity;
}

bool fbool_anHC06_isNotYetDetected() {
  return (fbool_HC06BaudRate_isUnknown() || fbool_HC06ScannedParity_isUnknown());
}

void set_HC06SerialConfiguration_toUnknown() {
  set_HC06BaudRateIndex_toUnknown();
  set_HC06ScannedParity_toUnknown();
}

char fchar_decimal_int_to_hexadecimal_char(int passed_int_decimal) {
  if (passed_int_decimal <= 9)
    return(char('0'+passed_int_decimal));
  else
    return(char('0'+passed_int_decimal+7));  
}

unsigned int fuint_hexadecimal_char_to_decimal_int(char passed_char_hexadecimal) {
  
  if (passed_char_hexadecimal <= '9')
    return((unsigned int)(passed_char_hexadecimal-'0'));
  else
    return((unsigned int)(passed_char_hexadecimal-'0'-7));  
}

void dry_read_all_serial_inputs()
{
  bool bool_tmp_any_input = false;
  
  while(1)
    {
     if (Serial.available()) 
       { 
        Serial.read();
        bool_tmp_any_input = true;
       } 
     if (Serial1.available())
       { 
        Serial1.read();
        bool_tmp_any_input = true;
       } 
     if (!bool_tmp_any_input)
       break;
     bool_tmp_any_input = false; //over again           
    }
}

//for user entry, the count excludes the trailing CRLF
unsigned int fuint_get_ReceivedSerialInputDataCount() {
  return uintReceivedSerialInputDataCount;  
}
void set_ReceivedSerialInputDataCount(unsigned int passed_uint_Count) {
 uintReceivedSerialInputDataCount = passed_uint_Count;  
}

void rewind_ReceivedSerialInputDataIndex() {
  uintReceivedSerialInputDataIndex = 0;
}
void fill_ReceivedSerialInputDataWithAutoAdvancingIndex(char passed_char_SeialInputData) {
  archarReceivedSerialInputData[uintReceivedSerialInputDataIndex++] = passed_char_SeialInputData;
}
char fchar_get_ReceivedSerialInputData_byIndex(int passed_index) {
  return (archarReceivedSerialInputData[passed_index]);
}
void set_ReceivedSerialInputData_byIndex(unsigned int passed_uint_index, char passed_char) {
  archarReceivedSerialInputData[passed_uint_index] = passed_char;
}
void reset_ReceivedSerialInputData() {
  unsigned int tmp_uint_index;
  
  for (tmp_uint_index = 0; tmp_uint_index <  max_uintReceivedSerialInputDataCount; tmp_uint_index++)
    set_ReceivedSerialInputData_byIndex(tmp_uint_index, 0x00); 
}

//Serial response can be either from user or HC-06 
//as the configurator does not deal with both at the same time
void initialize_ReceiveSerialInputData()
{   
 reset_ReceivedSerialInputData(); 
 rewind_ReceivedSerialInputDataIndex(); //for filling of Serial Input ata  
 dry_read_all_serial_inputs();  
}

void display_SerialInputDataArray() {
  Serial.println(archarReceivedSerialInputData);
}

//sending command to HC06 then wait for response
//return true if response was none or too many 
bool fbool_send_Command_to_HC06_then_check_response()
{
 unsigned long tmp_ulong_timelapse_to_use;
 unsigned char tmp_char_HC06InputData;
 unsigned int tmp_uint_HC06InputDataCount = 0; 
 bool tmp_bool_too_many_inputs = false;
 unsigned long tmp_ulongTimeoutStart, tmp_ulongCommandStart;;
 
 initialize_ReceiveSerialInputData();
 Serial1.print(archarCommandToHC06);
  
 Serial1.flush(); //to ensure that the output was sent completely
 tmp_ulongTimeoutStart = millis();  
 tmp_ulongCommandStart = tmp_ulongTimeoutStart; 
  
 while(true)
   {
    //anything coming from user would be read and ignored
    //this is out of caution, to prevent serial buffer overflow
    if (Serial.available())
      Serial.read(); 
      
    if (Serial1.available())
      {
       tmp_char_HC06InputData = char(Serial1.read()); //always read, regardless
       tmp_uint_HC06InputDataCount++;
       if (!tmp_bool_too_many_inputs) {
         if (tmp_uint_HC06InputDataCount == max_uintReceivedSerialInputDataCount) {
           set_ReceivedSerialInputData_byIndex((max_uintReceivedSerialInputDataCount-1), 0x00); //fill last in index will 0x00 to allow for display
           tmp_bool_too_many_inputs = true;
           }
         else {//char array is not yet filled up 
           fill_ReceivedSerialInputDataWithAutoAdvancingIndex(tmp_char_HC06InputData); 
           tmp_ulongTimeoutStart = millis();//extend wait time for any response
           }
         } //end bracket for (!tmp_bool_too_many_inputs)
      } //end bracket for if (Serial.available())
                   
    if (tmp_uint_HC06InputDataCount) //once any response was received, wait time should be much reduced
      tmp_ulong_timelapse_to_use = constTimeOutOnHC06ResponseWithEntryMs; //3/2/2021, currently 250ms
    else
      tmp_ulong_timelapse_to_use = constTimeOutOnHC06ResponseWithoutEntryMs; //currently 1250ms 
      
    if ((millis()-tmp_ulongTimeoutStart) > tmp_ulong_timelapse_to_use) //time up
      break;   
   } 

 if (tmp_bool_too_many_inputs) { 
   Serial.print("Error: number of HC06 response exceeds max (");
   Serial.print(max_uintReceivedSerialInputDataCount);
   Serial.println(")");
   } 
 else {
   if (tmp_uint_HC06InputDataCount != 0) {
     set_ReceivedSerialInputDataCount(tmp_uint_HC06InputDataCount);
     return true;      
   }
 }

 return false;  
} 

void display_Parity(unsigned int passed_uintParityType) {
  
  switch(passed_uintParityType) {
    case const_uintSubmenuChoiceSetParityToNone:
      Serial.print(F("None"));
      break;
    case const_uintSubmenuChoiceSetParityToEven:
      Serial.print(F("Even"));      
        break;
    case const_uintSubmenuChoiceSetParityToOdd:
      Serial.print(F("Odd")); 
      break;         
      }
}

bool fbool_search_HC06() {
 unsigned long tmp_ulong_InterfacingHC06_BaudRate;
 bool tmp_bool = false;

 Serial.print(F("Search for HC-06 @"));
 tmp_ulong_InterfacingHC06_BaudRate = fulong_get_InterfacingHC06_BaudRate();
 Serial.print(tmp_ulong_InterfacingHC06_BaudRate);  
 Serial.print(F(", Parity ")); 
 display_Parity(fuint_get_InterfacingHC06_Parity());
 Serial.print(": ");
 if (fbool_send_Command_to_HC06_then_check_response() && (strncmp(archarReceivedSerialInputData, "OK", 2) == 0))  //correct response is OK
   tmp_bool = true;
 else  
   Serial.print("None ");
 
 Serial.println("Detected!");
 
 return tmp_bool;     
}

//with InterfacingParity and Interfacing Baud Rate index known
void switch_InterfacingH06_SerialParameters() {
  unsigned long tmp_ulongInterfacingHC06_BudRate;
  unsigned int tmp_uintInterfacingHC06_Parity;

  tmp_ulongInterfacingHC06_BudRate = fulong_get_InterfacingHC06_BaudRate();
  //set_InterfacingBaudRate(tmp_ulongInterfacingBudRate);
  tmp_uintInterfacingHC06_Parity = fuint_get_InterfacingHC06_Parity();
  switch(tmp_uintInterfacingHC06_Parity) {
    case const_uintSubmenuChoiceSetParityToNone:
      Serial1.begin(tmp_ulongInterfacingHC06_BudRate);
      break;
    case const_uintSubmenuChoiceSetParityToEven:
      Serial1.begin(tmp_ulongInterfacingHC06_BudRate, SERIAL_8E1); //!!! failed with 7E1 
      break;
    case const_uintSubmenuChoiceSetParityToOdd:
      Serial1.begin(tmp_ulongInterfacingHC06_BudRate, SERIAL_8O1); //!!! failed with 7O1
      break;         
  }
}

void set_HC06BaudRate_to_Unknown() {
  set_uintHC06BaudRateIndex(const_uintBaudRateIndexValueForUnknownHC06);
}

void display_return_to_MainMenu() {
  Serial.println(F("0. Return to Main Menu"));
}

void display_HC06BaudRate() {
  Serial.print(F("\nBaud Rate of HC-06: ")); 
  Serial.println(fulong_get_HC06BaudRate());
}

void display_HC06Parity() {
  if (!fbool_HC06ParityUponPowerCycling_isUnknown()) {
    Serial.print(F("Parity of HC-06 Upon Power Cycling: "));
    display_Parity(fuint_get_HC06ParityUponPowerCycling()); 
    Serial.println();   
    }
  
  Serial.print(F("Scanned Parity of HC-06: "));
  display_Parity(fuint_get_HC06ScannedParity()); //??None, Even, or Odd
  Serial.println();
}

void display_InterfacingParity() {
  Serial.print(F("Parity Interfacing with HC-06: "));
  display_Parity(fuint_get_InterfacingHC06_Parity()); //??None, Even, or Odd
  Serial.println();
}

void display_SetHC06BaudRateChoices() {
  int tmp_int_index;
  char tmp_c;
  
  display_HC06BaudRate();
  for (tmp_int_index = 0; tmp_int_index < const_uint_maxHC06BaudRateIndex; tmp_int_index++) {
    if (tmp_int_index == 0)
      display_return_to_MainMenu();
    else {  
      tmp_c = fchar_decimal_int_to_hexadecimal_char(tmp_int_index);
      sprintf(arChar_sprintf, "%c. %7ld", tmp_c, fulong_get_BaudRateFromHC06Array(tmp_int_index));
      Serial.println(arChar_sprintf);
      }
    }
  Serial.print("Please enter a choice: ");
} 

void set_True_bool_return_to_MainMenu() {
  bool_return_to_MainMenu = true;
}
void set_False_bool_return_to_MainMenu() {
  bool_return_to_MainMenu = false;
}
bool fbool_return_to_MainMenu() {
  return (bool_return_to_MainMenu);
}

bool fbool_char_is_a_digit(char passed_char) {
  if ((passed_char >= '0') && (passed_char <= '9')) 
    return true;
   else
     return false;
}

bool fbool_char_is_A_to_F(char passed_char) {
  if ((passed_char >= 'A') && (passed_char <= 'F')) 
    return true;
   else
     return false;
}

bool fbool_char_is_hexdecimal(char passed_char) {
  if (fbool_char_is_a_digit(passed_char) || fbool_char_is_A_to_F(passed_char)) 
    return true;      
  else     
    return false;  
}

//needed for setting Baud Rate AT+BAUDn
void set_char_valid_entry(char passed_char_entry) {
  char_valid_entry = passed_char_entry; 
}
char fchar_get_valid_entry() {
  return char_valid_entry;
}

void set_uint_valid_entry(unsigned int passed_uint_entry) {
  uint_valid_entry = passed_uint_entry; 
}
unsigned int fuint_get_valid_entry() {
  return uint_valid_entry;
}

bool fbool_UserMenuChoice_is_single() {
  return (fuint_get_ReceivedSerialInputDataCount() == 1);
}

//3/2/2021, to qualify menu choice entry
//if successful, both char_valid_entry and int_valid_entry will be set
bool fbool_UserMenuChoice_is_single_in_range(unsigned int passed_uint_min, unsigned int passed_uint_max) {
  char tmp_char_entry;
  int tmp_uint_entry;

  while(true) {
    if (!fbool_UserMenuChoice_is_single()) { 
      Serial.println("Error: menu choice is not a single character");
      break;
      }
      
    tmp_char_entry = fchar_get_ReceivedSerialInputData_byIndex(0);
    if (!fbool_char_is_hexdecimal(tmp_char_entry)) { //for now, decimal will do, eventually, hexadecimal
      Serial.println("Error: menu choice is not hexadecimal");
      break;
      }
      
    tmp_uint_entry = fuint_hexadecimal_char_to_decimal_int(tmp_char_entry);
    if ((tmp_uint_entry < passed_uint_min) || (tmp_uint_entry > passed_uint_max)) { //choice was out of range
      Serial.println("Error: menu choice is out of range");
      break;
      }

    set_char_valid_entry(tmp_char_entry); //needed for setting baud rate (AT+BAUDn)
    set_uint_valid_entry(tmp_uint_entry); //needed for menu decision  
    
    return true;
    }
  return false;  
}

//the valid user entry is a single character within range, followed by CR & LF
//invalid entry will be displayed up to the holding limit of the character array
//for valid entry (with trailing CR & LF), the CR location will be replaced by an 0x00
//in addition, the ReceivedSerialInputDataCount excludes CR & LF
bool fbool_ReceivedUserEntry_with_TrailingCRLF()
{
 char tmp_char_ReceivedUserEntryData;
 unsigned int tmp_uintUserEntryDataCount = 0; 
 bool tmp_bool_too_many_inputs = false; 
 unsigned long tmp_ulong_last_UserEntry_Arrival_millis;
 unsigned int tmp_uint_expected_CR_location;
 char tmp_char_at_expected_CR_location, tmp_char_at_expected_LF_location;

  
 initialize_ReceiveSerialInputData(); 
 Serial.flush(); //clear output buffer to prepare to switch baud rate

 while(true)
   {
    //anything coming from BT would be read and ignored
    //this is out of caution, to prevent serial buffer overflow
    if (Serial1.available())
      Serial1.read();
            
    if (Serial.available()) 
      { 
       tmp_ulong_last_UserEntry_Arrival_millis = millis(); //each input could be the last 
       tmp_char_ReceivedUserEntryData = char(Serial.read());
       tmp_uintUserEntryDataCount++;
       if (!tmp_bool_too_many_inputs) {
         if (tmp_uintUserEntryDataCount == max_uintReceivedSerialInputDataCount) {
           set_ReceivedSerialInputData_byIndex((tmp_uintUserEntryDataCount-1), 0x00); //fill last in index will 0x00 to allow for display
           tmp_bool_too_many_inputs = true;
           }
         else  //char array is not yet filled up 
           fill_ReceivedSerialInputDataWithAutoAdvancingIndex(tmp_char_ReceivedUserEntryData);
         } //end bracket for (!tmp_bool_too_many_inputs)
      } //end bracket for if (Serial.available())
    
    //use timeout to determine end of entry, checked only after any response has been received
    if ((tmp_uintUserEntryDataCount > 0) && ((millis() -  tmp_ulong_last_UserEntry_Arrival_millis) > const_ulong_timeout_fromLastUserEntry_millis)) 
      break;  
   } //end bracket for while (true)

 //echo user entry
 display_SerialInputDataArray(); 

 if (tmp_bool_too_many_inputs) { 
   Serial.print("Error: number of user entry exceeds max (");
   Serial.print(max_uintReceivedSerialInputDataCount-3); //excluding CRLF
   Serial.println(")");
   }
 else {  
   if (tmp_uintUserEntryDataCount < 3)
     Serial.println(F("Error: too few entry (minimum: 1 character + CR + LF)"));
   else {
     tmp_uint_expected_CR_location = tmp_uintUserEntryDataCount - 2;
     tmp_char_at_expected_CR_location = fchar_get_ReceivedSerialInputData_byIndex(tmp_uint_expected_CR_location);
     tmp_char_at_expected_LF_location = fchar_get_ReceivedSerialInputData_byIndex(tmp_uint_expected_CR_location+1);
     
     if ((tmp_char_at_expected_CR_location == const_char_CR) && (tmp_char_at_expected_LF_location == const_char_LF)) {
       set_ReceivedSerialInputDataCount(tmp_uint_expected_CR_location);
       set_ReceivedSerialInputData_byIndex(tmp_uint_expected_CR_location, 0x00); //replaced CR with 0x00
       return true;
       }
     else
       Serial.println("Error: entry did not end with CR + LF");
   }
 }
 return false;     
}

//from lowest baud rate to the highest, Arduino sends "AT" to HC-06 
//if "OK" was received from HC-06, then the stored parity/baud rate would be determined
void execute_ScanForHC06() {
  unsigned int tmp_uint_Parity_index;
  bool tmp_bool_exit = false;

  Serial.println(F("Scan for an HC-06"));
  set_HC06SerialConfiguration_toUnknown();
  strcpy(archarCommandToHC06, "AT");
  for (tmp_uint_Parity_index = const_uint_minSetParityChoiceIndex; tmp_uint_Parity_index <=  const_uint_maxSetParityChoiceIndex; tmp_uint_Parity_index ++) {
    set_InterfacingHC06_Parity(tmp_uint_Parity_index);
    reset_InterfacingHC06_BaudRateIndex();
    Serial.println();

    Serial.print(F("Set Interfacing Parity to ")); 
    display_Parity(tmp_uint_Parity_index);
    Serial.println();
    
    while(true) { 
      switch_InterfacingH06_SerialParameters(); //change interfacing baud rate and parity accordingly    
      if (fbool_search_HC06()) {
        copy_InterfacingHC06BaudRateIndex_to_HC06BaudRateIndex();
        set_uintHC06ScannedParity(tmp_uint_Parity_index);
        tmp_bool_exit = true;
        break;
        }
      else {
        advance_InterfacingHC06_BaudRateIndex();
        if (fbool_InterfacingHC06_BaudRateIndex_reaches_Max()) 
          break;  
      } 
    }
   if (tmp_bool_exit)
     break; //successfully scan   
  }
}//3/8/2021, added to make even bracket pair

void scan_until_anHC06_IsDetected() {
  while (fbool_anHC06_isNotYetDetected()) {
    dry_read_all_serial_inputs();  //to allow for a fresh start before operation
    execute_ScanForHC06();
    }  
}

//AT+BAUD1(to HC-06), returning OK1200(to 1382400)
void execute_SetHC06BaudRateEntry() {
  bool tmp_bool=false;
  int tmp_uint_entry;
  unsigned long tmp_ulong_DesiredHC06BaudRate;
  char tmp_char;

  set_False_bool_return_to_MainMenu(); //stay until Exit was chosen
  if (fbool_ReceivedUserEntry_with_TrailingCRLF()) {
    while(true) {  
      if (!fbool_UserMenuChoice_is_single_in_range(0, const_uint_maxHC06BaudRateIndex-1))
        break;

      tmp_bool = true;  //valid entry         
      tmp_uint_entry = fuint_get_valid_entry();     
     
      if (tmp_uint_entry == const_uintMenuChoiceToExitToMain) //exit to main menu 
        set_True_bool_return_to_MainMenu();
      else {
        strcpy(archarCommandToHC06, "AT+BAUD");
        tmp_char = fchar_get_valid_entry();
        strncat(archarCommandToHC06, &tmp_char,1);       
        if (fbool_send_Command_to_HC06_then_check_response()) { 
          if (strncmp(archarReceivedSerialInputData, "OK", 2) != 0) //???3/4/2021
            Serial.println(F("Error: Invalid response(did not start with \"OK\")"));
          else { 
            tmp_ulong_DesiredHC06BaudRate = fulong_get_BaudRateFromHC06Array(tmp_uint_entry);            
            sprintf(arChar_sprintf, "%ld", tmp_ulong_DesiredHC06BaudRate);
            if (strcmp((archarReceivedSerialInputData + 2), arChar_sprintf) != 0)
              Serial.println(F("Error: Responding baud rate did not match!"));
            else {
              Serial.println(F("Successfully set HC-06 Baud Rate!"));
              set_uintInterfacingHC06_BaudRateIndex(tmp_uint_entry);
              copy_InterfacingHC06BaudRateIndex_to_HC06BaudRateIndex(); //will be used if setBaudRate command was executed successfully
              switch_InterfacingH06_SerialParameters(); //once new buad rate was set, change interfacing parameter
              }
            }
        }
      else 
        Serial.println(F("Error: failed to execute setBaudRate command!"));  
      }  
      break;   
    }  
    if (!tmp_bool)            
      Serial.println("Error: Invalid entry!");//entry not single character and/or out of range
  }     
}

//list avaiable baud rate then get choice from user, then execute
void execute_SetHC06BaudRate() { 
  while(true) {
    display_SetHC06BaudRateChoices();
    execute_SetHC06BaudRateEntry(); //once successful, change Interfacing baud rate accordingly
    if (fbool_return_to_MainMenu())
      break; 
    }   
}

//Arduino sends AT+NAMEname (name is 20 characters or less)
//expected return: OKsetname
void execute_SetHC06Name() {
  unsigned int tmp_uint_ReceivedSerialInputDataCount;
  
  Serial.print(F("Enter New Name for the HC-06(max: 20 characters): "));
  while(true) {
    if (!fbool_ReceivedUserEntry_with_TrailingCRLF()) //any error would be displayed already
      break;

    tmp_uint_ReceivedSerialInputDataCount = fuint_get_ReceivedSerialInputDataCount(); //with trailing CRLF
    if (tmp_uint_ReceivedSerialInputDataCount > (constMaxCharSetHC06Name+2)) {
      Serial.println(F("Error: Entry exceeds max 20 characters!"));
      break;
      }
      
    strcpy(archarCommandToHC06, "AT+NAME");
    strcat(archarCommandToHC06, archarReceivedSerialInputData);
    if (fbool_send_Command_to_HC06_then_check_response()) { 
      if (strcmp(archarReceivedSerialInputData, "OKsetname") == 0) //???3/4/2021
        Serial.println(F("Successfully set HC-06 name"));   
      else        
        Serial.println(F("Error: Incorrect Response from HC-06, suggest a re-scan"));
        }
    else
      Serial.println(F("Error: Failed to execute setName command!")); 
          
      break;    
    }         
}

void execute_SetHC06PairingPIN() {
  unsigned int tmp_uint_ReceivedSerialInputDataCount;
  unsigned int tmp_uint_index;
  bool tmp_bool_4digit_qualification = false;
  char tmp_char_ReceivedSerialInputData;
  
  Serial.print(F("Enter new Pairing PIN(exactly 4 digits): "));
  while(true) {
    if (!fbool_ReceivedUserEntry_with_TrailingCRLF()) //any error would be displayed already
      break;

    tmp_uint_ReceivedSerialInputDataCount = fuint_get_ReceivedSerialInputDataCount(); //with trailing CRLF
    if (tmp_uint_ReceivedSerialInputDataCount != 4)
      break;
      
    for (tmp_uint_index=0; tmp_uint_index < 4; tmp_uint_index ++) {
      tmp_char_ReceivedSerialInputData = fchar_get_ReceivedSerialInputData_byIndex(tmp_uint_index);
      if (!fbool_char_is_a_digit(tmp_char_ReceivedSerialInputData))
        break;
      }

      if (tmp_uint_index == 4)
        tmp_bool_4digit_qualification = true;
        
      if (!tmp_bool_4digit_qualification)
        break;
             
      strcpy(archarCommandToHC06, "AT+PIN");
      strcat(archarCommandToHC06, archarReceivedSerialInputData);
      if (fbool_send_Command_to_HC06_then_check_response()) { 
        if (strcmp(archarReceivedSerialInputData, "OKsetPIN") == 0)
          Serial.println("Successfully set HC06 pairing PIN");   
        else        
          Serial.println("Error: Incorrect Response, suggest a re-scan");
          }
      else
        Serial.println("Error: Failed to receive command response!");    
      break;    
    }

 if (!tmp_bool_4digit_qualification)
   Serial.println(F("Error: Entry has to be exactly 4 digits!"));               
}


void display_SetHC06ParityChoices() {
  display_return_to_MainMenu();
  Serial.println(F("1. Parity None"));
  Serial.println(F("2. Parity Even"));
  Serial.println(F("3. Parity Odd")); 
  Serial.print(F("Please enter a choice: "));    
}

void execute_SetHC06ParityEntry() {
  bool tmp_bool=false;
  int tmp_uint_entry;
  char tmp_char;

  set_False_bool_return_to_MainMenu(); //stay until Exit was chosen
  if (fbool_ReceivedUserEntry_with_TrailingCRLF()) {
    while(true) {  
      if (!fbool_UserMenuChoice_is_single_in_range(0, const_uint_maxSetParityChoiceIndex))
        break;

      tmp_bool = true;  //valid entry         
      tmp_uint_entry = fuint_get_valid_entry();     

      if (tmp_uint_entry == const_uintMenuChoiceToExitToMain) 
        set_True_bool_return_to_MainMenu();
      else {  
        strcpy(archarCommandToHC06, "AT+P");
        switch (tmp_uint_entry) {
          case const_uintSubmenuChoiceSetParityToNone:
            strcat(archarCommandToHC06, "N");
            break;
          case const_uintSubmenuChoiceSetParityToEven:
            strcat(archarCommandToHC06, "E");
            break;
          case const_uintSubmenuChoiceSetParityToOdd:
            strcat(archarCommandToHC06, "O");
            break;                        
            }             
        if (fbool_send_Command_to_HC06_then_check_response()) { 
          strcpy(archarCommandToHC06, "OK "); 
          switch (tmp_uint_entry) {
            case const_uintSubmenuChoiceSetParityToNone:
              strcat(archarCommandToHC06, "None");
              break;
            case const_uintSubmenuChoiceSetParityToEven:
              strcat(archarCommandToHC06, "Even");
              break;
            case const_uintSubmenuChoiceSetParityToOdd:
              strcat(archarCommandToHC06, "Odd");
              break;                        
              } 
                       
          if (strcmp(archarReceivedSerialInputData, archarCommandToHC06) == 0) { 
            Serial.print(F("Successfully set HC-06 to parity "));
            Serial.println(archarCommandToHC06+3); //Skip the leading "OK "
            Serial.println(F("(power cycling is required for configured parity to take effect!)\n"));
            set_uintHC06ParityUponPowerCycling(tmp_uint_entry);
            }
          else
            Serial.println(F("Error: Incorrect response from HC-06!"));  
        }
      else
        Serial.println(F("Error: Failed to execute setParity command!"));  
      }  
      break;   
    }  
    if (!tmp_bool)            
      Serial.println(F("Invalid entry!"));//entry not single character and/or out of range
  }  
}

void execute_SetHC06Parity() {
  while(true) {
    display_SetHC06ParityChoices();
    execute_SetHC06ParityEntry(); //once successful, change Interfacing baud rate accordingly
    if (fbool_return_to_MainMenu())
      break; 
    }   
}

void diplay_ReceivedSerialInputData() {
  Serial.println(archarReceivedSerialInputData);
}

//Arduino sends "AT+VERSION" to HC-06 and expects version info in return
void execute_GetHC06Version() {
  Serial.print(F("Version of HC-06: ")); 
  strcpy(archarCommandToHC06, "AT+VERSION"); 
  if (fbool_send_Command_to_HC06_then_check_response()) 
    diplay_ReceivedSerialInputData();
  else
    Serial.println(F("Failed to execute getVersion command!"));  
}

void execute_TurnHC06LED_OnOffEntry() {
  bool tmp_bool=false;
  int tmp_uint_entry;
  char tmp_char;

  set_False_bool_return_to_MainMenu(); //stay until Exit was chosen
  if (fbool_ReceivedUserEntry_with_TrailingCRLF()) {
    while(true) {  
      if (!fbool_UserMenuChoice_is_single_in_range(0, const_uint_maxTurnLED_OnOffChoiceIndex))
        break;

      tmp_bool = true;  //valid entry         
      tmp_uint_entry = fuint_get_valid_entry();     

      if (tmp_uint_entry == const_uintMenuChoiceToExitToMain) {
        set_True_bool_return_to_MainMenu();
        break;        
      }

      else {  
        strcpy(archarCommandToHC06, "AT+LED");
        sprintf(arChar_sprintf, "%d", (tmp_uint_entry-1));        
        strcat(archarCommandToHC06, arChar_sprintf);      
        if (fbool_send_Command_to_HC06_then_check_response()) { 
          strcpy(archarCommandToHC06, "LED ");          
          if (tmp_uint_entry == const_uintSubmenuChoiceTurnOnLED) 
            strcat(archarCommandToHC06, "ON"); 
          else
            strcat(archarCommandToHC06, "OFF"); 
                       
          if (strcmp(archarReceivedSerialInputData, archarCommandToHC06) == 0) {
            Serial.print(F("Successfully turned LED "));
            if (tmp_uint_entry == const_uintSubmenuChoiceTurnOnLED) 
              Serial.println(F("ON")); 
            else
              Serial.println(F("OFF")); 
            Serial.println();  
            }
          else
            Serial.println(F("Error: Unrecognized reponse, failed to set LED"));
        }
      else
        Serial.println(F("Failed to execute setLED command"));  
      }  
      break;   
    }  
    if (!tmp_bool)            
      Serial.println("Invalid entry!");//entry not single character and/or out of range
  }
}   

void display_TurnHC06LED_OnOffChoices() {
  display_return_to_MainMenu();
  Serial.println(F("1. Turn LED OFF"));
  Serial.println(F("2. Turn LED ON"));
  Serial.print(F("Please enter a choice: "));  
}

void execute_TurnLED_OnOFF() {
  while(true) {
    display_TurnHC06LED_OnOffChoices();
    execute_TurnHC06LED_OnOffEntry(); //once successful, change Interfacing baud rate accordingly
    if (fbool_return_to_MainMenu())
      break; 
    }   
}

void execute_SetSlaveMasterModeEntry() {
  bool tmp_bool=false;
  int tmp_uint_entry;
  char tmp_char;

  set_False_bool_return_to_MainMenu(); //stay until Exit was chosen
  if (fbool_ReceivedUserEntry_with_TrailingCRLF()) {
    while(true) {  
      if (!fbool_UserMenuChoice_is_single_in_range(0, const_uint_maxSetSlaveMasterModeChoiceIndex))
        break;

      tmp_bool = true;  //valid entry         
      tmp_uint_entry = fuint_get_valid_entry();     

      if (tmp_uint_entry == const_uintMenuChoiceToExitToMain) {
        set_True_bool_return_to_MainMenu();
        break;        
      }

      else {  
        strcpy(archarCommandToHC06, "AT+ROLE=");
        if (tmp_uint_entry == const_uintSubmenuChoiceSetSlaveMode)   
          strcat(archarCommandToHC06, "S");
        else
          strcat(archarCommandToHC06, "M");  
              
        if (fbool_send_Command_to_HC06_then_check_response()) { 
          strcpy(archarCommandToHC06, "OK+ROLE:");          
          if (tmp_uint_entry == const_uintSubmenuChoiceSetSlaveMode) 
            strcat(archarCommandToHC06, "S"); 
          else
            strcat(archarCommandToHC06, "M"); 
                       
          if (strcmp(archarReceivedSerialInputData, archarCommandToHC06) == 0) {
            Serial.print(F("Successfully set Slave/Master Mode to "));
            if (tmp_uint_entry == const_uintSubmenuChoiceSetSlaveMode) 
              Serial.println(F("Slave")); 
            else
              Serial.println(F("Master")); 
            Serial.println();  
            }
          else
            Serial.println(F("Error: Unrecognized reponse, failed to set Slave/Master Mode"));
        }
      else
        Serial.println(F("Failed to execute setSlaveMasterMode command"));  
      }  
      break;   
    }  
    if (!tmp_bool)            
      Serial.println("Invalid entry!");//entry not single character and/or out of range
  }
}  

void display_SetSlaveMasterModeChoices() {
  display_return_to_MainMenu();
  Serial.println(F("1. Set HC-06 to Slave Mode"));
  Serial.println(F("2. Set HC-06 to Master Mode"));
  Serial.print(F("Please enter a choice: "));  
}

void execute_SetHC06SlaveMasterMode() {
  while(true) {
    display_SetSlaveMasterModeChoices();
    execute_SetSlaveMasterModeEntry(); //once successful, change Interfacing baud rate accordingly
    if (fbool_return_to_MainMenu())
      break; 
    }    
}

void execute_MainMenuEntry() {
  while(true) {
    if (!fbool_ReceivedUserEntry_with_TrailingCRLF()) //any erros would be displayed already
      break;
      
    if (!fbool_UserMenuChoice_is_single_in_range(constMainMenuChoiceMin, constMainMenuChoiceMax))
      break; 
         
    switch(fuint_get_valid_entry()) {
      case constMainMenuChoiceScanForHC06:
        set_HC06SerialConfiguration_toUnknown();
        scan_until_anHC06_IsDetected();
        break;                        
      case constMainMenuChoiceSetHC06BaudRate:
        execute_SetHC06BaudRate();
        break;              
      case constMainMenuChoiceSetHC06Name:
        execute_SetHC06Name();
        break;               
      case constMainMenuChoiceSetHC06PairingPIN:
        execute_SetHC06PairingPIN();
        break;
      case constMainMenuChoiceSetHC06Parity:
        execute_SetHC06Parity();
        break;
      case constMainMenuChoiceGetHC06Version:
        execute_GetHC06Version();
        break;  
      case constMainMenuChoiceTurnHC06LED_OnOff:
        execute_TurnLED_OnOFF();
        break;  
      case constMainMenuChoiceSetHC06SlaveMasterMode:
        execute_SetHC06SlaveMasterMode();
        break;                                                    
      } //end bracket for switch (
   break;   
  } //end bracket for while (true)    
}

void display_MainMenuChoices() {
 display_HC06BaudRate();   
 Serial.print(F("Baud Rate Interfacing with HC-06: "));
 Serial.println(fulong_get_InterfacingHC06_BaudRate());
 display_HC06Parity();
 display_InterfacingParity();
 Serial.println(F("1. Scan for HC-06"));
 Serial.println(F("2. Set HC-06 Baud Rate")); //2/19/2021, Wavesen example did not change interfacing baud rate      
 Serial.println(F("3. Set HC-06 Name"));  
 Serial.println(F("4. Set HC-06 pairing PIN"));     
 Serial.println(F("5. Set HC-06 Parity")); 
 Serial.println(F("6. Get HC-06 Version")); 
 Serial.println(F("7. Turn LED ON/OFF"));                 
 Serial.println(F("8. Set HC-06 to Slave/Master Mode"));  
 Serial.print(F("Please enter a choice: "));      
}

void loop() {
  dry_read_all_serial_inputs();  //to allow for a fresh start before operation 
  display_MainMenuChoices();              
  execute_MainMenuEntry();
}

void waiting_for_initial_YesHC_06() {
  while (true) {
    Serial.print(F("Please enter keyword to proceed: "));
    if (fbool_ReceivedUserEntry_with_TrailingCRLF()) {      
      if (strcmp(archarReceivedSerialInputData, "YesHC-06") == 0)
        break;
    }
  }
}

void setup() {
  Serial.begin(9600); 
  delay(500); //giving Serial port time to be ready
  
  Serial.println(F("\nDetect and program/configure HC-06 (Rev. 2)")); 
  Serial.println(F("This sketch is highly experimental")); 
  Serial.println(F("It will not work on all HC-06 modules"));   
  Serial.println(F("Please observe all safety precautions")); 
  Serial.println(F("Avoid setting long name at higher baud rate")); 
  Serial.println(F("setLED and setSlaveMaster might have no effects"));       
  Serial.println(F("by formerisan, 15:54, 3/16/2021"));
  Serial.println();
  
  waiting_for_initial_YesHC_06();   
  
  scan_until_anHC06_IsDetected();
}
