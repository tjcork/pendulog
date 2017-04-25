/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */
#include <ESP8266WiFi.h>
#include <SocketIOClientv1.h>
#include <ArduinoJson.h>


StaticJsonBuffer<200> jsonBuffer;


SocketIOClient client;

const char* ssid = XXXXX;
const char* password = XXXXX;


char host[] = "192.168.0.30";
int port = 3000;
extern String RID;
extern String Rname;
extern String Rcontent;

unsigned long previousMillis = 0;
long interval = 10;
unsigned long lastreply = 0;
unsigned long lastsend = 0;
String JSON;
JsonObject& root = jsonBuffer.createObject();
#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(4))
#define JSON_STRING_LENGTH 250

struct dataPacket {
   char* Time;
   char* Yaw;
   char* Pitch;
   char* Roll;
};

dataPacket data;
char message[150];



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





void setup() {

  data.Time="0";
data.Yaw="55";
data.Pitch="55";
data.Roll="55";

  root["sensor"] = "gps";
  root["time"] = 1351824120;
  JsonArray& data = root.createNestedArray("data");
  data.add(double_with_n_digits(48.756080, 6));
  data.add(double_with_n_digits(2.302038, 6));
  root.printTo(JSON);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
if (client.connected())
  {
    client.send("connection", "message", "Connected !!!!");
  }
}

int i=0;
char istr[15];

void loop() {
unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    //client.heartbeat(0);
    sprintf(istr,"%d",i);
    data.Time=istr;
    createJSONString(message,data);
    client.sendJSON("new data", message);
    i++;
    lastsend = millis();
  }
  if (client.monitor())
  {
    lastreply = millis(); 
    Serial.println(RID);
    if (RID == "atime" && Rname == "time")
    {
      Serial.print("Time is ");
      Serial.println(Rcontent);
    }
  }
}
