#ifndef __RFM69_H__
#define __RFM69_H__

#include <stdint.h>

#define RFM69_FXOSC 32000000
#define RFM69_FSTEP (RFM69_FXOSC >> 19)

extern void rfm69SPIReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern void rfm69SPISendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t rfm69SetFrequency(uint32_t freq);
uint8_t rfm69SetFrequencyByte(uint32_t freq);
uint8_t rfm69SetBitRate(uint32_t bitrate);
uint8_t rfm69SetSyncWord(uint8_t len, uint8_t* data);
uint8_t rfm69SetPreambleSize(uint8_t len);
uint8_t rfm69SetDeviation(uint32_t deviation);
uint8_t rfm69SetOutputPower(uint8_t power);
uint8_t rfm69SetRSSIThreshold(int16_t rssi);

void rfm69SwitchToSleep();
void rfm69SwitchToStandby();
void rfm69SwitchToTx();
void rfm69SwitchToRx();

int16_t rfm69GetRSSI();
void rfm69PrintStatus();
uint8_t rfm69IsReady();

uint8_t rfm69ReadPayload(uint8_t* data, uint8_t len);

void rfm69WriteRegister(uint8_t addr, uint8_t value);
uint8_t rfm69ReadRegister(uint8_t addr);

#endif
