#ifndef MESSAGE_STORE_H
#define MESSAGE_STORE_H

#define MAX_MESSAGE_LENGTH (80)
#define RECALL_QUEUE_LENGTH (1)



class DataMessage {
  public: 
    char content[MAX_MESSAGE_LENGTH];
    unsigned long id;
    DataMessage() {}
    DataMessage(const char * msg, const unsigned long i) {
      strcpy(content,msg);  id=i;
    }
    DataMessage & operator=(const DataMessage &rhs) {
      strcpy(this->content,rhs.content);  this->id=rhs.id;
      return *this; 
    }
};

class RecentMessages {
  public:
    DataMessage messageBuffer[RECALL_QUEUE_LENGTH];
    char * foundMsg;   //keep found message location in memory
    uint8_t nextPush=0;
    size_t size = RECALL_QUEUE_LENGTH;
//    RecentMessages(const unsigned int bufferSize) {
//      size=bufferSize;
//      messageBuffer = new DataMessage [bufferSize];      
//    }
    RecentMessages() {}
    bool findID(const unsigned long queryID) {
     uint8_t index; 
     bool found=false;
     for (index=0; index<size; index++) {
        if (messageBuffer[index].id==queryID) {
          found=true;
          foundMsg=messageBuffer[index].content;  
          break;
        }
     }
     return found;      
    }
    void push(DataMessage d) {
      messageBuffer[nextPush]=d;
      nextPush = (nextPush+1==size) ? 0 : nextPush + 1 ;     //push new entries back to the beginning
    }
};


#endif //MESSAGE_STORE_H
