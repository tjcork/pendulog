#include "freeram.h"
#include "mpu.h"
#include "I2Cdev.h"
#include<stdlib.h>

#define BUFFER_LENGTH (80)
#define VALUE_LENGTH (7)
//#define CURRENT_TIME receivedTime+millis()-timeOffset
#define SAMPLE_FREQUENCY (20)
#define MAX_MESSAGE_LENGTH (55)
#define RECALL_QUEUE_LENGTH (10)

#define CONFIG_MAX_PACKET_SIZE (40)
#define PACKET_DELIMITER (';')
#define CONFIG_CHAR ('?')
#define RESEND_CHAR ('R')
#define CONNECTED_CHAR ('!')
#define DATA_CHAR ('>')

unsigned long long atoull(const char* ptr);
unsigned long atoul(const char* ptr);
void str_ll(uint64_t value, char* string);

/*
unsigned long currentID=0;
bool timeSet=false;
uint64_t receivedTime=0;
uint64_t timeStamp;
unsigned long timeOffset=0;
*/




enum PacketType {
  ERROR_PACKET=0,
  CONFIG_PACKET=1,
  RESEND_PACKET=2,
  CONNECTION_PACKET=3
};

enum ConfigPacketFormat {
  CONFIG_TYPE_INDEX=0,
  CONFIG_ID_INDEX=1,
  CONFIG_ID_SIZE=11,
  CONFIG_TIME_INDEX=2,
  CONFIG_TIME_SIZE=21,
  CONFIG_PARAMETERS=3
};

enum RequestPacket {
  RESEND_INDEX=1,
  REQUEST_PARAMETERS=2
};


class Configuration {
  public:
    char ID_str[CONFIG_ID_SIZE];
    char time_str[CONFIG_TIME_SIZE];
    unsigned long currentID = 0;
    unsigned long long receivedTime;
    unsigned long timeOffset=0;
    bool timeSet=false;
    Configuration() {}
    Configuration(const char* id, const char* time) {
      strncpy(ID_str, id, CONFIG_ID_SIZE);
      strncpy(time_str, time, CONFIG_TIME_SIZE);      
      set();
    }
    void ID(const char* id) {
      strncpy(ID_str,id,CONFIG_ID_SIZE);
      Serial.println(ID_str);
    }
    void TIME(const char* time) {
      strncpy(time_str, time, CONFIG_TIME_SIZE);
      Serial.println(time_str);
    }
    bool set() {
      timeSet=true;
      currentID=atoul(ID_str);
      Serial.print("ID: ");Serial.println(currentID);
      receivedTime=atoull(time_str);
      Serial.print("TIME: "); Serial.println(currentTime_str());
      Serial.print("Config Set");
    }
    unsigned long long currentTime() {
      if (timeSet)
        return receivedTime+millis()-timeOffset;
      else
        return millis();
    }
    char* currentTime_str() {
      str_ll(currentTime(),time_str);
      return time_str;
    }
    char* currentID_str() {

      return ID_str;
    }
    void incrementID() {
      currentID++;
    }
};


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

Configuration config;
PacketType typeReceived;
unsigned long idRequested;
bool waiting=false;
RecentMessages store;

char buffer[BUFFER_LENGTH];       // Buffer to store response
int timeout=1000;            // Wait 800ms each time for BLE to response, depending on your application, adjust this value accordingly
    char sTime[VALUE_LENGTH+2];
    char sYaw[VALUE_LENGTH+2];
    char sPitch[VALUE_LENGTH+2];
    char sRoll[VALUE_LENGTH+2];    


//=================================================================================
//https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
//

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}


