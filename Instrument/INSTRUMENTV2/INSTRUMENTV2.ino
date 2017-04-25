#include "freeram.h"
#include "mpu.h"
#include "I2Cdev.h"
#include<stdlib.h>

#include "configuration.h"
#include "BLEInstrumentHandler.h"
#include "messageStore.h"

#define SAMPLE_FREQUENCY (20)

Configuration config;
BLEInstrumentHandler bth(&config);


int ret;

unsigned int c = 0; //cumulative number of successful MPU/DMP reads
unsigned int np = 0; //cumulative number of MPU/DMP reads that brought no packet back
unsigned int err_c = 0; //cumulative number of MPU/DMP reads that brought corrupted packet
unsigned int err_o = 0; //cumulative number of MPU/DMP reads that had overflow bit set

String stringSend;
DataMessage d;
  



void setup() {
    Fastwire::setup(400,0);

  Serial.begin(115200);
  Serial1.begin(9600);
  delay(500);
  while(!Serial);
  
  ret = mympu_open(SAMPLE_FREQUENCY);
  
  Serial.println("Accelerometer Initialisation");
  Serial.print("MPU init: "); Serial.println(ret);
  Serial.print("Free mem: "); Serial.println(freeRam()); Serial.println();
  
  Serial.println("Waiting for BLE");
  bth.initialise();
  while (!config.isSet) {
    bth.monitor();
  }
  Serial.println("Config recieved");
}

void loop() {
    bth.monitor();
    ret = mympu_update();
    
    switch (ret) {
      case 0: c++; 	  
        //create send string;
        Serial1.print(DATA_CHAR);
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(config.currentID);
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(config.currentTime_str());
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(mympu.ypr[0]);
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(mympu.ypr[1]);
        Serial1.print(DELIMITER_CHAR);
        Serial1.println(mympu.ypr[2]);

        /*
        stringSend = String(DATA_CHAR) + ";" + String(config.currentID) + ";" + String(config.currentTime_str()) + ";" + String(mympu.ypr[0],3) + ";" + String(mympu.ypr[1],3) + ";" + String(mympu.ypr[2],3);
        Serial.println(stringSend);
        //Store incase of error in transmission 
        d.id=config.currentID;
        strcpy(d.content,stringSend.c_str());
        //bth.store.push(d);
        */
        //Serial1.println(stringSend);
        config.incrementID();
        
      break;
      case 1: np++; 
        //Serial.println("ERR: 1 - no packet available");
        break;
      case 2: err_o++; 
        //Serial.println("ERR: 2 - if BIT_FIFO_OVERFLOWN is set");
        break;
      case 3: err_c++; 
        //Serial.println("ERR: 3 - if frame corrupted");
        break; 
      default: 
      	Serial.print("READ ERROR!  ");
      	Serial.println(ret);
      	break;
  }
    
}

/*

send(char * p, messageType 
  p=faststrcat(p,DATA_CHAR_STR);

";"
config.currentID
";"
config.currentTime_str()
";"
mympu.ypr[0]
";"
mympu.ypr[1]
";"
mympu.ypr[2]


  p=faststrcat(p,data.getField(i)->name);
  p=faststrcat(p,"\":\"");
  p=faststrcat(p,data.getField(i)->string_value);
  p=faststrcat(p,"\"}");
  return p;
}








*/
