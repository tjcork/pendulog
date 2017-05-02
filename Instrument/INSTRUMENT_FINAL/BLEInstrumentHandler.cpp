#include "BLEInstrumentHandler.h"


unsigned long timeout=1000; // BLE HM-10 command timeout time

void BLEInstrumentHandler::initialise() {
  
  autoBaud(500);
  breakConnection();
  char temp[10];
//  BLECmd(timeout,"AT+RENEW",temp,10);
  delay(200);
  if (!isReady())
    autoBaud();     //incase instant baud change
  BLECmd(timeout,"AT+BAUD4",temp,10);
  delay(200);
  if (!isReady())
    autoBaud();     //incase instant baud change
  delay(200);
//  BLECmd(5000,"AT+RESET",temp,10);
  delay(200);
  autoBaud();    
  if (!BLECmd(timeout,"AT+POWE2",temp,10)) return;// false;
  if (!BLECmd(timeout,"AT+NOTI0",temp,10)) return;// false;
  delay(1000);
  autoBaud(500);
  return;
}

void BLEInstrumentHandler::begin() {
  USE_SERIAL.begin(BAUD_RATE); 
}





void BLEInstrumentHandler::breakConnection() {
  USE_SERIAL.print("AT");
  delay(500);
  USE_SERIAL.print("AT");
  delay(500);
}


bool BLEInstrumentHandler::autoBaud(int delaylen) {
  long bauds[] = {baud0,baud1,baud2,baud3,baud4};  //Skip: BAUD1,BAUD2,BAUD3, // common baud rates, for HM-10 module with SoftwareSerial, try not to go over 57600
  int baudcount=sizeof(bauds)/sizeof(long);
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<3; x++) {    // test at least 4 times for each baud
      USE_SERIAL.begin(bauds[i]);
      if (isReady()) {
        Serial.print("BLE BAUD:  ");Serial.println(bauds[i]);
        return true;
      }
      delay(delaylen);
    }
  }
  Serial.println("BLE BAUD:  UNKNOWN");
  return false;
}


//===============================================================================

