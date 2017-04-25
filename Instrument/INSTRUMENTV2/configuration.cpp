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

void Configuration::setSCHEMA(const char* schema) {

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

