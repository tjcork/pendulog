/*
 * WebSocketClientSocketIO.ino
 *
 *  Created on: 06.06.2016
 *
 */

 /*
  * BLE CREDITS
  * https://github.com/jpliew/BLEShieldSketch/blob/master/HM_10_Test.ino
  */

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>

#include <SocketIOClientv1.h>


SocketIOClient client;
char host[] = "192.168.0.30";
int port = 3000;
extern String RID;
extern String Rname;
extern String Rcontent;


#define CHUNK_SIZE 20


// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64
#define CALLBACK_FUNCTIONS 1
#define MESSAGE_INTERVAL 10000
#define HEARTBEAT_INTERVAL 25000
uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;
uint64_t lastreply=0;

unsigned long timeOffset;
unsigned long reqSent;
unsigned long roundTripTime;


#define CURRENT_TIME recievedTime+millis()-timeOffset

uint64_t recievedTime=0;
boolean timeSet=false;  



const char* ssid = XXXXX;
const char* password = XXXXX;


#include <ArduinoJson.h>

//ESP8266WiFiMulti WiFiMulti;

#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(4))

class dataPacket {
  public:
    char ID[4];
    char Time[21];
    char Yaw[9];
    char Pitch[9];
    char Roll[9];
    dataPacket() {}
    dataPacket(char* IDStr, char* timeStr, char* yawStr, char* pitchStr, char* rollStr) {
      setID(IDStr); setTimeStamp(timeStr); setYaw(yawStr); setPitch(pitchStr); setRoll(rollStr);
    }
    void setID(const char* IDStr) { strncpy(ID, IDStr, 4); }
    void setTimeStamp(const char* timeStr) { strncpy(Time, timeStr, 21); }
    void setYaw(const char* yawStr) { strncpy(Yaw, yawStr, 9); }
    void setPitch(const char* pitchStr) { strncpy(Pitch, pitchStr, 9); }
    void setRoll(const char* rollStr) { strncpy(Roll, rollStr, 9); }
    dataPacket & operator=(const dataPacket &rhs) {
      this->setID(rhs.ID); this->setTimeStamp(rhs.Time); this->setYaw(rhs.Yaw); this->setPitch(rhs.Pitch); this->setRoll(rhs.Roll);     
      return *this; 
    }
};


class dataChunk {
  public:
    dataChunk() {};
    dataPacket chunk[CHUNK_SIZE];
    char chunkJSON[CHUNK_SIZE*100];
    static int index;
    bool addData(dataPacket d) {
      if (isFull()) return false;
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
      StaticJsonBuffer<CHUNK_SIZE*70> jsonBuffer;
      JsonObject& rootObject = jsonBuffer.createObject();
      rootObject["numChunks"]=CHUNK_SIZE;
      JsonArray& data = rootObject.createNestedArray("data");
      for (int j = 0; j < 10; j++) {
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


/*
struct dataPacket {
   char* Time;
   char* Yaw;
   char* Pitch;
   char* Roll;
};
*/

#define JSON_STRING_LENGTH 250
dataPacket data;
dataChunk chunk;
char JSONData[JSON_STRING_LENGTH];


void createJSONString(char * JSONOutput, dataPacket& data);

boolean createSocketMessage(char* output, const char* Time,const char* Yaw,const char* Pitch,const char* Roll);

#define USE_SERIAL Serial



#define CONNECTION_TIMEOUT 10000
#define BUFFER_LENGTH 100


SoftwareSerial ble(14, 12);        //RX|TX

char blebuf[BUFFER_LENGTH];
char buffer[BUFFER_LENGTH];       // Buffer to store response
int timeout=1000;            // Wait 800ms each time for BLE to response, depending on your application, adjust this value accordingly

char *BLEstrings[4];
char socketMessage[200];
uint64_t timeLastRecieved;




long bauds[] = {115200,9600,57600,38400,2400,4800,19200}; // common baud rates, when using HM-10 module with SoftwareSerial, try not to go over 57600
long BLEAutoBaud() {
  int baudcount=sizeof(bauds)/sizeof(long);
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<3; x++) {    // test at least 3 times for each baud
      Serial.print("Testing baud ");
      Serial.println(bauds[i]);
      ble.begin(bauds[i]);
      if (BLEIsReady()) {
        return bauds[i];
      }
    }
  }
  return -1;
}


boolean BLECmd(long timeout, char* command, char* temp, boolean verboseoutput = true) {
  long endtime;
  boolean found=false;
  endtime=millis()+timeout;     // 
  memset(temp,0,100);         // clear buffer
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
    delay(1);
  }  

