#include "containers.h"

//      Configuration
//===============================================================================

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
  
    void Configuration::send() {
      for (int i = 0; i < CONFIG_PARAMETERS; i++) {
        switch(i) {
          case CONFIG_TYPE_INDEX:
             ble.print(CONFIG_CHAR);     //signal to begin
             break;
          case CONFIG_TIME_INDEX:
            ble.print(currentTime_str());
            break;
          case CONFIG_ID_INDEX:
            ble.print(initialID);
            break;
        }
        ble.print(PACKET_DELIMITER);
      }
      ble.println();
    }  

//      LostPacket
//===============================================================================
    
    LostPacket::LostPacket(unsigned long I) {
      ID=I;
      lostID = ID + RECALL_QUEUE_LENGTH;
    }
    
    bool LostPacket::isFound(unsigned long matchID) {
      return (ID == matchID);
    }
    
    bool LostPacket::isLost(unsigned long matchID) {
      return (matchID >= lostID);
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
        else if(queue[i].isLost(thisID)) {
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

//      DataPacket
//===============================================================================


    DataPacket::DataPacket(char* IDStr, char* timeStr, char* yawStr, char* pitchStr, char* rollStr) {
      setID(IDStr); setTimeStamp(timeStr); setYaw(yawStr); setPitch(pitchStr); setRoll(rollStr);
    }
    void DataPacket::setID(const char* IDStr) { 
      strncpy(ID, IDStr, ID_SIZE); 
    }
    void DataPacket::setTimeStamp(const char* timeStr) { 
      strncpy(Time, timeStr, TIME_SIZE); 
    }
    void DataPacket::setYaw(const char* yawStr) { 
      strncpy(Yaw, yawStr, YAW_SIZE); 
    }
    void DataPacket::setPitch(const char* pitchStr) { 
      strncpy(Pitch, pitchStr, PITCH_SIZE); 
    }
    void DataPacket::setRoll(const char* rollStr) { 
      strncpy(Roll, rollStr, ROLL_SIZE); 
    }
    unsigned long DataPacket::getID() { 
      return (atoul(ID)); 
    }
    DataPacket & DataPacket::operator=(const DataPacket &rhs) {
      this->setID(rhs.ID); this->setTimeStamp(rhs.Time); this->setYaw(rhs.Yaw); this->setPitch(rhs.Pitch); this->setRoll(rhs.Roll);     
      return *this; 
    }

//      DataChunk
//================================================================================

    bool DataChunk::addData(DataPacket d) {
      if (isFull()) {
        Serial.print("ERROR FULL"); 
        return false;
      }
      chunk[index++]=d;
      return true;
    }
    
    bool DataChunk::isFull() {
      if (index==CHUNK_SIZE) return true;
      else return false;
    }
    
    bool DataChunk::empty() {
      index=0;
    }
    
    char* DataChunk::toJSON() {
      StaticJsonBuffer<JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(CHUNK_SIZE) + CHUNK_SIZE*JSON_ARRAY_SIZE(DATA_FIELDS_COUNT-1)+500> jsonBuffer;
      JsonObject& rootObject = jsonBuffer.createObject();
      rootObject["type"]="chunk";
      rootObject["numChunks"]=CHUNK_SIZE;
      JsonArray& data = rootObject.createNestedArray("data");
      for (int j = 0; j < CHUNK_SIZE; j++) {
        JsonObject& d = data.createNestedObject();
        d["I"]=chunk[j].ID;
        d["T"]=chunk[j].Time;
        d["Y"]=chunk[j].Yaw;
        d["P"]=chunk[j].Pitch;
        d["R"]=chunk[j].Roll;
      }
      rootObject.printTo(chunkJSON,sizeof(chunkJSON));
      return chunkJSON;
    }
    
//================================================================================
