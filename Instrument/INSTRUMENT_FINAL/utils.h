/*
 * Helper functions
 * 
 * 
 * 
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

char* faststrcat( char* dest, char* src );

long readVcc();

unsigned long long atoull(const char* ptr);

unsigned long atoul(const char* ptr);

void str_ll(uint64_t value, char* string);

#endif



