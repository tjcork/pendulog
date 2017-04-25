/*
 * 
 *
 *
 * BLE CREDITS
 * https://github.com/jpliew/BLEShieldSketch/blob/master/HM_10_Test.ino
 */
 
//===============================================================================

#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <math.h>

#include "BLE.h"
#include "containers.h"

//===============================================================================

#define NETWORK_HOME
//#define NETWORK_BRISTOL
//#define NETWORK_LAPTOP

//#define SOCKET_IO_SERVER
#define WS_WEBSOCKET_SERVER


#define MAX_FRAME_LENGTH (64)       // Here we define a maximum framelength to 64 bytes. Default is 256.
#define CALLBACK_FUNCTIONS (1)
#define HEARTBEAT_INTERVAL (20000)

#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(4))

#define USE_SERIAL Serial
#define CONNECTION_TIMEOUT (10000)

#define JSON_STRING_LENGTH (250)

//===============================================================================

WebSocketsClient webSocket;

char host[] = "192.168.0.30";
int port = 3000;

#ifdef NETWORK_LAPTOP  
const char* ssid = XXXXX;
const char* password = XXXXX;
#endif
#ifdef NETWORK_BRISTOL
const char* ssid = XXXXX;
const char* password = XXXXX;
#endif
#ifdef NETWORK_HOME
const char* ssid = XXXXX;
const char* password = XXXXX;
#endif

//===============================================================================

bool isConnected = false;

uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;
uint64_t lastreply=0;


unsigned long reqSent;
unsigned long roundTripTime;

unsigned long currentID=0;
unsigned long lastID=0;

bool waiting = false;

uint64_t timeLastReceived;

PacketType packetType;
PacketRecovery recovery;
DataPacket data;
DataChunk chunk;
Configuration config;

//===============================================================================

void str_ll(uint64_t value, char* string);
void createJSONString(char * JSONOutput, DataPacket& data);
bool createSocketMessage(char* output, const char* Time,const char* Yaw,const char* Pitch,const char* Roll);


//===============================================================================

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

//===============================================================================

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
              
              #ifdef SOCKET_IO_SERVER
                // socket.io upgrade confirmation message (required)
                webSocket.sendTXT("5");
              #endif
              

            }
            break;
        case WStype_TEXT:
          {
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            
#ifdef SOCKET_IO_SERVER
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
                      receivedTime=atoull(RContent.c_str());
                      timeSet=true;
                    }
                }
              }
#endif  //SOCKET_IO_SERVER

#ifdef WS_WEBSOCKET_SERVER
            DynamicJsonBuffer dJBuffer;
            JsonObject& msg = dJBuffer.parseObject(payload);
            if (msg.success()) {
              const char * type = msg["type"];
              
              if (strcmp(type,"config")==0) {
                if (!config.timeSet) {
                    config.receivedTime=msg["time"];
                    config.timeOffset=eventTime;
                    roundTripTime=config.timeOffset-reqSent;
                    config.timeSet=true;
                }
                if (!config.idSet) {
                  config.initialID=msg["id"]; 
                  config.idSet=true;                 
                }
                config.configReceived=true;
              }
            }
#endif //WS_WEBSOCKET_SERVER

            break;
          }
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);
            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

//===============================================================================

/*
void createJSONString(char * JSONOutput, DataPacket& data) {
  StaticJsonBuffer<SENSORDATA_JSON_SIZE> jsonBuffer;
  JsonObject& JSON = jsonBuffer.createObject();
  JSON["T"]=data.Time;
  JSON["Y"]=data.Yaw;
  JSON["P"]=data.Pitch;
  JSON["R"]=data.Roll;
  JSON.printTo(JSONOutput,JSON_STRING_LENGTH); 
  return;
}
*/

void createSocketMessage(char* output, const char* messageType, const char* JSONString){
  strcpy(output, "42[\"");  strcat(output,messageType);  strcat(output,"\",");  strcat(output,JSONString);  strcat(output, "]");
}

//===============================================================================

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
    
#ifdef WS_WEBSOCKET_SERVER
    webSocket.begin("192.168.0.30", 3001);
#endif
#ifdef SOCKET_IO_SERVER
    webSocket.beginSocketIO("192.168.0.30", 3000);
#endif

    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);
       
    //getTime();
    Serial.println("Retrieving config..");
    while(!config.configReceived) {
      if (isConnected) {
        
#ifdef WS_WEBSOCKET_SERVER
        webSocket.sendTXT("{\"type\":\"getconfig\"}");
#endif
#ifdef SOCKET_IO_SERVER
        webSocket.sendTXT("42[\"gtime\",\"Time Please\"]");
#endif

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

    BLERequestConnection();
    while (! BLEIsConnected()) {
      delay(2000);
      Serial.print(".");
      BLERequestConnection();
    }
    Serial.println("BT CONNECTED");
    delay(500);
    config.send();
    
    timeLastReceived=millis();
}

//===============================================================================

void loop() {

  if (BLEAvailable()) {
    if (parseBLE(packetType,data,waiting)) {
      currentID=data.getID();
      if (recovery.recover(currentID)) {
         Serial.print("RECOVERED  :  "); Serial.println(currentID);
      }
      else {
        unsigned long numMissing = currentID-lastID-1;
        if (numMissing < 5) {
          //PACKET LOSS
          for (int n=1; n <= numMissing; n++) {
            unsigned long missed = lastID+n;
            Serial.print("Packet missed:    "); Serial.println(missed);
            requestResendPacket(missed);
            recovery.addMissing(missed);
          };
        }
        else {
         lastID=currentID; 
        }
        chunk.addData(data);
        if (chunk.isFull()) {  //chunk ready
          if(isConnected) {
            webSocket.sendTXT(chunk.toJSON());
          }
          chunk.empty();
        }
        lastID=currentID;
      }
    }
    else if (!waiting) {
      //PACKET ERROR
      unsigned long missed=lastID+1;
      Serial.print("PACKET ERROR:    "); Serial.println(missed);
      requestResendPacket(missed);
      recovery.addMissing(missed);
      lastID=lastID+1;
    }
    timeLastReceived=millis();
  }
  
  if((millis() - heartbeatTimestamp) > HEARTBEAT_INTERVAL and isConnected) {
      heartbeatTimestamp = millis();
      // heartbeat message
      webSocket.sendPing();
   } 
   
   if(millis() - timeLastReceived > CONNECTION_TIMEOUT) {
    Serial.println("Reconnecting BT module");
    while (! BLEIsConnected()) {
      BLEAutoBaud();
      delay(500);
    }
    timeLastReceived=millis();
  }
  if (WiFi.status() != WL_CONNECTED) {
      WIFI_Connect();
  }
}

//===============================================================================


unsigned long long int strtoull(const char* str, char** endptr, int base) {
  unsigned long long result = 0;
  while (*str && isdigit(*str)) {
    result *= 10;
    result += *str++ - '0';
  }
  return result;  
}

