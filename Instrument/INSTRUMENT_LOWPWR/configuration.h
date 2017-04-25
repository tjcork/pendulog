#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <arduino.h>
#include "utils.h"
#include "mpu.h"

#define TIME_SIZE (21)
#define ID_SIZE (11)
#define SCHEMA_SIZE (20)

enum ConfigPacketFormat {
  CONFIG_TYPE_INDEX=0,
  CONFIG_ID_INDEX=1,
  CONFIG_ID_SIZE=11,
  CONFIG_TIME_INDEX=2,
  CONFIG_TIME_SIZE=21,
  CONFIG_PARAMETERS=3
};

enum RequestPacket {
  RESEND_INDEX=1,
  REQUEST_PARAMETERS=2
};


typedef enum FieldIdentifiers {
  ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, MX, MY, MZ, GX, GY, GZ
};


class Configuration {
  public:
    FieldIdentifiers fields[18];
    float * dataPointers[16];
    int fieldCount=0;
    char ID_str[CONFIG_ID_SIZE];
    char time_str[CONFIG_TIME_SIZE];
    unsigned long currentID = 0;
    unsigned long long receivedTime;
    unsigned long timeOffset=0;
    bool isSet=false;
    bool timeSet=false;
    Configuration() {}
    void setID(const char* id);
    void setTIME(const char* time);
    void setSCHEMA();
    unsigned long long currentTime();
    char* currentTime_str();
    char* currentID_str();
    void incrementID();
};

#endif //CONFIGURATION_H
