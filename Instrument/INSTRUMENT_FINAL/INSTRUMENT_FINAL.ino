#include "freeram.h"
#include "mpu.h"
#include "I2Cdev.h"
#include<stdlib.h>
#include <arduino.h>

#include "configuration.h"
#include "BLEInstrumentHandler.h"
#include "messageStore.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

//   calibrateMPU9250(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers



  
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


unsigned long lastint=millis();
bool intFLAG=false;

void intfunction() {
  intFLAG=true;
}


void wakeUpNow() {  
  // execute code here after wake-up before returning to the loop() function  
  // timers and code using timers (serial.print and more...) will not work here.  
  // we don't really need to execute any special functions here, since we  
  // just want the thing to wake up
  intFLAG=true;
}  
 
  
void sleepNow() {  
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here  

    sleep_enable();          // enables the sleep bit in the mcucr register  
        sei();   // Must enable or can't wake!
      sleep_cpu();
    //sleep_mode();            // here the device is actually put to sleep!!
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP  
    sleep_disable();         // first thing after waking from sleep: disable sleep...  
    
}

void sleep2s() {  
  set_sleep_mode(SLEEP_MODE_STANDBY);             // select the watchdog timer mode
  MCUSR &= ~(1 << WDRF);                           // reset status flag
  WDTCSR |= (1 << WDCE) | (1 << WDE);              // enable configuration changes
  WDTCSR = (1<< WDP0) | (1 << WDP1) | (1 << WDP2); // set the prescalar = 7
  WDTCSR |= (1 << WDIE);                           // enable interrupt mode
  sleep_enable();                                  // enable the sleep mode ready for use
  sleep_mode();                                    // trigger the sleep
  /* ...time passes ... */
  sleep_disable();                                 // prevent further sleeps
}  
  




void setup() {

  
ADCSRA = 0;
DDRD &= B00000011;       // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
DDRB = B00000000;        // set pins 8 to 13 as inputs
PORTD |= B11111100;      // enable pullups on pins 2 to 7
PORTB |= B11111111;      // enable pullups on pins 8 to 13

  
  
  pinMode(7,INPUT_PULLUP);
    Fastwire::setup(400,0);


  Serial.begin(115200);
  Serial1.begin(9600);
  delay(2000);
     
  ret = mympu_open(SAMPLE_FREQUENCY);
  
  Serial.println("Accelerometer Initialisation");
  Serial.print("MPU init: "); Serial.println(ret);
  Serial.print("Free mem: "); Serial.println(freeRam()); Serial.println();
  
  Serial.println("Waiting for BLE");
  bth.begin();
  while (!config.isSet) {
    bth.monitor();
  }
  Serial.println("Config recieved");
  attachInterrupt(4, wakeUpNow, RISING); // use interrupt 0 (pin 2) and run function wakeUpNow when pin 2 gets LOW 
  //attachInterrupt(4, intfunction, RISING);


}



unsigned long timeca;
void loop() {

    bth.monitor();

    if (intFLAG) {
      intFLAG=false;
      timeca=micros();
      ret = mympu_read();
    switch (ret) {
      case 0: c++; 	  
        //create send string;
        
        Serial1.print(DATA_CHAR);
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(config.currentID);
        Serial1.print(DELIMITER_CHAR);
        Serial1.print(config.currentTime_str());
        Serial1.print(DELIMITER_CHAR);
        for(int i=0; i<config.fieldCount-3;i++) {
          Serial1.print(*config.dataPointers[i],2);
          Serial1.print(DELIMITER_CHAR);
        }
        Serial1.println(*config.dataPointers[config.fieldCount-3],2);
        Serial1.flush();
        config.incrementID();
//        Serial.println(micros()-timeca);
          //sleepNow();
        /*
        while(!intFLAG) {
          Serial.println("sleep");
          Serial.flush();
          
           if (Serial1.available())
            break;
          sleepNow();
        }
        */
        /*
        stringSend = String(DATA_CHAR) + ";" + String(config.currentID) + ";" + String(config.currentTime_str()) + ";" + String(mympu.ypr[0],3) + ";" + String(mympu.ypr[1],3) + ";" + String(mympu.ypr[2],3);
        Serial.println(stringSend);
        //Store incase of error in transmission 
        d.id=config.currentID;
        strcpy(d.content,stringSend.c_str());
        //bth.store.push(d);
        */
        //Serial1.println(stringSend);
        
        
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
