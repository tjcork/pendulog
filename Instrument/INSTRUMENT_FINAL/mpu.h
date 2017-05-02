/*
 * 
 * adapted from AVRCopter project
 * https://github.com/rpicopter/ArduinoMotionSensorExample
 * 
 * interfaces the MPU9250 module
 * retrieves data
 * converts data
 * 
 */


#ifndef MPU_H
#define MPU_H

struct s_mympu {
  float ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, MX, MY, MZ, GX, GY, GZ;  
};

extern struct s_mympu mympu;

int mympu_open(unsigned int rate);
int mympu_update();
int mympu_read();

#endif


