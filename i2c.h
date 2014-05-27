#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

void i2cOpen(const char* path);
void i2cClose();
void i2cSetAddress(int address);

int i2cWrite(const uint8_t* data, int len);
int i2cRead(uint8_t* data, int len);

void i2cWriteReg(uint8_t reg, uint8_t val);
void i2cWriteReg(uint8_t reg, const uint8_t* data, int len);
uint8_t i2cReadReg(uint8_t reg);
void i2cReadReg(uint8_t reg, uint8_t* data, int len);

#endif
