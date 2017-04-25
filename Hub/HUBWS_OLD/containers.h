#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <ArduinoJson.h>
#include "utils.h"
#include "BLE.h"


#define ID_SIZE (11)
#define TIME_SIZE (21)
#define YAW_SIZE (9)
#define PITCH_SIZE (9)
#define ROLL_SIZE (9)

#define RECALL_QUEUE_LENGTH (10)
#define CHUNK_SIZE (10)

//      Formats
//===============================================================================

enum PacketType : unsigned byte {
  PACKET_TYPE_INDEX=0,
  ERROR_PACKET,
  DATA_PACKET,
  CONNECTION_PACKET,
  RESPONSE_PACKET
};

enum ConnectionPacketFormat {
  CONNECTION_FIELDS_COUNT=0
};


enum DataPacketFormat {
  ID_INDEX = 1,
  TIME_INDEX,
  YAW_INDEX,
  PITCH_INDEX,
  ROLL_INDEX,
  DATA_FIELDS_COUNT=5
};

enum ConfigPacketFormat {
  CONFIG_TYPE_INDEX=0,
  CONFIG_ID_INDEX=1,
  CONFIG_ID_SIZE=11,
  CONFIG_TIME_INDEX=2,
  CONFIG_TIME_SIZE=22,
  CONFIG_PARAMETERS=3
};

//===============================================================================

class Configuration {
  public:
  Configuration() {}
  unsigned long initialID;
  bool configReceived=false;
  bool timeSet=false;
  bool idSet=false;
  char time_str[CONFIG_TIME_SIZE];
  unsigned long long receivedTime;
  unsigned long timeOffset=0;
  unsigned long long currentTime();
  char* currentTime_str();
  void send();  
};

extern Configuration config;

//===============================================================================

class LostPacket {
  public:
    unsigned long ID;
    unsigned long lostID;
    LostPacket() {}
    LostPacket(unsigned long I);
    bool isFound(unsigned long matchID);
    bool isLost(unsigned long matchID);
};

//===============================================================================

class PacketRecovery {
  public: 
    PacketRecovery() {}
    LostPacket queue[RECALL_QUEUE_LENGTH];
    int count=0;
    void remove(int index);
    bool recover(unsigned long thisID);
    void addMissing(unsigned long missingID);
};

//===============================================================================

class DataPacket {
  public:
    char ID[ID_SIZE];
    char Time[TIME_SIZE];
    char Yaw[YAW_SIZE];
    char Pitch[PITCH_SIZE];
    char Roll[ROLL_SIZE];
    DataPacket() {}
    DataPacket(char* IDStr, char* timeStr, char* yawStr, char* pitchStr, char* rollStr);
    void setID(const char* IDStr);
    void setTimeStamp(const char* timeStr);
    void setYaw(const char* yawStr);
    void setPitch(const char* pitchStr);
    void setRoll(const char* rollStr);
    unsigned long getID();
    DataPacket & operator=(const DataPacket &rhs);
};

//===============================================================================

class DataChunk {
  public:
    DataChunk() {}
    DataPacket chunk[CHUNK_SIZE];
    char chunkJSON[CHUNK_SIZE*100+250];
    int index=0;
    bool addData(DataPacket d);
    bool isFull();
    bool empty();
    char* toJSON();
};

//===============================================================================

#endif