boolean BLECmd(long timeout, char* command, char* temp, size_t len, boolean verboseoutput = true) {
  long endtime;
  boolean found=false;
  endtime=millis()+timeout;
  //memset(temp,0,len);         // clear buffer
  found=true;
  if(verboseoutput) Serial.print("Arduino send = ");
  if(verboseoutput) Serial.println(command);
  Serial1.print(command);
  
  // The loop below wait till either a response is received or timeout
  // The problem with this BLE Shield is the HM-10 module does not response with CR LF at the end of the response,
  // so a timeout is required to detect end of response and also prevent the loop locking up.

  while(!Serial1.available()){
    if(millis()>endtime) {      // timeout, break
      found=false;
      break;
    }
    delay(2);
  }  

  if (found) {            // response is available
    int i=0;
    while(Serial1.available()) {    // loop and read the data
      char a=Serial1.read();
      // Serial.print((char)a); // Uncomment this to see raw data from BLE
      temp[i]=a;          // save data to buffer
      i++;
      if (i>=len-1) break;  // prevent buffer overflow, need to break
      delay(1);           // give it a 2ms delay before reading next character
    }
    temp[i]=0;
    if(verboseoutput) Serial.print("BLE reply    = ");
    if(verboseoutput) Serial.println(temp);
    return true;
  } else {
    if(verboseoutput) Serial.println("BLE timeout!");
    return false;
  }
}

boolean BLEInit() {
  char temp[10];
  BLECmd(timeout,"AT+RENEW",temp,10);
  delay(200);
    BLEAutoBaud();     //incase instant baud change
  BLECmd(timeout,"AT+BAUD4",temp,10);
  delay(200);
    BLEAutoBaud();     //incase instant baud change
  delay(200);
  BLECmd(5000,"AT+RESET",temp,10);
    BLEAutoBaud();    
  if (!BLECmd(timeout,"AT+POWE2",temp,10)) return false;
  if (!BLECmd(timeout,"AT+NOTI1",temp,10)) return false;
  return true;
}


boolean BLEIsReady() {
  char temp[4];     //catch first additional character
  BLECmd(250, "AT",temp,4,false);    // Send AT and store response to buffer 
  if (strcmp(temp,"OK")==0){    
    return true;
  } else {
    return false;
  }  
}


enum Bauds {
  BAUD0=9600,
  BAUD1=19200,
  BAUD2=38400,
  BAUD3=57600,
  BAUD4=115200
};

long bauds[] = {BAUD0,BAUD1,BAUD2,BAUD3,BAUD4}; // common baud rates, for HM-10 module with SoftwareSerial, try not to go over 57600
bool BLEAutoBaud() {
  int baudcount=sizeof(bauds)/sizeof(long);
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<4; x++) {    // test at least 4 times for each baud
      Serial1.begin(bauds[i]);
      if (BLEIsReady()) {
        Serial.print("BLE BAUD:  ");Serial.println(bauds[i]);
        return true;
      }
    }
  }
  Serial.println("BLE BAUD:  UNKNOWN");
  return false;
}


