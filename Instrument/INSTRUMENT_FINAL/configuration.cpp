#include "configuration.h"

   
void Configuration::setID(const char* id) {
  strncpy(ID_str,id,CONFIG_ID_SIZE);
  currentID=atoul(ID_str);
  Serial.print("ID set:   ");Serial.println(ID_str);
}

void Configuration::setTIME(const char* time) {
  strncpy(time_str, time, CONFIG_TIME_SIZE);
  receivedTime=atoull(time_str);
  Serial.print("Time received:  ");Serial.println(time_str);
  timeSet=true;
}

void Configuration::setSCHEMA() {
  Serial.println("SCHEMAS ARE:  ");
  for(int i = 0; i < fieldCount-2; i++) {
    Serial.print(fields[i+2]);
    //ignore time and id parameters.
    //currently forced to position 0 and position 1
    switch(fields[i+2]) {
      case YW:
        dataPointers[i]=&(mympu.YW);
        break;
      case PT:
        dataPointers[i]=&(mympu.PT);
        break;
      case RL:
        dataPointers[i]=&(mympu.RL);
        break;
      case QW:
        dataPointers[i]=&mympu.QW;
        break;
      case QX:
        dataPointers[i]=&mympu.QX;
        break;
      case QY:
        dataPointers[i]=&mympu.QY;
        break;
      case QZ:
        dataPointers[i]=&mympu.QZ;
        break;
      case AX:
        dataPointers[i]=&mympu.AX;
        break;
      case AY:
        dataPointers[i]=&mympu.AY;
        break;
      case AZ:
        dataPointers[i]=&(mympu.AZ);
        break;
      case MX:
        dataPointers[i]=&mympu.MX;
        break;
      case MY:
        dataPointers[i]=&mympu.MY;
        break;
      case MZ:
        dataPointers[i]=&mympu.MZ;
        break;
      case GX:
        dataPointers[i]=&mympu.GX;
        break;
      case GY:
        dataPointers[i]=&mympu.GY;
        break;
      case GZ:
        dataPointers[i]=&mympu.GZ;
        break;
      default:
        Serial.print("UNKNOWN FIELD");
    }
  }
 return;
}

unsigned long long Configuration::currentTime() {
  if (timeSet)
    return receivedTime+millis()-timeOffset;
  else
    return millis();
}

char* Configuration::currentTime_str() {
  str_ll(currentTime(),time_str);
  return time_str;
}

char* Configuration::currentID_str() {
  return ID_str;
}

void Configuration::incrementID() {
  currentID++;
}

