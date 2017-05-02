/*
 * Configuration class
 * details each data field and its handling
 * 
 */

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <ArduinoJson.h>
#include "utils.h"
#include "BLEHandler.h"


#define ID_SIZE (11)
#define TIME_SIZE (21)
#define YAW_SIZE (9)
#define PITCH_SIZE (9)
#define ROLL_SIZE (9)

#define RECALL_QUEUE_LENGTH (10)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define CHUNK_SIZE 10
#define CHUNK_SIZE_STR (const char *) STR(CHUNK_SIZE)

//      Formats
//===============================================================================

enum ConnectionPacketFormat {
  CONNECTION_FIELDS_COUNT=0
};

enum ConfigPacketFormat {
  CONFIG_TYPE_INDEX=0,
  CONFIG_ID_INDEX=1,
  CONFIG_ID_SIZE=11,
  CONFIG_TIME_INDEX=2,
  CONFIG_TIME_SIZE=22,
  CONFIG_SCHEMA_INDEX=3,
  CONFIG_SCHEMA_SIZE=50,
  CONFIG_PARAMETERS=4
};


enum ErrorDetection {
  NUMERICAL_DETECT,
  FLOAT_DETECT
};


typedef enum FieldIdentifiers {
  ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, MX, MY, MZ, GX, GY, GZ
};

class FieldsFormat {
  FieldIdentifiers * format;
  int count;
  FieldsFormat(int num, const FieldIdentifiers ff[]) {
    format = new FieldIdentifiers [num];
    for (int i = 0; i < num; i++) {
      format[i]=ff[i];
    }
    count=num;
  }
  ~FieldsFormat() {
    delete format;
  }
    FieldsFormat& operator=(const FieldsFormat &rhs) {
      delete format;
      format = new FieldIdentifiers[rhs.count];
      count = rhs.count;
      return *this;
    }
};




class Field {
  public:
    char name[3];
    unsigned int identifier;
    size_t size;
    char * string_value;
    ErrorDetection errorDetection;
    Field(const char * shortname, FieldIdentifiers fieldid, size_t stringsize = 9, ErrorDetection type = FLOAT_DETECT) {
      strncpy(name,shortname,3);
      identifier=fieldid;
      size=stringsize;
      string_value=new char[size];
      errorDetection=type;
    }
    ~Field() {
      delete [] string_value;
    }  
};

//=========================================================================================

class Schema {
  public:
  Field fields[18] = { Field("ID",ID,11,NUMERICAL_DETECT),
                       Field("TM",TI,21,NUMERICAL_DETECT),
                       Field("YW",YW,9),
                       Field("PT",PT,9),
                       Field("RL",RL,9),
                       Field("QW",QW,9),
                       Field("QX",QX,9),
                       Field("QY",QY,9),
                       Field("QZ",QZ,9),
                       Field("AX",AX,12),
                       Field("AY",AY,12),
                       Field("AZ",AZ,12),
                       Field("MX",MX,9),
                       Field("MY",MY,9),
                       Field("MZ",MZ,9),
                       Field("GX",GX,9),
                       Field("GY",GY,9),
                       Field("GZ",GZ,9) };
  FieldIdentifiers spec[18];
  unsigned int fieldCount;
  Schema(unsigned int number, const FieldIdentifiers format[]) {
    fieldCount=number < 18 ? number : 18;
    for(int i=0; i < fieldCount or i < 18; i++)
      spec[i]=format[i];
  }
  void setSchema(unsigned int number, const FieldIdentifiers format[]) {
    fieldCount=number < 18 ? number : 18;
    for(int i=0; i < fieldCount or i < 18; i++) 
      spec[i]=format[i];
  }
  Field * getField(int index) {
    return &(fields[spec[index]]);
  }
};

//===============================================================================


class Configuration {
  public:

  unsigned long initialID;
  Schema * format;
  bool configReceived=false;
  bool timeSet=false;
  bool idSet=false;
  char time_str[CONFIG_TIME_SIZE];
  unsigned long long receivedTime;
  unsigned long timeOffset=0;
  unsigned long long currentTime();
  char* currentTime_str();
  void send();  
  Configuration(Schema * s) : format(s) {}
};

extern Configuration config;

#endif

