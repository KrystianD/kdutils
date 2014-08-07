/*
 * nmea.c
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __NMEA_H__
#define __NMEA_H__

#include <stdint.h>

void nmeaReset();
void nmeaProcess(char c);

// -1 means no satellite
extern void nmeaGPGSA(char mode, uint8_t type, int16_t sats[12]);

extern void nmeaGPGGA(
  int32_t time, float latVal, char latSign, float lonVal, char lonSign,
  uint8_t fix, uint8_t satellites, float alt);

extern void nmeaGPRMC(
  int16_t time, char status, float latVal, char latSign, float lonVal, char lonSign,
  float speed);

extern void nmeaPMTK(uint8_t res, uint16_t cmd, uint8_t flag);

#endif
