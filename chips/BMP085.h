#ifndef __BMP085_H__
#define __BMP085_H__

#include <stdint.h>

#define BMP085_SUCCESS 0
#define BMP085_ERROR   1

#define BMP085_ADDR    0x77

extern int16_t bmp085Temperature; // temp result in 0.1 degree
extern int32_t bmp085Pressure;

extern uint32_t getTicks();
extern uint8_t bmp085I2CReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t bmp085I2CSendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t bmp085Init();
uint8_t bmp085StartTempConv();
uint8_t bmp085StartPressureConv(int oss);
uint8_t bmp085IsResultReady();
uint8_t bmp085Process();

float bmp085GetAltitude();

#endif
