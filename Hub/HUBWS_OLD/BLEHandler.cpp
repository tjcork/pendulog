#define DELIMITER_CHAR
#define DATA_CHAR
#define CONFIG_REQUEST_CHAR
#define CONNECTION_PONG_CHAR

enum ErrorDetection {
  NUMERICAL_DETECT,
  FLOAT_DETECT
}

enum FieldIdentifiers {
  ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, MX, MY, MZ, GX, GY, GZ
}

class Field {
  char name[3];
  unsigned int identifier;
  size_t size;
  char * string_value;
  ErrorDetection errorDetection;
  Field(const * char shortname, FieldIdentifiers fieldid, size_t stringsize = 9, ErrorDetection type = FLOAT_DETECT) {
    strncpy(name,shortname,3);
    identifier=fieldid;
    size=stringsize;
    string_value=new char[size];
    errorDetection=type;
  }
  ~Field() {
    delete [] string_value;
  }  
};


class Schema {
  Field fields[18];
  fields[ID] = Field("ID",ID,22,NUMERICAL);
  fields[TM] = Field("TM",TM,11,NUMERICAL);
  fields[YW] = Field("YW",YW,9);
  fields[PT] = Field("PT",PT,9);
  fields[RL] = Field("RL",RL,9);
  fields[QW] = Field("QW",QW,9);
  fields[QX] = Field("QX",QX,9);
  fields[QY] = Field("QY",QY,9);
  fields[QZ] = Field("QZ",QZ,9);
  fields[AX] = Field("AX",AX,9);
  fields[AY] = Field("AY",AY,9);
  fields[AZ] = Field("AZ",AZ,9);
  fields[MX] = Field("MX",MX,9);
  fields[MY] = Field("MY",MY,9);
  fields[MZ] = Field("MZ",MZ,9);
  fields[GX] = Field("GX",GX,9);
  fields[GY] = Field("GY",GY,9);
  fields[GZ] = Field("GZ",GZ,9);

  FieldIdentifiers spec[18];
  unsigned int fieldCount;
  Schema(unsigned int number, const FieldIdentifiers format[]) {
    fieldCount=number < 18 ? number : 18;
    for(int i=0; i < fieldCount or i < 18; i++)
      spec[i]=format[i];
  }
  setSchema(unsigned int number, const FieldIndentifiers format[]) {
    fieldCount=number < 18 ? number : 18;
    for(int i=0; i < fieldCount or i < 18; i++) 
      spec[i]=format[i];
  }
  Field * getField(int index) {
    return fields[spec[index]];
  }
};


enum HandlerStates {
  NO_CONNECTION,
  NO_PACKET,
  DATA_PACKET,
  CONNECTION_PONG,
  CONFIG_REQUEST,
  ERROR_PACKET
}


operatingSchema = Schema(5, {ID,TI,YW,PT,RL});

