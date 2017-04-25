#include <ESP8266WiFi.h>

// Here we define a maximum framelength to 64 bytes. Default is 256.

#define MAX_FRAME_LENGTH 64

// Define how many callback functions you have. Default is 1.
#define CALLBACK_FUNCTIONS 1

#define MESSAGE_INTERVAL 1000
#define HEARTBEAT_INTERVAL 25000

#include <WebSocketClient.h>



#include <ArduinoJson.h>


char message[200];



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







WiFiClient client;
WebSocketClient webSocketClient;

uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;

void setup() {

  Serial.begin(115200);

  Serial.print("Connecting to Wifi");

  WiFi.begin(XXXXX,XXXXX);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // This is how you get the local IP as an IPAddress object
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to the websocket server
  if (client.connect("192.168.0.30", 3000)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = "/";
  webSocketClient.host = "192.168.0.30:3000";

  if (webSocketClient.handshake(client, true)) {
    Serial.println("Handshake successful");
    // socket.io upgrade confirmation message (required)
    webSocketClient.sendData("5");
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }
  }
}

int i=0;
char istr[20];

void loop() {
  String data;

  if (client.connected()) {

    webSocketClient.getData(data);

    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data);
    }

    uint64_t now = millis();

    if(now - messageTimestamp > MESSAGE_INTERVAL) {
        messageTimestamp = now;
        sprintf(istr,"%d",i);
        createSocketMessage(message,istr,"100","100","100");
        webSocketClient.sendData(message);
        i++;
        //webSocketClient.sendData("42[\"new data\",{\"Time\":\"99\",\"Yaw\":\"99\",\"Pitch\":\"99\",\"Roll\":\"99\"}]");
    }
    if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
        heartbeatTimestamp = now;
        // socket.io heartbeat message
        webSocketClient.sendData("2");
    }

  } else {

    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }

}


/*
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

*/