  if (found) {            // response is available
    int i=0;
    while(ble.available()) {    // loop and read the data
      char a=ble.read();
      // Serial.print((char)a); // Uncomment this to see raw data from BLE
      temp[i]=a;          // save data to buffer
      i++;
      if (i>=BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
      delay(1);           // give it a 2ms delay before reading next character
    }
    if(verboseoutput) Serial.print("BLE reply    = ");
    if(verboseoutput) Serial.println(temp);
    return true;
  } else {
    if(verboseoutput) Serial.println("BLE timeout!");
    return false;
  }
}


boolean BLEIsReady() {
  BLECmd(timeout, "AT" ,buffer,false);    // Send AT and store response to buffer 
  if (strcmp(buffer,"OK")==0){    
    return true;
  } else {
    return false;
  }  
}

boolean BLEInit() {
  if (!BLECmd(timeout,"AT+RENEW",buffer)) return false;
  if (!BLECmd(timeout,"AT+BAUD4",buffer))
    BLEAutoBaud();
  if (!BLECmd(timeout+timeout+timeout,"AT+RESET",buffer)) {
    BLEAutoBaud();
    if (!BLEIsReady()) return false;  
  }    
  if (!BLECmd(timeout,"AT+ROLE1",buffer)) return false;
  if (!BLECmd(timeout,"AT+IMME1",buffer)) return false;
  if (!BLECmd(timeout,"AT+POWE2",buffer)) return false;
  if (!BLECmd(timeout,"AT+NOTI1",buffer)) return false;
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

boolean BLEIsConnected() {
  long endtime;
  static int counter=0;
    char timeString[30];
    ble.print("\a");     //signal to begin
    str_ll(CURRENT_TIME,timeString);
    ble.println(timeString);
    delay(200);
  
  if(! BLECmd(timeout,"AT+CON88C25512421A",buffer)) {  //88C25512421A
    if (counter==2) {   //no response, probably successfully connected
      counter=0;
      return true;
    }
    counter++;     //count timeouts
    return false;
  }
  if (strcmp(buffer,"OK+CONNA")==0){    
    endtime=millis()+20000;
    while(!ble.available()){
      if(millis()>endtime) return false;
      delay(5);
    }
    
    int i=0;
    char temp[BUFFER_LENGTH];
    while(ble.available()) {    // loop and read the data
      char a=ble.read();
      temp[i]=a;          // save data to buffer
      i++;
      if (i>=BUFFER_LENGTH) break;  // prevent buffer overflow, need to break
      delay(2);           // give it a 2ms delay before reading next character
    }
    temp[i]=0;
    if (strcmp(temp,"OK+CONN")==0)
      return true;
    else if (strNumerical(temp))
      return true;
    else {
      Serial.print("Bad connection response:  "); Serial.println(temp);
      return false;
    }
  }
  else if (strcmp(buffer,"OK+CONNE")==0) {
    delay(2000);
  }
  else if (strNumerical(buffer)) {
    Serial.println("REPSONSE CONTAINS DIGITS:   CONNECTED");
    return true;
  }
  else
    return false;
}
  

boolean parseBLE(char* input, dataPacket& data) {
  char *ptr = NULL;
  char* strings[5];
  byte index = 0;
  ptr = strtok(input, ";");  // takes a list of delimiters
  while(ptr != NULL and index < 5)
  {
      strings[index] = ptr;
      index++;
      ptr = strtok(NULL, ";");  // takes a list of delimiters
  }
  if (index == 5) {
    data.setID(strings[0]);
    data.setTimeStamp(strings[1]);
    data.setYaw(strings[2]);
    data.setPitch(strings[3]);
    data.setRoll(strings[4]);
    return true;
  }
  else
    return false;
}



long long atoll(const char* ptr) {
  long long result = 0;
  while (*ptr && isdigit(*ptr)) {
    result *= 10;
    result += *ptr++ - '0';
  }
  return result;
}

#include <math.h>

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
  WiFi.mode(WIFI_AP);
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


void setup() {
    
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);
    BLEAutoBaud();

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WIFI_Connect();
   
    if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return;
    }
    if (client.connected()) {
        client.send("connection", "message", "Connected !!!!");
    }
    
