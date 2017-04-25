/*
 * 
 *
 *
 * 
 */
 
//===============================================================================

//#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <math.h>

#include "BLEHandler.h"
#include "DataHandler.h"
#include "configurator.h"


//===============================================================================



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

char host[] = "178.62.89.40";
int port = 3000;

const char* ssid = XXXXX;
const char* password = XXXXX;


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

//FieldIdentifiers defaultSchemaFormat[]={ID,TI,YW,PT,RL};


//PacketType packetType;
//PacketRecovery recovery;
//DataPacket data;
//DataChunk chunk;


FieldIdentifiers defaultSchemaFormat[8]={ID,TI,YW,PT,RL,AX,AY,AZ};
//FieldIdentifiers defaultSchemaFormat[15]={ ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, GX, GY, GZ};

Schema loadedSchema = Schema(8, defaultSchemaFormat);  

Configuration config(&loadedSchema);
BLEHandler bt(&loadedSchema);
DataHandler dataHandler(&webSocket);


//===============================================================================

void str_ll(uint64_t value, char* string);
//void createJSONString(char * JSONOutput, DataPacket& data);
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
              //--------------------------------------------------
              if (strcmp(type,"config")==0) {
                    
                    config.receivedTime=atoull((const char*)msg["time"]);
                    config.timeOffset=eventTime;
                    roundTripTime=config.timeOffset-reqSent;
                    config.timeSet=true;
                
                  config.initialID=msg["id"]; 
                  config.idSet=true;
                  config.send();          
                config.configReceived=true;
              }
              //--------------------------------------------------
              if (strcmp(type,"newconfig")==0) {
                Serial.println("New config recieved from server");
                 // parse config packet, example:
                 //{"samplerate":"20","numchunks":"10","schema":["0","1","2","3","4"],"mode":"Normal","eulercalc":"true","errorresend":"false","type":"newconfig"}
                 int schemaFieldCount=msg["schema"].size();
                 Serial.println(schemaFieldCount);
                 for(int i = 0; i < schemaFieldCount; i++) {
                  //Serial.print((int)msg["schema"][i]);
                  loadedSchema.spec[i]=(FieldIdentifiers)(int)msg["schema"][i];
                  Serial.print(loadedSchema.spec[i]);
                 }
                 loadedSchema.fieldCount=schemaFieldCount;
                 config.send();               
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
    WIFI_Connect();
    
    webSocket.begin(host,port);

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
    bt.begin();
    config.send();
    
    timeLastReceived=millis();
}

//===============================================================================

void loop() {
  if (bt.monitor()) {
    dataHandler.push(loadedSchema);
  }
  if (bt.recovered()) {
    Serial.println("Recovered");
  }
  
  /*
  if (BLEAvailable()) {
    if (parseBLE(packetType,data,waiting)) {
      
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
  */
  /*
  if((millis() - heartbeatTimestamp) > HEARTBEAT_INTERVAL and isConnected) {
      heartbeatTimestamp = millis();
      // heartbeat message
      webSocket.sendPing();
   } 
   */

   
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

