#include "freeram.h"

#include "mpu.h"
#include "I2Cdev.h"

int ret;
void setup() {
    Fastwire::setup(400,0);
    Serial.begin(38400);
    ret = mympu_open(16);
    Serial.print("MPU init: "); Serial.println(ret);
    Serial.print("Free mem: "); Serial.println(freeRam());
	
}

unsigned int c = 0; //cumulative number of successful MPU/DMP reads
unsigned int np = 0; //cumulative number of MPU/DMP reads that brought no packet back
unsigned int err_c = 0; //cumulative number of MPU/DMP reads that brought corrupted packet
unsigned int err_o = 0; //cumulative number of MPU/DMP reads that had overflow bit set

void loop() {
    ret = mympu_update();

    switch (ret) {
	case 0: c++; break;
	case 1: np++; 
	  //Serial.println("ERR: 1 - no packet available");
    return;
	case 2: err_o++; 
    //Serial.println("ERR: 2 - if BIT_FIFO_OVERFLOWN is set");
    return;
	case 3: err_c++; 
	  //Serial.println("ERR: 3 - if frame corrupted");
    return; 
	default: 
		Serial.print("READ ERROR!  ");
		Serial.println(ret);
		return;
    }

    Serial.print(c);Serial.print("\t"); 
    Serial.print(mympu.ypr[0]);Serial.print("\t"); 
    Serial.print(mympu.ypr[1]);Serial.print("\t"); 
    Serial.print(mympu.ypr[2]);Serial.print("\t"); 
    Serial.print('\n');
  
//    if (!(c%25)) {
//	    Serial.print(np); Serial.print("  "); Serial.print(err_c); Serial.print(" "); Serial.print(err_o);
//	    Serial.print(" Y: "); Serial.print(mympu.ypr[0]);
//	    Serial.print(" P: "); Serial.print(mympu.ypr[1]);
//	    Serial.print(" R: "); Serial.print(mympu.ypr[2]);
//	    Serial.print("\tgy: "); Serial.print(mympu.gyro[0]);
//	    Serial.print(" gp: "); Serial.print(mympu.gyro[1]);
//	    Serial.print(" gr: "); Serial.println(mympu.gyro[2]);
//    }


}

