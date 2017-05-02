/*
 *  BLE Handler 
 *  parses all BLE messages
 *  manages connection
 * 
 * 
 */

#ifndef BLEHandler_H
#define BLEHandler_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "configurator.h"
#include "ErrorHandler.h"
#include <WebSocketsClient.h>

extern SoftwareSerial ble;


//forward declarations
//-------------------
class ErrorHandler;
//-------------------
extern WebSocketsClient webSocket;


enum Bauds {
  BAUD0=9600,
  BAUD1=19200,
  BAUD2=38400,
  BAUD3=57600,
  BAUD4=115200
};


#define MIN_REQUEST_PERIOD (0)

#define DELIMITER_CHAR (';')
#define DATA_CHAR ('>')
#define CONFIG_REQUEST_CHAR ('?')
#define CONFIG_SET_CHAR ('*')
#define CONNECTION_PONG_CHAR ('@')
#define PING_CHAR ('!')
#define DELIMITER_CHAR (';')
#define RESEND_CHAR ('^')



enum HandlerStates {
  NO_CONNECTION,
  IDENTIFY_PACKET,
  DATA_PACKET,
  CONNECTION_PONG,
  CONFIG_REQUEST,
  ERROR_PACKET
};


class BLEHandler {
  public:
    BLEHandler(Schema *d);
    ~BLEHandler();
    ErrorHandler * errorHandler;
    Schema * data;
    void begin();
    bool connect();
    bool connected=false;
    bool monitor();    //true on successful data receive
    void send(char requestType, unsigned int numFields, char* sendFields[]);
    void send(char requestType, char* sendField);
    void send(char requestType, unsigned long sendField);
    bool recovered();
    bool autoBaud();
  private:
    bool lostAndRecovered=false;
    bool typeDetect();
    void noConnectionHandler();
    void connectionPongHandler();
    bool dataPacketHandler();
    void configRequestHandler();
    bool BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput=true);
    bool isReady();    
    bool packetTerminate();
    unsigned long currentBaud;
    HandlerStates handlerState = NO_CONNECTION;
    unsigned int fieldIndex=0;
    unsigned int stringIndex=0;
};

void requestResendPacket(unsigned long packetIDRequest);

#endif //BLEHandler_H