bool BLEInstrumentHandler::BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput) {
  long endtime;
  bool found=false;
  endtime=millis()+timeout;
  //memset(temp,0,len);         // clear buffer
  found=true;
  if(verboseoutput) Serial.print("Arduino send = ");
  if(verboseoutput) Serial.println(command);
  USE_SERIAL.print(command);
  
  // wait till either a response is received or timeout
  // a timeout waits until the full response could have been recieved
  
  while(!USE_SERIAL.available()){
    if(millis()>endtime) {      // timeout, break
      found=false;
      break;
    }
    delay(2);
  }  

  if (found) {            // response is available
    int i=0;
    while(USE_SERIAL.available()) {    // loop and read the data
      char a=USE_SERIAL.read();
      temp[i]=a;          // save data to buffer
      i++;
      if (i>=len-1) break;  // prevent buffer overflow, need to break
      delay(1);           // give 1ms delay before next character
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

//===============================================================================

bool BLEInstrumentHandler::isReady() {
  char temp[4];     //catch first additional character
  BLECmd(500, "AT",temp,4,false);    // Send AT and store response to buffer 
  if (strcmp(temp,"OK")==0){    
    return true;
  } else {
    delay(100);
    return false;
  }  
}

//=========================================================================================

bool BLEInstrumentHandler::monitor() {
  static unsigned long timeout=millis();
  if(!configuration->isSet) {
    if (millis() > timeout) {
      noConfigHandler();
      timeout = millis() + CONFIG_REQUEST_INTERVAL;
    }    
  }
  switch(handlerState) {
    case IDENTIFY_PACKET:
      typeDetect();
      break;
    case NO_CONFIG:
      noConfigHandler();
      handlerState=IDENTIFY_PACKET;
      break;
    case NEW_CONFIG:
      Serial.println("New configuration recieved");
      if (configPacketHandler()) {
        //successful config recieved
        Serial.println("config set true");
        
      }
      configuration->isSet=true;
      break;
    case CONNECTION_PING:
      Serial.println("Connection Ping");
      if (packetTerminate()) {
        connectionPingHandler();
        handlerState=IDENTIFY_PACKET;
      }
      break;
    case RESEND_REQUEST:
      resendRequestHandler();
      break;
    default:
      handlerState=IDENTIFY_PACKET;
  }
  return false;
}

//=========================================================================================

bool BLEInstrumentHandler::typeDetect () {
  //returns false if USE_SERIAL. not available
  //used for connection timeout
    static bool state=false;
    static HandlerStates upgradeTo=IDENTIFY_PACKET;
    //maintain state such that command character must be followed by delimiter

    if(USE_SERIAL.available()) {
      char a=USE_SERIAL.read();
      //Serial.print(a);
      if (state and a==DELIMITER_CHAR) {
        state=false;
        handlerState=upgradeTo;
        //Serial.printf("Success, upgrade to state: %d\n", handlerState);
        return true;
      }
      else {
        if (a==RESEND_REQUEST_CHAR) {
          upgradeTo=RESEND_REQUEST;
        }
        else if (a==CONFIG_SET_CHAR) {
          upgradeTo=NEW_CONFIG;
        }
        else if (a==CONNECTION_PING_CHAR) {
          upgradeTo==CONNECTION_PING;
        }
        else {
          handlerState=IDENTIFY_PACKET;
          state=false;
          return true;
        }
        state=true;
        return true;
      }
    }
    return false;
}


//=========================================================================================

void BLEInstrumentHandler::noConfigHandler() {
  static unsigned long timeout;
  send(CONFIG_REQUEST_CHAR,0,NULL);
  Serial.println("Config requested");
}

//=========================================================================================

bool BLEInstrumentHandler::packetTerminate() {
  //collect \r and \n characters and maintain state or pass errors back to initial state
  char a;
  if (USE_SERIAL.available()) {
    a=USE_SERIAL.read();
    //Serial.print("PACKET TERMINATE: "); Serial.println(a);
    if (a=='\n') {
      return true;
    }
    if (a=='\r') {
      return false;
    }
    else 
      handlerState=IDENTIFY_PACKET;
  }
  return false;  
}

//=========================================================================================

void BLEInstrumentHandler::connectionPingHandler() {
  send(CONNECTION_PONG_CHAR,0,NULL);
}

//=========================================================================================
#define ID_SIZE (12)


void BLEInstrumentHandler::resendRequestHandler() {
  static char reqbuffer[ID_SIZE];
  static unsigned int index = 0;
  bool exit = false;
  char a;
  
  while (USE_SERIAL.available()) {
    a=USE_SERIAL.read();
    if (a=='\r') continue;
    if (a=='\n') {
      if(index < ID_SIZE) {
        reqbuffer[index]='\0';
        if (store.findID(atoul(reqbuffer))) {
          Serial.print("Recovered message: "); Serial.println(store.foundMsg);
          USE_SERIAL.println(store.foundMsg);  //send
        }
      }
      exit = true;
      break;
    }
    if (isdigit(a)) {
      reqbuffer[index]=a;
      index++;      
    }
    else { 
      exit=true;
      break;
    }   
    if (index >= ID_SIZE) {
      exit=true;
      break;
    }
  }
  if (exit) {
    index=0;
    handlerState=IDENTIFY_PACKET;  
  }
}

//=========================================================================================
  
bool BLEInstrumentHandler::configPacketHandler() {
  char a;
  static unsigned int fieldIndex=ID_CONFIG_FIELD;
  static unsigned int index=0;
  static unsigned int schemaIndex=0;
  static char configBuffer[22];
  bool exit=false;
  bool retv=false;
  
  while (USE_SERIAL.available()) {
    a=USE_SERIAL.read();
    //Serial.print(a);
    switch(fieldIndex) {
      case ID_CONFIG_FIELD:
        if (a==DELIMITER_CHAR) {
          configBuffer[index]='\0';
          configuration->setID(configBuffer);
          fieldIndex=TIME_CONFIG_FIELD;
          index=0;
          break;
        }
        if (!isdigit(a)) {
          exit=true;
          break;         
        }
        configBuffer[index++]=a;
        if (index >= ID_SIZE) {
          exit=true;
          break;
        }
        break;
        
      case TIME_CONFIG_FIELD:      
        if (a==DELIMITER_CHAR) {
          configBuffer[index]='\0';
          configuration->setTIME(configBuffer);
          fieldIndex=SCHEMA_CONFIG_FIELD;
          index=0;
          break;
        }
        if (!isdigit(a)) {
          exit=true;
          break;         
        }
        configBuffer[index++]=a;
        if (index >= TIME_SIZE) {
          exit=true;
          break;
        }
        break;
        
      case SCHEMA_CONFIG_FIELD:      //;1.2.3.4.5.6.\r\n
        Serial.print(a);
        if (a=='\r' or a==DELIMITER_CHAR)
          continue;
        if (a=='\n') {
          configuration->fieldCount=schemaIndex;
          configuration->setSCHEMA();
          schemaIndex=0;
          exit=true;
          retv=true;      //success
          break;
        }
        if (a=='.') {
         configBuffer[index]='\0';
         index=0;
         configuration->fields[schemaIndex++]=atoi(configBuffer);
         continue; 
        }
        if (!isdigit(a)) {
          exit=true;
          break;         
        }
        configBuffer[index++]=a;
        if (index >= SCHEMA_SIZE) {
          exit=true;
          break;
        }
        break;
    }
    
    if (exit) {
      index=0;
      fieldIndex=ID_CONFIG_FIELD;  
      handlerState=IDENTIFY_PACKET;
      break;
    }
  }
  return retv;
}

//=========================================================================================


void BLEInstrumentHandler::send(char requestType, char* sendField) {
  USE_SERIAL.print(requestType);
  USE_SERIAL.print(DELIMITER_CHAR);
  USE_SERIAL.print(sendField);
  USE_SERIAL.println();
  return;  
}

void BLEInstrumentHandler::send(char requestType, unsigned long sendField) {
  USE_SERIAL.print(requestType);
  USE_SERIAL.print(DELIMITER_CHAR);
  USE_SERIAL.print(sendField);
  USE_SERIAL.println();
  return;  
}

void BLEInstrumentHandler::send(char requestType, unsigned int numFields, char* sendFields[]) {
  USE_SERIAL.print(requestType);
  USE_SERIAL.print(DELIMITER_CHAR);
  for (int i=0; i < numFields; i++) {
    USE_SERIAL.print(sendFields[i]);
    USE_SERIAL.print(DELIMITER_CHAR);
  }
  USE_SERIAL.println();
  return;  
}

