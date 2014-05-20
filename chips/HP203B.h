#ifndef __HP203B_H__
#define __HP203B_H__

#include <stdint.h>

#define HP203B_SUCCESS 0
#define HP203B_ERROR   1

#define HP203B_ADDR      0x76

#define HP203B_SOFT_RST  0x06
#define HP203B_ADC_CVT   0x40
#define HP203B_READ_PT   0x10
#define HP203B_READ_AT   0x11
#define HP203B_READ_P    0x30
#define HP203B_READ_A    0x31
#define HP203B_READ_T    0x32
#define HP203B_ANA_CAL   0x28
#define HP203B_READ_REG  0x80
#define HP203B_WRITE_REG 0xc0

#define HP203B_OSR_4096  (0b000 << 2)
#define HP203B_OSR_2048  (0b001 << 2)
#define HP203B_OSR_1024  (0b010 << 2)
#define HP203B_OSR_512   (0b011 << 2)
#define HP203B_OSR_256   (0b100 << 2)
#define HP203B_OSR_128   (0b101 << 2)

#define HP203B_CH_PRESSTEMP 0b00
#define HP203B_CH_TEMP      0b10

extern uint32_t getTicks();
extern uint8_t hp203bI2CReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t hp203bI2CSendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t hp203bInit();
uint8_t hp203BStartConversion(uint8_t OSR);
uint8_t hp203bReadPressure(uint32_t* pressure);
uint8_t hp203bReadAltitude(uint32_t* altitude);
uint8_t hp203bIsResultReady();
uint8_t hp203bProcess();

#endif
