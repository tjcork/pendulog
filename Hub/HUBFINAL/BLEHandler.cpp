#include "BLEHandler.h"

SoftwareSerial ble(14, 12);        //RX|TX





unsigned long timeout=1000;

BLEHandler::BLEHandler(Schema * d)
{
  data=d;
  errorHandler= new ErrorHandler(this);  
}
BLEHandler::~BLEHandler() {
  delete errorHandler;
}



void BLEHandler::begin() {
  autoBaud();
  char temp[10];
//  BLECmd(timeout,"AT+RENEW",temp,10);
  delay(200);
  if (!isReady())
    autoBaud();     //incase instant baud change
  BLECmd(timeout,"AT+BAUD3",temp,10);
  delay(200);
  if (!isReady())
    autoBaud();     //incase instant baud change
  delay(200);
//  BLECmd(5000,"AT+RESET",temp,10);
  delay(200);
    autoBaud();    
  if (!BLECmd(timeout,"AT+ROLE1",temp,10)) return;//false;
  if (!BLECmd(timeout,"AT+IMME1",temp,10)) return;// false;
  if (!BLECmd(timeout,"AT+POWE2",temp,10)) return;// false;
  if (!BLECmd(timeout,"AT+NOTI0",temp,10)) return;// false;
  return;
}


bool BLEHandler::autoBaud() {
  long bauds[] = {BAUD0,BAUD3,BAUD4};  //Skip: BAUD1,BAUD2,BAUD3, // common baud rates, for HM-10 module with SoftwareSerial, try not to go over 57600
  int baudcount=sizeof(bauds)/sizeof(long);
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<3; x++) {    // test at least 4 times for each baud
      ble.begin(bauds[i]);
      if (isReady()) {
        Serial.print("BLE BAUD:  ");Serial.println(bauds[i]);
        return true;
      }
      delay(200);
    }
  }
  Serial.println("BLE BAUD:  UNKNOWN");
  return false;
}



/*
commandHandler(const char * command) {
  unsigned long endtime;
  bool found=false;
  //memset(temp,0,len);         // clear buffer

  static commandHandlerState

  switch(commandHandlerState) {
    case COMMAND_SEND:
      ble.print(command);
      endtime=millis()+timeout;
      commandHandlerState;
      break;
    case COMMAND_WAIT;
      int i=0;
      if(ble.available()){
         char a=ble.read();
          // Serial.print((char)a); // Uncomment this to see raw data from BLE
          temp[i]=a;          // save data to buffer
          i++;
          if (i>=len-1) break;  // prevent buffer overflow, need to break
      }
      temp[i]=0;
    
      if(millis()>endtime) {      // timeout, break
        commandHandlerState=NO_RESPONSE;
      break;
    }
    case NO_RESPONSE:

      break;
        
  }
  
*/



//=========================================================================================

bool BLEHandler::monitor() {
  static unsigned long timeout=0;
  unsigned long receiveTimeout = 5000;
  switch(handlerState) {
    case IDENTIFY_PACKET:
      if (typeDetect()) {
        //recieved something, so must be connected else command response
        timeout=millis()+receiveTimeout;
      }
      else if (millis() > timeout) {
        Serial.println("CONNECTION LOST"); 
        handlerState=NO_CONNECTION;
      }
      break;
    case NO_CONNECTION:
      noConnectionHandler();
      timeout=millis()+receiveTimeout;
      handlerState=IDENTIFY_PACKET;
      break;
    case DATA_PACKET:
      if (dataPacketHandler()) {
        fieldIndex=0;
        stringIndex=0;
    /*   
        if (errorHandler->success(data)) {
          //set rescued flag
          lostAndRecovered=true;
          return false;
        }
        else
          return true;     
      }
    */
        timeout=millis()+receiveTimeout;
        handlerState=IDENTIFY_PACKET;
        return true;
      }
      else if (millis() > timeout) {
        Serial.println("ERROR - NO SUCCESSFUL PACKET RECIEVED IN 5s - CONNECTION LOST"); 
        handlerState=NO_CONNECTION;
      }
      break;
    case CONNECTION_PONG:
      if (packetTerminate()) {
        connectionPongHandler();
      }
      break;
    case CONFIG_REQUEST:
      Serial.println("Config request received");
      if (packetTerminate())
        configRequestHandler();
      break;    
    case ERROR_PACKET:
      Serial.println("errorState");
      //errorHandler->error(fieldIndex,*data);
    default:
        fieldIndex=0;
        stringIndex=0;
        handlerState=IDENTIFY_PACKET;
  }
  return false;
}



bool BLEHandler::recovered() {
  if (lostAndRecovered==true) {
    lostAndRecovered=false;
    return true;
  }
  return false;
}



//=========================================================================================

typedef enum ConnectionHandlerStates {
  PING_SEND,
  CONNECTION_REQUEST
};

void BLEHandler::noConnectionHandler() {
  static unsigned long timeout;
  static ConnectionHandlerStates connectionHandlerState=PING_SEND;
    
  switch(connectionHandlerState) {
    case PING_SEND:
      send(PING_CHAR,0,NULL);
      connectionHandlerState=CONNECTION_REQUEST;
      break;

    case CONNECTION_REQUEST:
      ble.print("AT+CON88C25512421A");
      connectionHandlerState=PING_SEND;
      Serial.println("CONNECT REQUEST SENT");
      break;
  }
}

