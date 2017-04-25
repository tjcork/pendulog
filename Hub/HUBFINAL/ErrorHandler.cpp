#include "ErrorHandler.h"


//      ErrorHandler
//===============================================================================
    
    void ErrorHandler::error(unsigned int fieldIndex,Schema &badData) {
     /*
      if (fieldIndex > 0) {
        //error occurred after ID field so we can guess which packet it was
        //requires badData.fields[0].identifier==ID
        bleH->send(RESEND_CHAR, badData.fields[0].string_value);
        store.addMissing(atoul(badData.fields[0].string_value));
        expectingNext++;
        Serial.printf("error added id: %d\n");
      }
      else {
      */
        bleH->send(RESEND_CHAR, expectingNext);
        store.addMissing(expectingNext);
        //derive from last receieved
        Serial.printf("error added (assumed) id: %d\n",expectingNext);
        expectingNext++;
        
      }
   
    
    bool ErrorHandler::success(Schema &data) {
      unsigned long currentID=atoul(data.fields[0].string_value);
      //Serial.println(currentID);
      if (store.recover(currentID)) {

        Serial.println("Found in store");
        return true;
      } 
      else if (currentID != expectingNext) {
        //packets completely missed
        Serial.println("missed");
        //Serial.printf("Multiple Packets missed, recieved: %lu expected: %lu\n", currentID, expectingNext);
      }
      expectingNext =  currentID + 1;
      return false;
    }

//      LostPacket
//===============================================================================

     
    bool LostPacket::isFound(unsigned long matchID) {
      lifetime--;
      return (ID == matchID);
    }
    
    bool LostPacket::isLost() {
      return (lifetime <= 0);
    }
    
//      PacketRecovery
//===============================================================================

    void PacketRecovery::remove(int index) {
      count = count == 0 ? 0 : count - 1;
      if (index==count)
        return;
      for (int i=index; i<count; i++) 
        queue[i]=queue[i+1];
    }
    
    bool PacketRecovery::recover(unsigned long thisID) {
      if (count==0) {
        return false;
      }
      bool rett=false;
      for(int i=0; i<count; i++) {
        if(queue[i].isFound(thisID)) {
          remove(i);
          rett=true;
        }
        else if(queue[i].isLost()) {
          Serial.print(thisID); Serial.println(" was lost, not resent in time");
          remove(i);
        }
      }
      return rett;
    }
    
    void PacketRecovery::addMissing(unsigned long missingID) {
      count++;
      if (count == RECALL_QUEUE_LENGTH) {
        remove(0);
      }
      queue[count-1]= LostPacket(missingID);
    }


