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
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>


#include <WebSocketClient.h>

WebSocketClient webSocket;
WiFiClient client;

// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64
#define CALLBACK_FUNCTIONS 1
#define MESSAGE_INTERVAL 10000
#define HEARTBEAT_INTERVAL 25000
uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;

 
const char* ssid = XXXXX;
const char* password = XXXXX;


#include <ArduinoJson.h>

//ESP8266WiFiMulti WiFiMulti;

char host[] = "192.168.0.30";
int port = 3000;


#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(4))

struct dataPacket {
   char* Time;
   char* Yaw;
   char* Pitch;
   char* Roll;
};


#define JSON_STRING_LENGTH 250
dataPacket data;
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
    ble.begin(115200);
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
  char* strings[4];
  byte index = 0;
  ptr = strtok(input, ";");  // takes a list of delimiters
  while(ptr != NULL and index < 4)
  {
      strings[index] = ptr;
      index++;
      ptr = strtok(NULL, ";");  // takes a list of delimiters
  }
  if (index == 4) {
    data.Time=strings[0];
    data.Yaw=strings[1];
    data.Pitch=strings[2];
    data.Roll=strings[3];
    return true;
  }
  else
    return false;
}


/*
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
                isConnected = true;

			    // send message to server when Connected
                // socket.io upgrade confirmation message (required)
				webSocket.sendTXT("5");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }
}
*/

void WIFI_Connect()
{
  WiFi.disconnect();
  Serial.println("Connecting WiFi...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay ( 250 );
      digitalWrite(2,0);
      Serial.print ( "." );
      delay ( 250 );
    }
  }
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



//    WiFiMulti.addAP("VM890434-2G", "eqaeuzwm");
//    WiFiMulti.addAP("Bristol-Research-Net","na:wiat:nieb:ottn:awi");
/*
    WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
*/
    WIFI_Connect();
    
/*    
const char* ssid     = "Bristol-Research-Net";
const char* password = "na:wiat:nieb:ottn:awi";
  WiFi.begin(ssid, password);

  int ct=0;
  while (WiFi.status() != WL_CONNECTED) {
    if (ct==10) {
      WiFi.begin(ssid, password);
      ct=0;
    }
    ct++;
    delay(500);
    Serial.print(".");
  }
    Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
*/



    
//    Serial.println("CONNECTED");


    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        while(1);
      }
   // Handshake with the server
    webSocket.path = "/";
    webSocket.host = host;
  
    if (webSocket.handshake(client, true)) {
      Serial.println("Handshake successful");
      // socket.io upgrade confirmation message (required)
      webSocket.sendData("5");
    } else {
      Serial.println("Handshake failed.");
      //while(1) {
        // Hang on failure
      //}
    }
webSocket.sendData("5");
    /*
    webSocket.beginSocketIO("192.168.0.30", 3000);
    //webSocket.beginSocketIO("ws://echo.websocket.org",80);  
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);
  */

    Serial.println("Starting BT module");
    while (! BLEIsReady()) {
      delay(100);
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
    ble.print("PENDSTART");     //signal to begin
    
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
        if(client.connected()) {
          Serial.println("TRYING TO SEND");
/*
          createJSONString(JSONData, data);
          webSocket.sendJSON("new data", JSONData);
*/
          //Serial.print("SENT: ");Serial.println(JSONData);
          //Serial.print(BLEstrings[0]);
          //createSocketMessage(socketMessage,data.Time,data.Yaw,data.Pitch,data.Roll);
          //webSocket.sendMessage(socketMessage);
          webSocket.sendData("42[\"new data\",{\"Time\":\"99\",\"Yaw\":\"99\",\"Pitch\":\"99\",\"Roll\":\"99\"}]");
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
  if (webSocket.monitor())
  {
    Serial.println(RID);
    if (RID == "initial points")
    {
      Serial.println(Rcontent);
    }
  }
  */
  
        if((millis() - heartbeatTimestamp) > HEARTBEAT_INTERVAL and client.connected()) {
            heartbeatTimestamp = millis();
            // socket.io heartbeat message
            webSocket.sendData("2");
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
  JSON["Time"]=data.Time;
  JSON["Yaw"]=data.Yaw;
  JSON["Pitch"]=data.Pitch;
  JSON["Roll"]=data.Roll;
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