//=========================================================================================


void BLEHandler::send(char requestType, char* sendField) {
  ble.print(requestType);
  ble.print(DELIMITER_CHAR);
  ble.print(sendField);
  ble.println();
  return;  
}

void BLEHandler::send(char requestType, unsigned long sendField) {
  ble.print(requestType);
  ble.print(DELIMITER_CHAR);
  ble.print(sendField);
  ble.println();
  return;  
}

void BLEHandler::send(char requestType, unsigned int numFields, char* sendFields[]) {
  ble.print(requestType);
  ble.print(DELIMITER_CHAR);
  for (int i=0; i < numFields; i++) {
    ble.print(sendFields[i]);
    ble.print(DELIMITER_CHAR);
  }
  ble.println();
  return;  
}

//=========================================================================================

bool BLEHandler::typeDetect () {
  //returns false if ble. not available
  //used for connection timeout
    static bool state=false;
    static HandlerStates upgradeTo=IDENTIFY_PACKET;
    //maintain state such that command character must be followed by delimiter

    if(ble.available()) {
      char a=ble.read();
      //Serial.print(a);
      if (state and a==DELIMITER_CHAR) {
        state=false;
        handlerState=upgradeTo;
        //Serial.printf("Success, upgrade to state: %d\n", handlerState);
        return true;
      }
      else {
        if (a==DATA_CHAR) {
          upgradeTo=DATA_PACKET;
        }
        else if (a==CONFIG_REQUEST_CHAR) {
          Serial.println("CONFIG_REQUEST_CHAR");
          upgradeTo=CONFIG_REQUEST;
        }
        else if (a==CONNECTION_PONG_CHAR) {
          upgradeTo==CONNECTION_PONG;
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

bool BLEHandler::packetTerminate() {
  //collect \r and \n characters and maintain state or pass errors back to initial state
  char a;
  if (ble.available()) {
    a=ble.read();
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

void BLEHandler::configRequestHandler() {
  webSocket.sendTXT("{\"type\":\"getconfig\"}");
  //config.send();
  handlerState=IDENTIFY_PACKET;      
}

//=========================================================================================

void BLEHandler::connectionPongHandler() {
  connected=true;
  handlerState=IDENTIFY_PACKET;
}

//=========================================================================================
  
bool BLEHandler::dataPacketHandler() {
  char a;
  Field * currentField = data->getField(fieldIndex);
    
  while (ble.available()) {
    a=ble.read();
    Serial.print(a);
    if (a==DELIMITER_CHAR) {
      if(fieldIndex < data->fieldCount) {
        if(stringIndex < currentField->size) {
          currentField->string_value[stringIndex]='\0';
        }
        else {
          Serial.print("ERROR: Field too long");
          handlerState=ERROR_PACKET;
          break;
        }
        stringIndex=0;
        fieldIndex++;
        
        currentField=data->getField(fieldIndex);
        continue;
      }
      else {
        Serial.print("ERROR: Too many fields recieved");
        handlerState=ERROR_PACKET;
        break;
      }
    }
    
    if (a=='\r') continue;
    if (a=='\n') {
      if(stringIndex < currentField->size) {
        currentField->string_value[stringIndex]='\0';
      }
      else {
        Serial.print("ERROR: Field too long");
        handlerState=ERROR_PACKET;
        break;
      }
      if (fieldIndex+1==data->fieldCount)  {      
        //Serial.println("SUCCESS");
        handlerState=IDENTIFY_PACKET;
        return true;
      }
      else {
        Serial.print("ERRORRRR");
        Serial.printf("ERROR: Too few fields recieved index %d vs schema %d\n",fieldIndex,data->fieldCount);
        handlerState=ERROR_PACKET;
      }
      break;
    }
    
    switch(currentField->errorDetection) {
      case FLOAT_DETECT:
          if ((a == '-') or (a == '.'))
            break;
      case NUMERICAL_DETECT:
          if (isdigit(a))
            break;  
        default:
          Serial.printf("Incorrect character:   %c      in field:  %s\n", a, currentField->name);
          handlerState=ERROR_PACKET;
          return false;
    }
    if(stringIndex < currentField->size) {
      currentField->string_value[stringIndex]=a;
      stringIndex++;
    }
    else {
      Serial.printf("ERROR:  field size (%d) is larger than allowed %d",stringIndex,currentField->size);
      handlerState=ERROR_PACKET;
      break;
    }
  }
  return false;
}

//===============================================================================

bool BLEHandler::BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput) {
  long endtime;
  bool found=false;
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

//===============================================================================

bool BLEHandler::isReady() {
  char temp[4];     //catch first additional character
  BLECmd(500, "AT",temp,4,false);    // Send AT and store response to buffer 
  if (strcmp(temp,"OK")==0){    
    return true;
  } else {
    return false;
  }  
}

//===============================================================================

void requestResendPacket(unsigned long packetIDRequest) {
  static uint32_t lastRequested = millis();
  if (millis() - lastRequested > MIN_REQUEST_PERIOD) {
    ble.print(RESEND_CHAR);ble.print(DELIMITER_CHAR);ble.println(packetIDRequest);
    lastRequested=millis();
  }
  return;  
}




