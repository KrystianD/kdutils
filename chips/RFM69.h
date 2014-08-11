#ifndef __RFM69_H__
#define __RFM69_H__

#include <stdint.h>

#define RFM69_SUCCESS 0
#define RFM69_ERROR   1

#define RFM69_FXOSC 32000000
#define RFM69_FSTEP (RFM69_FXOSC >> 19)

extern uint8_t rfm69SPIReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t rfm69SPISendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t rfm69SetFrequency(uint32_t freq);
uint8_t rfm69SetFrequencyByte(uint32_t freq);
uint8_t rfm69SetBitRate(uint32_t bitrate);
uint8_t rfm69SetSyncWord(uint8_t len, uint8_t* data);
uint8_t rfm69SetPreambleSize(uint8_t len);
uint8_t rfm69SetDeviation(uint32_t deviation);
uint8_t rfm69SetOutputPower(uint8_t power);
uint8_t rfm69SetRSSIThreshold(int16_t rssi);

uint8_t rfm69SwitchToSleep();
uint8_t rfm69SwitchToStandby();
uint8_t rfm69SwitchToTx();
uint8_t rfm69SwitchToRx();

int16_t rfm69GetRSSI();
void rfm69PrintStatus();
uint8_t rfm69IsReady(uint8_t* ready);

uint8_t rfm69ReadPayload(uint8_t* data, uint8_t len);
uint8_t rfm69ReadPayloadVarLen(uint8_t* data, uint8_t* len);
uint8_t rfm69WritePayload(const uint8_t* data, uint8_t len);

uint8_t rfm69WriteRegister(uint8_t addr, uint8_t value);
uint8_t rfm69ReadRegister(uint8_t addr, uint8_t* value);

#endif
