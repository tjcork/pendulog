#ifndef DataHandler_H
#define DataHandler_H

#include "BLEHandler.h"
#include "utils.h"
#include <WebSocketsClient.h>


class DataHandler {
  public:
    DataHandler(WebSocketsClient * w) : ws(w) {
      outMessage[0] = '\0';
      ref=faststrcat(ref,"{\"type\":\"chunk\",\"data\":[");   
     }
     void push(Schema &data);
  private:
    WebSocketsClient * ws;
    char * writeDataPacket(char* p, Schema &data);
    unsigned int packetCount=0;
    char outMessage[4000];
    char * ref=outMessage;
};


#endif //BLEHandler_H