class BLEHandler {
  public:
    BLEHandler() {}
    SoftwareSerial ble(14, 12);        //RX|TX
    Schema data(5, {ID,TI,YW,PT,RL});
 `  void begin();
    bool connect();
    bool connected=false;
    bool update();    //true on successful data receive
    void send(char requestType, unsigned int numFields, char* Fields);
  private:
    unsigned long currentBaud;
    bool typeDetect();
    bool packetTerminate(PacketType type);
    HandlerStates handlerState = NO_CONNECTION;
    unsigned int fieldIndex=0;
    unsigned int stringIndex=0;
};


void BLEHandler::begin() {
  ble.begin(BAUD);
}

bool BLEHandler::update() {
  if (!connected)
    handlerState=NO_CONNECTION;
    
  switch(handlerState) {
    case NO_CONNECTION:
      noConnectionHandler();
      break;
    case NO_PACKET:
      typeDetect();
      break;
    case DATA_PACKET:
      if (dataPacketHandler()) {
        fieldIndex=0;
        stringIndex=0;
        handlerState=NO_PACKET;   
        return true;     
      }
      break;
    case CONNECTION_PONG:
      if (packetTerminate(CONNECTION_PACKET))
        connectionPongHandler();
      break;
    case CONFIG_REQUEST:
      if (packetTerminate(CONNECTION_PACKET))
        configRequestHandler();
      break;    
    case ERROR_PACKET:
    default:
        fieldIndex=0;
        stringIndex=0;
        handlerState=NO_PACKET;
  }
  return false;
}


noConnectionHandler() {
  static unsigned long timeout;
  unsigned long waitTime=1000;
  static bool pingSent = false;
  static connectionHandlerState;

  switch(connectionHandlerState) {
    case PING_SEND:
      send(PING_CHAR,0,NULL);
      timeout=millis()+waitTime;
      connectionHandlerState=PONG_WAIT;
      break;
    case PONG_WAIT:
      if (millis() > timeout) {
        //no response from connected device, likely not connected
        connectionHandlerState=CONNECTION_REQUEST;   
      }
      break;
    case CONNECTION_REQUEST:
      
    
  }
    else {
      BLECmd(timeout,"AT+CON88C25512421A",temp,10);
      }
  }
   
  
}


commandHandler() {
  unsigned long endtime;
  bool found=false;
  //memset(temp,0,len);         // clear buffer

  static commandHandlerState

  switch(commandHandlerState) {
    case COMMAND_SEND:
      ble.print(command);
      endtime=millis()+timeout;
      break;
  
  }



  
  if(verboseoutput) Serial.print("Arduino send = ");
  if(verboseoutput) Serial.println(command);
  
  
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
  

}



command() {
  
}
command(const char* command){
  long endtime;
  bool found=false;
  endtime=millis()+timeout;
  



  
bool BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput) {
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
  
}


void BLERequestConnection() {
  char temp[10];
  BLECmd(timeout,"AT+CON88C25512421A",temp,10);
  return; 
}

//===============================================================================

bool BLEIsConnected() {
  unsigned long starttime=millis(); 
  unsigned long timeout = 1000;
  PacketType type;
  DataPacket data;
  bool midPacket = false;
  
  while (millis()-starttime < timeout) {
    if (parseBLE(type, data, midPacket) and (type == CONNECTION_PACKET or DATA_PACKET))
      if (!midPacket)
         return true;
  }
  return false;
}


void send(char requestType, unsigned int numFields, char* sendFields[]) {
  ble.print(requestType);
  ble.print(DELIMITER_CHAR);
  for (int i=0; i < numFields; i++) {
    ble.print(sendFields[i]);
    ble.print(DELIMITER_CHAR);
  }
  ble.println();
  return;  
}

bool BLEHandler::typeDetect () {
    static state=false;
    //maintain state such that command character must be followed by delimiter
    char a=ble.read();
    if (state and a==DELIMITER_CHAR) {
      state=false
      return true;
    }
    else {
      if (a==DATA_CHAR) {
        handlerState=DATA_PACKET;
      }
      else if (a==CONFIG_REQUEST_CHAR) {
        handlerState=CONFIG_REQUEST;
      }
      else if (a==CONNECTION_PONG_CHAR) {
        handlerState=CONNECTION_PONG;
      }
      else {
        handlerState=NO_PACKET;
        state=false;
        return false;
      }
      state=true;
      return false;
    }
}

bool BLEHandler::packetTerminate(PacketType type) {
  //collect \r and \n characters and maintain state or pass errors back to initial state
  char a;
  if (ble.available() {
    a=ble.read()
    if (a=='\n') {
      handlerState=type;
      return true;
    }
    if (a=='\r') {
      handlerState=type;
    }
    else 
      handlerState=NO_PACKET;
  }
  return false;  
}


void BLEHandler::configRequestHandler() {
  //send config
  handlerState=NO_PACKET;      
}

void BLEHandler::connectionPongHandler() {
  connected=true;
  handlerState=NO_PACKET;
}
  
  
bool BLEHandler::dataPacketHandler() {
  Field currentField=data.getField(fieldIndex);
  while (ble.available()) {
    a=ble.read();
    
    if (a==DELIMITER_CHAR) {
      if(fieldIndex < data.fieldCount) {
        if(stringIndex<currentField.size) {
          currentField.string_value[stringIndex]='\0';
        }
        else {
          Serial.print("ERROR: Field too long");
          handlerState=ERROR_PACKET;
          break;
        }
        stringIndex=0;
        fieldIndex++;        
        currentField=data.getField(fieldIndex);
      }
      else {
        Serial.print("ERROR: Too many fields recieved");
        handlerState=ERROR_PACKET;
        break;
      }
    }
    
    if (a=='\r') continue;
    if (a=='\n') {
      if (fieldIndex==data.fieldCount)        
        return true;
      else {
        Serial.print("ERROR: Too few fields recieved");
        handlerState=ERROR_PACKET;
      }
      break;
    }
    
    switch(currentField.errorDetection) {
      case NUMERICAL_DETECT:
          if (isdigit(a))
            break;  
       case FLOAT_DETECT:
          if ((a == '-') or (a == '.'))
            break;
        default:
          Serial.printf("Incorrect character:   %c      in field:  %s", a, currentField.name);
          handlerState=ERROR_PACKET;
          break;
    }
    if (handlerState==ERROR_PACKET)     //triggered in switch statement
      break;
    if(stringIndex<currentField.size) {
      currentField.string_value[stringIndex]=a;
      stringIndex++;
    }
    else {
      Serial.printf("ERROR:  field size (%d) is larger than allowed %d",i,currentField.size);
      handlerState=ERROR_PACKET;
      break;
    }
  }
  return false;
}


int timeout=1000;            // Wait 800ms each time for BLE to response


//===============================================================================


 bool BLEAvailable() {
  return ble.available();
 }


//===============================================================================

bool BLEAutoBaud() {
  long bauds[] = {BAUD0,BAUD1,BAUD2,BAUD3,BAUD4}; // common baud rates, for HM-10 module with SoftwareSerial, try not to go over 57600
  int baudcount=sizeof(bauds)/sizeof(long);
  for(int i=0; i<baudcount; i++) {
    for(int x=0; x<4; x++) {    // test at least 4 times for each baud
      ble.begin(bauds[i]);
      if (BLEIsReady()) {
        Serial.print("BLE BAUD:  ");Serial.println(bauds[i]);
        return true;
      }
    }
  }
  Serial.println("BLE BAUD:  UNKNOWN");
  return false;
}

//===============================================================================

bool BLECmd(long timeout, char* command, char* temp, size_t len, bool verboseoutput) {
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

bool BLEIsReady() {
  char temp[4];     //catch first additional character
  BLECmd(500, "AT",temp,4,false);    // Send AT and store response to buffer 
  if (strcmp(temp,"OK")==0){    
    return true;
  } else {
    return false;
  }  
}

//===============================================================================

bool BLEInit() {
  char temp[10];
  BLECmd(timeout,"AT+RENEW",temp,10);
  delay(200);
  if (!BLEIsReady())
    BLEAutoBaud();     //incase instant baud change
  BLECmd(timeout,"AT+BAUD4",temp,10);
  delay(200);
  if (!BLEIsReady())
    BLEAutoBaud();     //incase instant baud change
  delay(200);
  BLECmd(5000,"AT+RESET",temp,10);
  delay(200);
    BLEAutoBaud();    
  if (!BLECmd(timeout,"AT+ROLE1",temp,10)) return false;
  if (!BLECmd(timeout,"AT+IMME1",temp,10)) return false;
  if (!BLECmd(timeout,"AT+POWE2",temp,10)) return false;
  if (!BLECmd(timeout,"AT+NOTI1",temp,10)) return false;
  return true;
}



//===============================================================================

void BLERequestConnection() {
  char temp[10];
  BLECmd(timeout,"AT+CON88C25512421A",temp,10);
  return; 
}



//===============================================================================

bool BLEIsConnected() {
  unsigned long starttime=millis(); 
  unsigned long timeout = 1000;
  PacketType type;
  DataPacket data;
  bool midPacket = false;
  ble.println("!;");
  
  while (millis()-starttime < timeout) {
    if (parseBLE(type, data, midPacket) and (type == CONNECTION_PACKET or DATA_PACKET))
      if (!midPacket)
         return true;
  }
  return false;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


#ifdef OLDCODE
bool BLEIsConnected(bool connWait ,int recursionCount) {
  char temp[10];
  memset(temp,0,10);
  bool bleRead=false;
  

  
  if (ble.available()) {
    int i=0;  
    while(ble.available()) {                      // loop and read the data
      bleRead=true;
      char a=ble.read();
      if (a==CONNECTED_CHAR)
        return true;
      if (a==CONFIG_REQUEST_C) {                              //request for time received
        /*char timeString[30];
        ble.print("\a");                          //provide time
        str_ll(CURRENT_TIME,timeString);
        ble.println(timeString); */
        config.send();
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
  if(!connWait and !BLECmd(timeout,"AT+CON88C25512421A",temp,10)) {  //88C25512421A    //check if we had already received CONNA and are waiting, request connection if not
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
  if (strcmp(temp,"OK+CONN")==0 or strcmp(temp,CONNECTED_CHAR_S)==0) {
    Serial.print("CONNECTED");
    return true;
  }
  if (strcmp(temp,CONFIG_REQUEST)==0) {
    /*char timeString[30];
    ble.print("\a");                          //provide time
    str_ll(CURRENT_TIME,timeString);
    ble.println(timeString); */
    Serial.print("REQUEST");
    config.send();
    return true;
  }    
  if (connWait) {
    delay(2000);
    if(recursionCount>5) return false;
    return BLEIsConnected(true,recursionCount+1);
  }
  //received something but unknown
  Serial.print("ERROR: Received unknown    -    ");Serial.println(temp);
  return false;
}
*/
#endif


//===============================================================================

bool parseBLE(PacketType &type, DataPacket& data, bool &midPacket) {
  static char* strings[DATA_FIELDS_COUNT];
  static char BLEBuffer[BLE_MAX_PACKET_SIZE];
  static int strIndex = 0;
  static int pos = 0;
  static bool errorParsing=false;
  static bool commandPacket=false;
  static char* ptr = &(BLEBuffer[0]);
  static unsigned long timeout= millis();
  bool exiting=false;
  bool retv=false;        //set failed
  static unsigned long lastRec = millis();
  if(midPacket and (millis() - lastRec > 1000)) {
    //discard last data
    ptr = &(BLEBuffer[0]);
    pos=0;
    strIndex=0;
    errorParsing=false;
    Serial.println("Packet TIMOUT");
  }

  while(ble.available()) {    // loop and read the data
    char a=ble.read();
    //Serial.print(a);
    midPacket=true;
    if (strIndex >= DATA_FIELDS_COUNT) {
      Serial.println("ERROR:  Too many fields received");
      errorParsing=true;
    }
    if (pos + 1 == BLE_MAX_PACKET_SIZE) {   //overflow error
      Serial.println("ERROR:  Packet too long, no newline");
      errorParsing=true;
    }    
    if (a=='\n') {            //end of data packet
      if (!errorParsing) {
        BLEBuffer[pos]='\0';
        strings[strIndex]=ptr;  
        if (type == DATA_PACKET and strIndex == DATA_FIELDS_COUNT) {
          data.setID(strings[ID_INDEX]);
          data.setTimeStamp(strings[TIME_INDEX]);
          data.setYaw(strings[YAW_INDEX]);
          data.setPitch(strings[PITCH_INDEX]);
          data.setRoll(strings[ROLL_INDEX]);
          retv=true;                        //exit with success
        }
        if (type == CONNECTION_PACKET and strIndex==CONNECTION_FIELDS_COUNT) {
          retv=true;
        }
        else {
          Serial.printf("ERROR:  Incorrect number of parameters,   %d received for packet type %s",strIndex,type == DATA_PACKET ? "DATA" : type == CONNECTION_PACKET ? "CONNECTION" : "OTHER");
        }
      }
      exiting=true;
      break;
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
      case PACKET_TYPE_INDEX:
        if (a==DATA_CHAR) {
          type=DATA_PACKET;
        }
        else if (a==RESPONSE_CHAR) {
          type=RESPONSE_PACKET;
        }
        else if (a==CONNECTION_CHAR) {
          type=CONNECTION_PACKET;
        }
        else { 
          type=ERROR_PACKET; 
          errorParsing=true;
        }
        continue;     //dont store anything in buffer until successful TYPE character followed by DELIMITER char
        
      case ID_INDEX:            //ID Packet (uint)
        // treat errors same as TIME
      case TIME_INDEX:          //Time Packet (uint)
        if (!isdigit(a)) {
          
          Serial.printf("ERROR:  Non numerical char '%c' in %s\n",a,strIndex == ID_INDEX ? "ID" : "Time");
          errorParsing=true; 
          continue;
        }
        break; 
        
      case YAW_INDEX:
      case PITCH_INDEX:
      case ROLL_INDEX:     //YPR (float)
        if (!isdigit(a) and (a != '-') and (a != '.')) {
          Serial.printf("ERROR:  Non numerical char '%c' in %s\n",a,strIndex == YAW_INDEX ? "Yaw" : strIndex == PITCH_INDEX ? "Pitch" : "Roll");
          errorParsing=true;
          continue;
        }
        break;
      default:    //more than 5 parameters, errror
        Serial.println("ERROR:  More than 5 parameters  ");
        errorParsing=true;
        continue;
    }
    BLEBuffer[pos]=a;   //store successful character and increment
    pos++;
  }
  lastRec=millis();
  if (exiting) {
      midPacket=false;
      //static var cleanup
      strIndex=0;
      pos=0;
      ptr=&(BLEBuffer[0]);
      errorParsing=false;
      commandPacket=false;
      return retv;
  }
  return false;   //finished receiving, not end of packet '\n', midPacket flag should be set
}


//===============================================================================

void requestResendPacket(unsigned long packetIDRequest) {
  static uint32_t lastRequested = millis();
  if (millis() - lastRequested > MIN_REQUEST_PERIOD) {
    ble.print(RESEND_CHAR);ble.print(PACKET_DELIMITER);ble.println(packetIDRequest);
    lastRequested=millis();
  }
  return;  
}




