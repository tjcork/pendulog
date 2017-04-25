#include "DataHandler.h"


void DataHandler::push(Schema &data) {
  static char * p=ref;
  if (packetCount < CHUNK_SIZE - 1) {
    p=writeDataPacket(p,data);
    p=faststrcat(p,",");
    packetCount++;
  }
  else {    //last packet
    p=writeDataPacket(p,data);
    p=faststrcat(p,"],\"numChunks\":\"");
    p=faststrcat(p,CHUNK_SIZE_STR);
    p=faststrcat(p,"\"}");
    packetCount=0;
    ws->sendTXT(outMessage);
    *ref='\0';
    p=ref;
  }
}

char * DataHandler::writeDataPacket(char* p, Schema &data) {
  p=faststrcat(p,"{");
  int i;
  for(i = 0; i < data.fieldCount-1; i++) {
    p=faststrcat(p,"\"");
    p=faststrcat(p,data.getField(i)->name);
    p=faststrcat(p,"\":\"");
    p=faststrcat(p,data.getField(i)->string_value);
    p=faststrcat(p,"\",");
  }
  p=faststrcat(p,"\"");
  p=faststrcat(p,data.getField(i)->name);
  p=faststrcat(p,"\":\"");
  p=faststrcat(p,data.getField(i)->string_value);
  p=faststrcat(p,"\"}");
  return p;
}

/*

void DataHandler::writeDataPacket(Schema &data) {
  Serial.println(micros());
  unsigned int lastIndex=data.fieldCount-1;
  outMessage += "{";
  for(int i = 0; i < lastIndex; i++) {

    
    outMessage += "\"" + String(data.getField(i)->name) + "\":\"" + String(data.getField(i)->string_value) + "\",";
  }
  outMessage += "\"" + String(data.getField(lastIndex)->name) + "\":\"" + String(data.getField(lastIndex)->string_value) + "\"}";  
  Serial.println(micros());
}
*/

