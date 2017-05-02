/*
 * Code for instrument
 * Sparkfun pro micro 3V 8Mhz
 *
 * 
 * 
 * 
 * Author: Tom Corkett
 * 
 */
 
 
  
//============================================================================

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

#define SAMPLE_FREQUENCY (20)


//============================================================================
//variables

Configuration config;
BLEInstrumentHandler bth(&config);

int ret;
unsigned int c = 0; //successful MPU reads
unsigned int np = 0; //no packet read count
unsigned int err_c = 0; //corrupted read count
unsigned int err_o = 0; //overflow count

String stringSend;
DataMessage d;

unsigned long lastint=millis();
bool intFLAG=false;


//============================================================================
// Sleep functions

void intfunction() {
  intFLAG=true;
}

void wakeUpNow() {  
  //wake hardware interrupt - data ready
  intFLAG=true;
}  
   
void sleepNow() {  
    set_sleep_mode(SLEEP_MODE_STANDBY);   // sleep mode is set here  
    sleep_enable();          // enables the sleep bit in the mcucr register  
    sei();   
    sleep_cpu();
    // sleeping....  
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
  //sleeping...
  sleep_disable();                                 // prevent further sleeps
}  
  
//============================================================================

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
     
  mympu_open(SAMPLE_FREQUENCY);
  
  Serial.println("Accelerometer Initialisation");
  Serial.print("MPU init: "); Serial.println(ret);
  Serial.print("Free mem: "); Serial.println(freeRam()); Serial.println();
  
  Serial.println("Waiting for BLE");
  bth.begin();
  while (!config.isSet) {
    bth.monitor();
  }
  Serial.println("Config recieved");
  attachInterrupt(4, wakeUpNow, RISING); // run function wakeUpNow when data ready
  //attachInterrupt(4, intfunction, RISING);
}

 
//============================================================================


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
        sleepNow();
        
        /*
         *  FOR REPEATING SLEEPS - to fill output fifo
        while(!intFLAG) {
          Serial.println("sleep");
          Serial.flush();
          
           if (Serial1.available())
            break;
          sleepNow();
        }
        */
        //Store incase of error in transmission 
        d.id=config.currentID;
        strcpy(d.content,stringSend.c_str());
        bth.store.push(d);      
        
      break;
      case 1: np++; 
        //no packet available
        break;
      case 2: err_o++; 
        //BIT_FIFO_OVERFLOWN is set
        break;
      case 3: err_c++; 
        //frame corrupted
        break; 
      default: 
      	Serial.print("READ ERROR!  ");
      	Serial.println(ret);
      	break;
  }
    }
    
}

