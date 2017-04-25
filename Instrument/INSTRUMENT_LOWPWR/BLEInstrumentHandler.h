#ifndef BLEInstrumentHandler_H
#define BLEInstrumentHandler_H

#include <Arduino.h>
#include "configuration.h"
#include "messageStore.h"
#include "utils.h"


class Configuration;


#define USE_SERIAL Serial1

#define MIN_REQUEST_PERIOD (0)
#define CONFIG_REQUEST_INTERVAL (3000)
#define BAUD_RATE baud4

#define DELIMITER_CHAR (';')
#define DATA_CHAR ('>')
#define CONFIG_REQUEST_CHAR ('?')
#define CONFIG_SET_CHAR ('*')
#define CONNECTION_PONG_CHAR ('@')
#define CONNECTION_PING_CHAR ('!')
#define RESEND_REQUEST_CHAR ('^')

enum ErrorDetection {
  NUMERICAL_DETECT,
  FLOAT_DETECT
};

//=========================================================================================


enum HandlerStates {
  IDENTIFY_PACKET,
  NO_CONFIG,
  NEW_CONFIG,
  CONNECTION_PING,
  RESEND_REQUEST
};

enum ConfigFields {
  ID_CONFIG_FIELD,
  TIME_CONFIG_FIELD,
  SCHEMA_CONFIG_FIELD
};


enum Bauds {
  baud0=9600,
  baud1=19200,
  baud2=38400,
  baud3=57600,
  baud4=115200
};


class BLEInstrumentHandler {
  public:
    Configuration * configuration;
    RecentMessages store;
    BLEInstrumentHandler(Configuration * c) : configuration(c) {}
    void begin();
    void initialise();
    bool connect();
    bool monitor();    //true on successful data receive
    void send(char requestType, unsigned int numFields, char* sendFields[]);
    void send(char requestType, char* sendField);
    void send(char requestType, unsigned long sendField);
    bool autoBaud(int delaylen=500);
  private:
    bool typeDetect();
    void noConfigHandler();
    void connectionPingHandler();
    void resendRequestHandler();
    bool configPacketHandler();
    bool BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput=true);
    bool isReady();
    void breakConnection();    
    bool packetTerminate();
    unsigned long currentBaud;
    HandlerStates handlerState = NO_CONFIG;
};

#endif //BLEInstrumentHandler_H
