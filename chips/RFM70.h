/*
 * RFM70.h
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __RFM70_H__
#define __RFM70_H__

#include <stdint.h>

#define RFM70_SUCCESS 0
#define RFM70_ERROR   1

#define RFM70_ER(x) if (x) return RFM70_ERROR;

#include <RFM70_defs.h>

extern uint8_t rfm70EnableChip();
extern uint8_t rfm70DisableChip();
extern uint8_t rfm70SPIReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t rfm70SPISendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t rfm70Init();
uint8_t rfm70InitRegisters();
uint8_t rfm70ReadStatus(uint8_t* status);
uint8_t rfm70ClearStatus();
uint8_t rfm70GetBank(uint8_t* bank);
uint8_t rfm70SetBank(uint8_t bank);

uint8_t rfm70WriteRegister(uint8_t addr, const uint8_t* data, uint8_t len);
uint8_t rfm70WriteRegisterValue(uint8_t addr, uint8_t value);
uint8_t rfm70ReadRegister(uint8_t addr, uint8_t* data, uint8_t len);
uint8_t rfm70ReadRegisterValue(uint8_t addr, uint8_t* val);
uint8_t rfm70PowerDown();
uint8_t rfm70PowerUp();
uint8_t rfm70SwitchToRxMode();
uint8_t rfm70SwitchToTxMode();

uint8_t rfm70SetBits(uint8_t addr, uint8_t mask);
uint8_t rfm70ClearBits(uint8_t addr, uint8_t mask);

static inline uint8_t rfm70SetTxAddress(const char* addr, uint8_t len)
{
	RFM70_ER(rfm70WriteRegister(RFM70_TX_ADDR, (const uint8_t*)addr, len));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70SetRxAddress(uint8_t num, const char* addr, uint8_t len)
{
	RFM70_ER(rfm70WriteRegister(RFM70_RX_ADDR_P0 + num, (const uint8_t*)addr, len));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70SetPipeWidth(uint8_t num, uint8_t width)
{
	RFM70_ER(rfm70WriteRegisterValue(RFM70_RX_PW_P0 + num, width));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70SetChannel(uint8_t ch)
{
	RFM70_ER(rfm70WriteRegisterValue(RFM70_RF_CH, ch));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70WriteTxPayload(const uint8_t* data, uint8_t len)
{
	RFM70_ER(rfm70SPISendCommand(RFM70_WRITE_TX_PL, data, len));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70WriteTxPayloadNOACK(const uint8_t* data, uint8_t len)
{
	RFM70_ER(rfm70SPISendCommand(RFM70_WRITE_TX_PL_NOACK, data, len));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70ReadRxPayload(uint8_t* data, uint8_t len)
{
	RFM70_ER(rfm70SPIReadCommand(RFM70_READ_RX_PL, data, len));
	return RFM70_SUCCESS;
}
static uint8_t rfm70SetFeatures(uint8_t v)
{
	uint8_t val;
	RFM70_ER(rfm70ReadRegisterValue(RFM70_FEATURE, &val));
	if (val != v)
	{
		RFM70_ER(rfm70SPISendCommand(RFM70_ACTIVATE, (const uint8_t*)"\x73", 1));
		RFM70_ER(rfm70WriteRegisterValue(RFM70_FEATURE, 0x07));
	}
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70EnableFeatures()
{
	RFM70_ER(rfm70SetFeatures(0x07));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70DisableFeatures()
{
	RFM70_ER(rfm70SetFeatures(0x00));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70EnableCRC()
{
	RFM70_ER(rfm70SetBits(RFM70_CONFIG, RFM70_CONFIG_EN_CRC));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70DisableCRC()
{
	RFM70_ER(rfm70ClearBits(RFM70_CONFIG, RFM70_CONFIG_EN_CRC));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70Set1Mbps()
{
	RFM70_ER(rfm70ClearBits(RFM70_RF_SETUP, RFM70_RF_SETUP_RF_DR));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70Set2Mbps()
{
	RFM70_ER(rfm70SetBits(RFM70_RF_SETUP, RFM70_RF_SETUP_RF_DR));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70SetCRC1bytes()
{
	RFM70_ER(rfm70ClearBits(RFM70_CONFIG, RFM70_CONFIG_CRCO));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70SetCRC2bytes()
{
	RFM70_ER(rfm70SetBits(RFM70_CONFIG, RFM70_CONFIG_CRCO));
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70DataSent(uint8_t* status)
{
	uint8_t val;
	RFM70_ER(rfm70ReadStatus(&val));
	*status = val & RFM70_STATUS_TX_DS ? 1 : 0;
	return RFM70_SUCCESS;
}
static inline uint8_t rfm70DataSentOrMaxRetr(uint8_t* status)
{
	uint8_t val;
	RFM70_ER(rfm70ReadStatus(&val));
	*status = (val & (RFM70_STATUS_TX_DS | RFM70_STATUS_MAX_RT)) ? 1 : 0;
	return RFM70_SUCCESS;
}

#ifdef RFM70_DEBUG
uint8_t rfm70PrintStatus();
#endif

#endif
