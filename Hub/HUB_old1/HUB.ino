/*
 * 
 *
 *
 * BLE CREDITS
 * https://github.com/jpliew/BLEShieldSketch/blob/master/HM_10_Test.ino
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <math.h>

#define CHUNK_SIZE (10)
#define MAX_FRAME_LENGTH (64)       // Here we define a maximum framelength to 64 bytes. Default is 256.
#define CALLBACK_FUNCTIONS (1)
#define MESSAGE_INTERVAL (10000)
#define HEARTBEAT_INTERVAL (20000)
#define CURRENT_TIME recievedTime+millis()-timeOffset
#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(4))
#define USE_SERIAL Serial
#define CONNECTION_TIMEOUT (10000)
#define BUFFER_LENGTH (100)
#define JSON_STRING_LENGTH (250)


#define BLE_MAX_PACKET_SIZE (100)
#define PACKET_DELIMITER (';')
#define COMMAND_CHAR ('\a')


#define BAUD0 9600
#define BAUD1 19200
#define BAUD2 38400
#define BAUD3 57600
#define BAUD4 115200

WebSocketsClient webSocket;
SoftwareSerial ble(14, 12);        //RX|TX


char host[] = "192.168.0.30";
int port = 3000;
bool isConnected = false;

uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;
uint64_t lastreply=0;

unsigned long timeOffset=0;
unsigned long reqSent;
unsigned long roundTripTime;

unsigned long currentID=0;
unsigned long lastID=0;

uint64_t recievedTime=0;
boolean timeSet=false;
bool waiting = false;


const char* ssid = XXXXX;
const char* password = XXXXX;


enum DataPacketFormat {
  ID_INDEX = 0,
  ID_SIZE = 4,
  TIME_INDEX = 1,
  TIME_SIZE = 21,
  YAW_INDEX = 2,
  YAW_SIZE = 9,
  PITCH_INDEX = 3,
  PITCH_SIZE = 9,
  ROLL_INDEX = 4,
  ROLL_SIZE = 9,
  DATA_PARAMETERS = 5
};



class dataPacket {
  public:
    char ID[ID_SIZE];
    char Time[TIME_SIZE];
    char Yaw[YAW_SIZE];
    char Pitch[PITCH_SIZE];
    char Roll[ROLL_SIZE];
    dataPacket() {}
    dataPacket(char* IDStr, char* timeStr, char* yawStr, char* pitchStr, char* rollStr) {
      setID(IDStr); setTimeStamp(timeStr); setYaw(yawStr); setPitch(pitchStr); setRoll(rollStr);
    }
    void setID(const char* IDStr) { strncpy(ID, IDStr, ID_SIZE); }
    void setTimeStamp(const char* timeStr) { strncpy(Time, timeStr, TIME_SIZE); }
    void setYaw(const char* yawStr) { strncpy(Yaw, yawStr, YAW_SIZE); }
    void setPitch(const char* pitchStr) { strncpy(Pitch, pitchStr, PITCH_SIZE); }
    void setRoll(const char* rollStr) { strncpy(Roll, rollStr, ROLL_SIZE); }
    unsigned long getID() { return ((unsigned)atol(ID)); }
    dataPacket & operator=(const dataPacket &rhs) {
      this->setID(rhs.ID); this->setTimeStamp(rhs.Time); this->setYaw(rhs.Yaw); this->setPitch(rhs.Pitch); this->setRoll(rhs.Roll);     
      return *this; 
    }
};

class dataChunk {
  public:
    dataChunk() {};
    dataPacket chunk[CHUNK_SIZE];
    char chunkJSON[CHUNK_SIZE*100+50];
    static int index;
    bool addData(dataPacket d) {
      if (isFull()) {
        Serial.print("ERROR FULL"); 
        return false;
      }
      chunk[index++]=d;
      return true;
    }
    bool isFull() {
      if (index==CHUNK_SIZE) return true;
      else return false;
    }
    bool empty() {
      index=0;
    }
    char* toJSON() {
      StaticJsonBuffer<CHUNK_SIZE*100+100> jsonBuffer;
      JsonObject& rootObject = jsonBuffer.createObject();
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

};
//initialise static
int dataChunk::index = 0;

dataPacket data;
dataChunk chunk;

void createJSONString(char * JSONOutput, dataPacket& data);
boolean createSocketMessage(char* output, const char* Time,const char* Yaw,const char* Pitch,const char* Roll);


char blebuf[BUFFER_LENGTH];

int timeout=1000;            // Wait 800ms each time for BLE to response, depending on your application, adjust this value accordingly

char *BLEstrings[4];
char socketMessage[200];
uint64_t timeLastRecieved;




long bauds[] = {BAUD0,BAUD1,BAUD2,BAUD3,BAUD4}; // common baud rates, for HM-10 module with SoftwareSerial, try not to go over 57600
bool BLEAutoBaud() {
  int baudcount=sizeof(bauds)/sizeof(long);
  Serial.print("BLE BAUD:  ");
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<4; x++) {    // test at least 4 times for each baud
      ble.begin(bauds[i]);
      if (BLEIsReady()) {
        Serial.println(bauds[i]);
        return true;
      }
    }
  }
  Serial.print("UNKNOWN");
  return false;
}


boolean BLECmd(long timeout, char* command, char* temp, size_t len, boolean verboseoutput = true) {
  long endtime;
  boolean found=false;
  endtime=millis()+timeout;
  //memset(temp,0,len);         // clear buffer
  found=true;
  if(verboseoutput) Serial.print("Arduino send = ");
  if(verboseoutput) Serial.println(command);
  ble.print(command);
  
  // The loop below wait till either a response is received or timeout
  // The problem with this BLE Shield is the HM-10 module does not response with CR LF at the end of the response,
  // so a timeout is required to detect end of response and also prevent the loop locking up.

  while(!ble.available()){
    if(millis()>endtime) {      // timeout, break
      found=false;
      break;
    }
    delay(2);
  }  

  if (found) {            // response is available
    int i=0;
    while(ble.available()) {    // loop and read the data
      char a=ble.read();
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


boolean BLEIsReady() {
  char temp[4];     //catch first additional character
  BLECmd(250, "AT",temp,4,false);    // Send AT and store response to buffer 
  if (strcmp(temp,"OK")==0){    
    return true;
  } else {
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
  if (!BLECmd(timeout,"AT+ROLE1",temp,10)) return false;
  if (!BLECmd(timeout,"AT+IMME1",temp,10)) return false;
  if (!BLECmd(timeout,"AT+POWE2",temp,10)) return false;
  if (!BLECmd(timeout,"AT+NOTI1",temp,10)) return false;
  return true;
}



boolean strNumerical(char* str) {
  char *p=str;
  while (*p) {      // loop characters
    if (isdigit(*p)) return true;
    else  p++;
  }
  return false; 
}

boolean BLEIsConnected(bool connWait=false,int recursionCount=0) {
  char temp[10];
  memset(temp,0,10);
  boolean bleRead=false;
  if (ble.available()) {
    int i=0;  
    while(ble.available()) {                      // loop and read the data
      bleRead=true;
      char a=ble.read();
      if (a=='\a') {                              //request for time received
        char timeString[30];
        ble.print("\a");                          //provide time
        str_ll(CURRENT_TIME,timeString);
        ble.println(timeString); 
        return true;
      }    
      if (isDigit(a)) {
        return true;
      }
      temp[i]=a;          // save data to buffer
      i++;
      if (i>=BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
      delay(2);           // give it a 2ms delay before reading next character
    }
  }
  if (bleRead and strcmp(temp,"OK+CONNA")==0) {  //connecting
    delay(100);
    return BLEIsConnected(true);
  }
  if (bleRead and strcmp(temp,"OK+CONNE")==0)    //error
    return false;
  if (bleRead and strcmp(temp,"OK+CONNF")==0)    //out of range
    return false;
  if (bleRead and strcmp(temp,"OK+CONN")==0)     //success
    return true;
  if(!connWait and !BLECmd(timeout,"AT+CON88C25512421A",temp,10)) {  //88C25512421A    //check if we had already recieved CONNA and are waiting, request connection if not
    Serial.println("Checking Baud...");
    BLEAutoBaud();            //timeout could mean baud is improperly set
    return false;             //could also mean connected, let outer loop rerequest
  
  }
  if (strcmp(temp,"OK+CONNA")==0) {
    delay(1000);
    if(recursionCount>10) return false;
    return BLEIsConnected(true,recursionCount+1);
  }
  if (strcmp(temp,"OK+CONNE")==0) {
    delay(1000);
    return false;
  }
  if (strcmp(temp,"OK+CONNF")==0)
    return false;
  if (strcmp(temp,"OK+CONN")==0)
    return true;
  if (connWait) {
    delay(2000);
    if(recursionCount>5) return false;
    return BLEIsConnected(true,recursionCount+1);
  }
  return false;
}

boolean parseBLE(dataPacket& data, bool &midPacket) {
  static char* strings[DATA_PARAMETERS];
  static char BLEBuffer[BLE_MAX_PACKET_SIZE];
  static int strIndex = 0;
  static int pos = 0;
  static bool errorParsing=false;
  static bool commandPacket=false;
  static char* ptr = &(BLEBuffer[0]);

  while(ble.available()) {    // loop and read the data
    char a=ble.read();
    //Serial.print(a);
    midPacket=true;
    
    if (pos==BLE_MAX_PACKET_SIZE-1) {   //overflow error
      Serial.println("ERROR:  Packet too long, no newline");
      errorParsing=true;
    }    
    if (a=='\n') {            //end of data packet
      bool retv=false;        //set failed
      if (!errorParsing) {
        BLEBuffer[pos]='\0';
        strings[strIndex]=ptr;  
        if (strIndex+1 == DATA_PARAMETERS) {
          data.setID(strings[ID_INDEX]);
          data.setTimeStamp(strings[TIME_INDEX]);
          data.setYaw(strings[YAW_INDEX]);
          data.setPitch(strings[PITCH_INDEX]);
          data.setRoll(strings[ROLL_INDEX]);
          retv=true;                        //exit with success
        }
        else {
          Serial.println("ERROR:  Incorrect number of parameters");
        }
      }
      midPacket=false;
      //static var cleanup
      strIndex=0;
      pos=0;
      ptr=&(BLEBuffer[0]);
      errorParsing=false;
      commandPacket=false;
      return retv;
    }
      
    if (errorParsing) continue;       //if there's an error, clear the buffer until the next '\n' reached

    if (a==PACKET_DELIMITER) {        //end of substring
      BLEBuffer[pos]='\0';
      strings[strIndex]=ptr;
      if (pos+1==BLE_MAX_PACKET_SIZE) {
        errorParsing=true;
        continue;
      }
      pos++;
      ptr=&(BLEBuffer[pos]);
      strIndex++;
      continue;          
    }

    if (a=='\r') {
        continue;
    }
    
    switch (strIndex) {       //Error checking
      case ID_INDEX:            //ID Packet (uint)
        if (a==COMMAND_CHAR) {
          commandPacket=true;
          break;
        }
        // treat errors same as TIME
      case TIME_INDEX:          //Time Packet (uint)
        if (!isdigit(a)) {
          
          Serial.printf("ERROR:  Non numerical char '%c' in ID/Time\n",a);
          errorParsing=true; 
          continue;
        }
        break; 
      case YAW_INDEX:
      case PITCH_INDEX:
      case ROLL_INDEX:     //YPR (float)
        if (!isdigit(a) and (a != '-') and (a != '.')) {
          Serial.printf("ERROR:  Non numerical char '%c' in Y/P/R\n",a);
          errorParsing=true;
          continue;
        }
        break;
      case DATA_PARAMETERS:
      default:    //more than 5 parameters, errror
        Serial.println("ERROR:  More than 5 parameters");
        errorParsing=true;
        continue;
    }
    
    BLEBuffer[pos]=a;
    pos++;
  }
  return false;   //finished recieving, not end of packet '\n', midPacket flag should be set
}



unsigned long long atoull(const char* ptr) {
  unsigned long long result = 0;
  while (*ptr && isdigit(*ptr)) {
    result *= 10;
    result += *ptr++ - '0';
  }
  return result;
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




void WIFI_Connect()
{
  WiFi.disconnect();
  Serial.println("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay ( 250 );
      Serial.print ( "." );
      delay ( 250 );
    }
    else
      return;
  }
  WIFI_Connect();
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    uint64_t eventTime=millis();
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
              USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
              isConnected = true;
              // socket.io upgrade confirmation message (required)
              webSocket.sendTXT("5");
              // send message to server when Connected

            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            if (payload[0]=='4' and payload[1]=='2') {
              //message
              String received = (char*)payload;
              String RID = received.substring(received.indexOf("[\"")+2, received.indexOf("\","));
              if (RID=="rtime") {
                  if (!timeSet) {
                    String RContent = received.substring(received.indexOf(":")+2,received.length()-3);
                    Serial.println(RContent);
                    Serial.print("RTT:   "); Serial.println(roundTripTime);
                    timeOffset=eventTime;
                    roundTripTime=timeOffset-reqSent;
                    recievedTime=atoull(RContent.c_str());
                    timeSet=true;
                  }
              }
            }
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);
            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}


void createJSONString(char * JSONOutput, dataPacket& data) {
  StaticJsonBuffer<SENSORDATA_JSON_SIZE> jsonBuffer;
  JsonObject& JSON = jsonBuffer.createObject();
  JSON["T"]=data.Time;
  JSON["Y"]=data.Yaw;
  JSON["P"]=data.Pitch;
  JSON["R"]=data.Roll;
  JSON.printTo(JSONOutput,JSON_STRING_LENGTH); 
  return;
}


void createSocketMessage(char* output, const char* messageType, const char* JSONString){
  strcpy(output, "42[\"");  strcat(output,messageType);  strcat(output,"\",");  strcat(output,JSONString);  strcat(output, "]");
}


void requestResendPacket(unsigned long packetIDRequest) {
  ble.println(packetIDRequest);
  return;  
}





void setup() {
    Serial.begin(115200);
    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(500);
      }
    Serial.println();  
    BLEAutoBaud();
    Serial.println();
    WIFI_Connect();

    webSocket.beginSocketIO("192.168.0.30", 3000);
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);
       
    //getTime();
    while(!timeSet) {
      if (isConnected) {
        webSocket.sendTXT("42[\"gtime\",\"Time Please\"]");
        reqSent=millis();
      }
      delay(500);
    }    

    Serial.println("Starting BT module");
    while (! BLEIsReady()) {
      delay(100);
      BLEAutoBaud();
      Serial.print(".");
    }
    Serial.println();
    Serial.println("BT READY");
    if(! BLEInit()) {
      Serial.println("Failed to initialise... Retrying");
      while (! BLEInit()) {
        BLEIsReady();        
        Serial.print(".");
      }
    }
    Serial.println("BT INITIALISED");Serial.println();    
    Serial.println("Connecting BT module");
    while (! BLEIsConnected()) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("BT CONNECTED");

    char timeString[30];
    ble.print("\a");     //signal to begin
    str_ll(CURRENT_TIME,timeString);
    ble.println(timeString);
    
    timeLastRecieved=millis();
}

void loop() {
 
  if (ble.available()) {
    if (parseBLE(data,waiting)) {
      currentID=data.getID();
      unsigned long numMissing = currentID-lastID-1;
      if (numMissing < 5) {
        //PACKET LOSS
        for (int n=1; n <= numMissing; n++) {
          Serial.print("Packet missed:    "); Serial.println(lastID+n);
          requestResendPacket(lastID+n);
        };
      }
      else {
       lastID=currentID; 
      }
      timeLastRecieved=millis();
      chunk.addData(data);
      if (chunk.isFull()) {  //chunk ready
        if(isConnected) {
          char message[CHUNK_SIZE*70+120];
          createSocketMessage(message, "data chunk", chunk.toJSON());
          webSocket.sendTXT(message);
        }
        chunk.empty();
      }
      lastID=currentID;
    }
    else if (!waiting) {
      //PACKET ERROR
      Serial.print("PACKET ERROR:    "); Serial.println(lastID+1);
      requestResendPacket(lastID+1);
      lastID=lastID+1;
    }
  }
  
  if((millis() - heartbeatTimestamp) > HEARTBEAT_INTERVAL and isConnected) {
      heartbeatTimestamp = millis();
      // socket.io heartbeat message
      webSocket.sendTXT("2");
   } 
   if(millis() - timeLastRecieved > CONNECTION_TIMEOUT) {
    Serial.println("Reconnecting BT module");
    while (! BLEIsConnected()) {
      delay(500);
      Serial.print(".");
    }
    timeLastRecieved=millis();
  }
  if (WiFi.status() != WL_CONNECTED) {
      WIFI_Connect();
  }
}


