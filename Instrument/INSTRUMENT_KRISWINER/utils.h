#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>


struct s_mympu {
  float ID, TI, YW, PT, RL, QW, QX, QY, QZ, AX, AY, AZ, MX, MY, MZ, GX, GY, GZ;
};
extern s_mympu mympu;


char* faststrcat( char* dest, char* src );

long readVcc();

unsigned long long atoull(const char* ptr);

unsigned long atoul(const char* ptr);

void str_ll(uint64_t value, char* string);

#endif



