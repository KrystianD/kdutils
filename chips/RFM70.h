/*
 * RFM70.h
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __RFM70_H__
#define __RFM70_H__

#include <stdint.h>

#include "RFM70_defs.h"

extern uint8_t rfm70_rfm70_SPI_RW(uint8_t val);
extern void rfm70EnableChip();
extern void rfm70DisableChip();
extern void rfm70EnableSPI();
extern void rfm70DisableSPI();

void rfm70Init();
void rfm70InitRegisters();
uint8_t rfm70ReadStatus();
uint8_t rfm70GetBank();
void rfm70SetBank(uint8_t bank);
void rfm70ReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
void rfm70SendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

void rfm70WriteRegister(uint8_t addr, const uint8_t* data, uint8_t len);
void rfm70WriteRegisterValue(uint8_t addr, uint8_t value);
void rfm70ReadRegister(uint8_t addr, uint8_t* data, uint8_t len);
uint8_t rfm70ReadRegisterValue(uint8_t addr);
void rfm70PowerDown();
void rfm70PowerUp();
void rfm70SwitchToRxMode();
void rfm70SwitchToTxMode();

static inline void rfm70SetTxAddress(const char* addr, uint8_t len)
{
	rfm70WriteRegister(RFM70_TX_ADDR, (const uint8_t*)addr, len);
}
static inline void rfm70SetRxAddress(uint8_t num, const char* addr, uint8_t len)
{
	rfm70WriteRegister(RFM70_RX_ADDR_P0 + num, (const uint8_t*)addr, len);
}
static inline void rfm70SetPipeWidth(uint8_t num, uint8_t width)
{
	rfm70WriteRegisterValue(RFM70_RX_PW_P0 + num, width);
}
static inline void rfm70SetChannel(uint8_t ch)
{
	rfm70WriteRegisterValue(RFM70_RF_CH, ch);
}
static inline void rfm70WriteTxPayload(const uint8_t* data, uint8_t len)
{
	rfm70SendCommand(RFM70_WRITE_TX_PL, data, len);
}
static inline void rfm70ReadRxPayload(uint8_t* data, uint8_t len)
{
	rfm70ReadCommand(RFM70_READ_RX_PL, data, len);
}
static void rfm70SetFeatures(uint8_t v)
{
	uint8_t val;
	val = rfm70ReadRegisterValue(RFM70_FEATURE);
	if (val != v)
	{
		rfm70SendCommand(RFM70_ACTIVATE, "\x73", 1);
		rfm70WriteRegisterValue(RFM70_FEATURE, 0x07);
	}
}
static inline void rfm70EnableFeatures()
{
	rfm70SetFeatures(0x07);
}
static inline void rfm70DisableFeatures()
{
	rfm70SetFeatures(0x00);
}

#endif