boolean parseBLE(PacketType &type, Configuration &config, unsigned long &idRequest, bool &midPacket) {
  static char* strings[CONFIG_PARAMETERS];
  static char BLEBuffer[CONFIG_MAX_PACKET_SIZE];
  static int strIndex = 0;
  static int pos = 0;
  static bool errorParsing=false;
  static char* ptr = &(BLEBuffer[0]);
  static unsigned long lastRec = millis();
  if(midPacket and millis() - lastRec > 1000) {
    //discard last data
    pos=0;
    strIndex=0;
    errorParsing=false;
    ptr= &(BLEBuffer[0]);
  }

  while(Serial1.available()) {    // loop and read the data
    char a=Serial1.read();
    Serial.print(a);
    midPacket=true;    
    if (pos + 1 == CONFIG_MAX_PACKET_SIZE) {   //overflow error
      Serial.println("ERROR:  Packet too long, no newline");
      errorParsing=true;
    }
    if (a=='\n') {            //end of data packet
      bool retv=false;        //set failed
      if (!errorParsing) {
        BLEBuffer[pos]='\0';
        strings[strIndex]=ptr;
        switch (type) {
          case CONFIG_PACKET:
            if (strIndex==CONFIG_PARAMETERS) {
              //assign config packet values;
                config.ID(strings[CONFIG_ID_INDEX]);
                config.TIME(strings[CONFIG_TIME_INDEX]);
              retv=true;
            }
            break;
          case RESEND_PACKET:
            if (strIndex+1==REQUEST_PARAMETERS) {
              //assign id request
              idRequest=atoul(strings[RESEND_INDEX]);
              retv=true;      
            }
            break;
          case CONNECTION_PACKET:
              //if (strIndex==1) { //end of connection packet
                  midPacket=false;
                  //static var cleanup
                  strIndex=0;
                  pos=0;
                  ptr=&(BLEBuffer[0]);
                  errorParsing=false;
                  Serial1.println("@;");
                  delay(500);
                  return false;
                //}
                //Serial1.println("@;");
                  
            
          default:
           Serial.println("No packet type assigned");
        }
      }
      midPacket=false;
      //static var cleanup
      strIndex=0;
      pos=0;
      ptr=&(BLEBuffer[0]);
      errorParsing=false;
      return retv;
    }
      
    if (errorParsing) continue;       //if there's an error, clear the buffer until the next '\n' reached

    if (a==PACKET_DELIMITER) {        //end of substring
      BLEBuffer[pos]='\0';
      strings[strIndex]=ptr;
      if (pos+1==CONFIG_MAX_PACKET_SIZE) {
        errorParsing=true;
        continue;
      }
      pos++;
      ptr=&(BLEBuffer[pos]);
      strIndex++;
      continue;        
    }

    if (a=='\r') { continue;   }
    
    if (strIndex==0) {
      if (a==CONFIG_CHAR) {
        type=CONFIG_PACKET;
        config.timeOffset=millis();
        break;
      }
      else if (a==RESEND_CHAR) {
        type=RESEND_PACKET;
      }
      else if (a=='!') {
        type = CONNECTION_PACKET;
        errorParsing=true;
      }      
      else
        type=ERROR_PACKET;
    }
    else if (!isDigit(a)) {
      errorParsing=true;
      continue;
    }
    BLEBuffer[pos]=a;
    pos++;
  }
  lastRec=millis();
  return false;   //finished receiving, not end of packet '\n', midPacket flag should be set
}



