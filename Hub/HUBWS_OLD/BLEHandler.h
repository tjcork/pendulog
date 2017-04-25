#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "containers.h"

#define BUFFER_LENGTH (100)
#define BLE_MAX_PACKET_SIZE (100)

#define MIN_REQUEST_PERIOD (0)

#define PACKET_DELIMITER (';')
#define CONNECTED_CHAR ('!')
#define CONNECTED_CHAR_S ("!")
#define RESPONSE_CHAR ('\a')
#define RESEND_CHAR ('R')
#define CONFIG_CHAR ('\a')
#define DATA_CHAR ('D')
#define CONNECTION_CHAR ('!')

#define CONFIG_REQUEST ("\a")
#define CONFIG_REQUEST_C ('\a')

extern SoftwareSerial ble;

//forward declarations
//-------------------
class DataPacket;   
enum PacketType : unsigned byte;
//-------------------


enum Bauds {
  BAUD0=9600,
  BAUD1=19200,
  BAUD2=38400,
  BAUD3=57600,
  BAUD4=115200
};

bool BLEAvailable();

bool BLEAutoBaud();

bool BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput = true);

bool BLEIsReady();

bool BLEInit();

void BLERequestConnection();

bool BLEIsConnected();
//bool BLEIsConnected(bool connWait=false,int recursionCount=0);

bool parseBLE(PacketType &type, DataPacket& data, bool &midPacket);

void requestResendPacket(unsigned long packetIDRequest);

#endif