    //getTime();
    if (client.connected()) {
      client.send("gtime", "message", "Send Time");
      reqSent=millis();
      while (!timeSet)
      {
        if (client.monitor() and RID == "rtime")
        {
          timeOffset=millis();
          roundTripTime=timeOffset-reqSent;
          recievedTime=atoll(Rcontent.c_str());
          timeSet=true;
          Serial.println("GOT TIME");
          Serial.println(Rcontent);
          char ser[100];
          str_ll(CURRENT_TIME,ser);
          Serial.println(ser);
        }
      }
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

int i=0;
  
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
      WIFI_Connect();
    }


  
  if(millis() - timeLastRecieved > CONNECTION_TIMEOUT) {
    Serial.println("Reconnecting BT module");
    while (! BLEIsConnected()) {
      delay(500);
      Serial.print(".");
    }
    timeLastRecieved=millis();
  }
  /*
  if(Serial.available()) {
    ble.write(Serial.read());
  }
  */
  
  
  while(ble.available()) {    // loop and read the data
    char a=ble.read();
    //Serial.print(a);
    if (a=='\n') {
      //end of data packet
      blebuf[i]=0;
      if (parseBLE(blebuf, data)) {
        //SEND data
        chunk.addData(data);
        if (chunk.isFull()) {  //chunk ready
          if(client.connected()) {
            Serial.println(chunk.toJSON());
            createJSONString(JSONData, data);
            client.sendJSON("data chunk", JSONData);
          }
          chunk.empty();
        }
      } else {
        //ERROR Parsing
        Serial.print("ERROR PARSING:  ");Serial.println(blebuf);
      }
      //memset(blebuf,0,100);         // clear buffer
      blebuf[0]=0;
      i=0;
      timeLastRecieved=millis();
      continue;
    }
    else if (a=='\r')
      continue;
    blebuf[i]=a;          // save data to buffer
    i++;
    if (i>=BUFFER_LENGTH) {
      blebuf[0]=0;
      i=0;
      Serial.println("Buffer Overflow!!");
      break;  // prevent buffer overflow, need to break     
    }
  }
  /*
   if (client.monitor())
  {
    lastreply = millis(); 
    Serial.println(RID);
    if (RID == "rtime" && Rname == "time")
    {
       Serial.println("TIME");
    }
  }
  */
  
  if((millis() - heartbeatTimestamp) > HEARTBEAT_INTERVAL and client.connected()) {
      heartbeatTimestamp = millis();
      // socket.io heartbeat message
      if (client.connected())
        client.heartbeat(0);
  }


  /*
    if(isConnected) {

        uint64_t now = millis();

//        if(now - messageTimestamp > MESSAGE_INTERVAL) {
//            messageTimestamp = now;
//            // example socket.io message with type "messageType" and JSON payload
//            
//            //webSocket.sendTXT("42[\"new data\",{\"note\":\"another\"}]");
//            webSocket.sendTXT(createSocketMessage("100","100","100","100").c_str());
//        }
        if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
            heartbeatTimestamp = now;
            // socket.io heartbeat message
            webSocket.sendTXT("2");
        }
    }
    */
    
    
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



boolean createSocketMessage(char* output, const char* Time,const char* Yaw,const char* Pitch,const char* Roll){
  strcpy(output, "42[\"new data\",{\"Time\":\"");
  strcat(output, Time);
  strcat(output, "\",\"Yaw\":\"");
  strcat(output, Yaw);
  strcat(output, "\",\"Pitch\":\"");
  strcat(output, Pitch);
  strcat(output, "\",\"Roll\":\"");
  strcat(output, Roll);
  strcat(output, "\"}]");
  return true;  
  //return "42[\"new data\",{\"Time\":\"" + Time + "\",\"Yaw\":\"" + Yaw + "\",\"Pitch\":\"" + Pitch + "\",\"Roll\":\"" + Roll + "\"}]";
}