boolean startMessage() {
  unsigned long endtime;
  endtime=millis()+1000;
  while(!Serial1.available()){
    if(millis()>endtime) return false;
    delay(5);
  }
  
  int i=0;
  char temp[BUFFER_LENGTH];
  while(Serial1.available()) {    // loop and read the data
    char a=Serial1.read();
    temp[i]=a;          // save data to buffer
    i++;
    if (i>=BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
    delay(2);           // give it a 2ms delay before reading next character
  }
  if (strcmp(temp,"PENDSTART")==0)
    return true;
  Serial.print("err:  ");
  Serial.println(temp);
  return false;
}




bool readTimeMessage(uint64_t &receivedTime, unsigned long &timeStamp ) {
  unsigned long endtime;
  uint64_t result = 0;
  endtime=millis()+1000;
  while(!Serial1.available()){
    if(millis()>endtime) return false;
    delay(5);
  }
  timeStamp=millis();
  if (Serial1.read()==7) {     //BELLXXX...XXXX/r/n
    while(Serial1.available()) {    // loop and read the data
      char a=Serial1.read();
      Serial.print(a);
      if (a=='\r') {
        delay(2);
        if (Serial1.available()) Serial1.read();  //clear '/n'
        receivedTime=result;
        return true;
      }
      if (!isdigit(a)) return false;
      result *= 10;
      result += a - '0';
      delay(2);           // give it a 2ms delay before reading next character
    }
  }
  return false;
}


void str_ll(uint64_t value, char* string)
{
    const int NUM_DIGITS    = log10(value) + 1;

    char sz[NUM_DIGITS + 1];
    
    sz[NUM_DIGITS] =  0;
    for ( size_t i = NUM_DIGITS; i--; value /= 10)
    {
        sz[i] = '0' + (value % 10);
    }
    strcpy(string,sz);
    return;
}

unsigned long long atoull(const char* ptr) {
  unsigned long long result = 0;
  while (*ptr && isdigit(*ptr)) {
    result *= 10;
    result += *ptr++ - '0';
  }
  return result;
}

unsigned long atoul(const char* ptr) {
  unsigned long result = 0;
  while (*ptr && isdigit(*ptr)) {
    result *= 10;
    result += *ptr++ - '0';
  }
  return result;
}



int ret;

void setup() {
    Fastwire::setup(400,0);

  Serial.begin(9600);
  Serial1.begin(9600);
  delay(500);
  //while(!Serial);

  ret = mympu_open(SAMPLE_FREQUENCY);
  Serial.println("Accelerometer Initialisation");
  Serial.print("MPU init: "); Serial.println(ret);
  Serial.print("Free mem: "); Serial.println(freeRam()); Serial.println();
  
  Serial.println("Waiting for BLE");
  BLEAutoBaud();
  if(! BLEInit()) {
      Serial.println("Failed to initialise... Retrying");
      BLEAutoBaud();
      while (! BLEInit()) {
        BLEAutoBaud();
        BLEIsReady();        
        Serial.print(".");
      }
  }
  Serial.println("BLE READY"); Serial.println();

  Serial1.begin(115200);
  Serial.println("Waiting for config");
  int failcounter=0;
  
  while(!parseBLE(typeReceived, config, idRequested, waiting)) {
    if (failcounter==100) {
      BLEAutoBaud();
      failcounter=0;
    }
    failcounter++;
    Serial1.println("?;");
    Serial.print(".");
    delay(500);
  }
  if (typeReceived==CONFIG_PACKET) {
    Serial.println("Config Received");
    config.set();
  }
  //Serial1.println('\a');  //send BELL character for success
  Serial.print("TIME IS:");
  Serial.println(config.currentTime_str());
  Serial.println("SYNCED");   

}

unsigned int c = 0; //cumulative number of successful MPU/DMP reads
unsigned int np = 0; //cumulative number of MPU/DMP reads that brought no packet back
unsigned int err_c = 0; //cumulative number of MPU/DMP reads that brought corrupted packet
unsigned int err_o = 0; //cumulative number of MPU/DMP reads that had overflow bit set

char sendBuffer[BUFFER_LENGTH];
char timeString[30];
String stringSend;
DataMessage d;
  

void loop() {
     
    ret = mympu_update();
    
    switch (ret) {
	case 0: c++; 	  
	  //create send string;
    stringSend = String(DATA_CHAR) + ";" + String(config.currentID) + ";" + String(config.currentTime_str()) + ";" + String(mympu.ypr[0],3) + ";" + String(mympu.ypr[1],3) + ";" + String(mympu.ypr[2],3);
    //Store incase of error in transmission 
    d.id=config.currentID;
    strcpy(d.content,stringSend.c_str());
    store.push(d);
    Serial1.println(stringSend);
    config.incrementID();
	break;
	case 1: np++; 
	  //Serial.println("ERR: 1 - no packet available");
    return;
	case 2: err_o++; 
    //Serial.println("ERR: 2 - if BIT_FIFO_OVERFLOWN is set");
    return;
	case 3: err_c++; 
	  //Serial.println("ERR: 3 - if frame corrupted");
    return; 
	default: 
		Serial.print("READ ERROR!  ");
		Serial.println(ret);
		return;
  }

  if (Serial1.available()) {
    
    //Serial.print("VCC VOLTAGE (mV): ");Serial.println(readVcc());
    if (parseBLE(typeReceived, config, idRequested, waiting)) {
      switch (typeReceived) {
        case CONFIG_PACKET:
          Serial.println("New config received");
          config.set();
          break;
        case RESEND_PACKET: 
            if (store.findID(idRequested)) {
              Serial.print("Recovered message for ID: "); Serial.println(idRequested); Serial.println(store.foundMsg);
              Serial1.println(store.foundMsg);  //send
            }
            else {
              Serial.print("Packet resend requested, MISSED:   "); Serial.println(idRequested);
            }
            break;
      }
      
    }
  }
    
}


