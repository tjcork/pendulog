#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "BLEHandler.h"
#include "utils.h"
#define RECALL_QUEUE_LENGTH (10)


class Schema;
class BLEHandler;

class LostPacket {
  public:
    unsigned long ID;
    int lifetime = RECALL_QUEUE_LENGTH;
    LostPacket() {}
    LostPacket(unsigned long I) : ID(I) {}
    bool isFound(unsigned long matchID);
    bool isLost();
};

class PacketRecovery {
  public: 
    PacketRecovery() {}
    LostPacket queue[RECALL_QUEUE_LENGTH];
    int count=0;
    void remove(int index);
    bool recover(unsigned long thisID);
    void addMissing(unsigned long missingID);
};

class ErrorHandler {
  public:
    BLEHandler * bleH;
    unsigned long expectingNext=0;
    PacketRecovery store;
    ErrorHandler(BLEHandler *b) : bleH(b) {}
    ErrorHandler(unsigned long I) : expectingNext(I+1) {}
    void error(unsigned int fieldIndex,Schema &badData); 
    bool success(Schema &data);
};




#endif //ERROR_HANDLER_H

