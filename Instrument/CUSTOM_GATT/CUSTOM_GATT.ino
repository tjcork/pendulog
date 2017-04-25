/*

AT+GATTADDSERVICE=UUID128=00-00-ff-e0-00-00-10-00-80-00-00-80-5f-9b-34-fb
AT+GATTADDCHAR=UUID=0xFFE1,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=20,DESCRIPTION=HMSoft
AT+GATTADDCHAR=UUID=0xFFE1,PROPERTIES=0x04,MIN_LEN=1,MAX_LEN=20,DESCRIPTION=HMSoft
AT+GATTADDCHAR=UUID128=00-00-29-02-00-00-10-00-80-00-00-80-5f-9b-34-fb,PROPERTIES=0x04,MIN_LEN=1,MAX_LEN=20,VALUE=0x1000
AT+GATTADDCHAR=UUID128=00-00-28-03-00-00-10-00-80-00-00-80-5f-9b-34-fb,PROPERTIES=0x04,MIN_LEN=1,MAX_LEN=20,VALUE=0x14
0x2803
AT+GAPSETADVDATA=02-01-06-03-02-e0-ff


 */


#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif
 
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BluefruitLE_SPI.h"

#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused


#define BLUEFRUIT_UART_MODE_PIN        12    // we won't be changing modes
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output
 
// Create the bluefruit object
 
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

 
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
 
// The service and characteristic index information 
int32_t gattServiceId;
int32_t gattNotifiableCharId;
int32_t gattWritableResponseCharId;
int32_t gattWritableNoResponseCharId;
int32_t gattReadableCharId;
 
void setup(void)
{
  //remove these 2 lines if not debugging - nothing will start until you open the serial window
  while (!Serial); // required for Flora & Micro
  delay(500);
 
  boolean success;
 
  Serial.begin(115200);
  Serial.println(F("Adafruit Custom GATT Service Example"));
  Serial.println(F("---------------------------------------------------"));
 
  randomSeed(micros());
 
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
 
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );
 
  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }
 
  /* Disable command echo from Bluefruit */
  ble.echo(false);
 
  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
 
  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  //ble.setInterCharWriteDelay(5); // 5 ms
 
 
  /* Add the Custom GATT Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Custom GATT Service definition: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=00-00-ff-e0-00-00-10-00-80-00-00-80-5f-9b-34-fb"), &gattServiceId);
  if (! success) {
    error(F("Could not add Custom GATT service"));
  }
 

  Serial.println(F("Adding the Notifiable characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=00-00-ff-e1-00-00-10-00-80-00-00-80-5f-9b-34-fb,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=20"), &gattNotifiableCharId);
    if (! success) {
    error(F("Could not add Custom Notifiable characteristic"));
  }
  
  /* Add the Custom GATT Service to the advertising data */
  //0x1312 from AT+GATTLIST - 16 bit svc id
  Serial.print(F("Adding Custom GATT Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-03-02-e0-ff") );
 
  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();
 
  Serial.println();
}
 
void loop(void)
{
 
}
