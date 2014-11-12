/*
 * RFM70.c
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "RFM70.h"
#include "settings.h"

#ifndef RFM70_DEBUG
#define RFM70_DEBUG(x,...)
#endif

uint8_t rfm70Init()
{
	rfm70DisableChip();
	// rfm70DisableSPI();
	rfm70InitRegisters();
	return RFM70_SUCCESS;
}

uint8_t rfm70ReadStatus(uint8_t* status)
{
	RFM70_ER(rfm70ReadRegisterValue(RFM70_STATUS, status));
	return RFM70_SUCCESS;
}

uint8_t rfm70GetBank(uint8_t* bank)
{
	uint8_t status;
	RFM70_ER(rfm70ReadStatus(&status));
	*bank = status & 0x80 ? 1 : 0;
	return RFM70_SUCCESS;
}
uint8_t rfm70SetBank(uint8_t bank)
{
	uint8_t curBank;
	RFM70_ER(rfm70GetBank(&curBank));
	if (curBank != bank)
	{
		RFM70_ER(rfm70SPISendCommand(RFM70_ACTIVATE, (const uint8_t*)"\x53", 1));
	}
	return RFM70_SUCCESS;
	
	// while (curBank != bank)
	// {
	// RFM70_sendCommand_P (RFM70_ACTIVATE, PSTR("\x53"), 1);
	// curBank = RFM70_getBank ();
	// if (curBank == bank)
	// return;
	// _delay_ms (10);
	// }
}

// inline void rfm70SPIReadCommand(uint8_t cmd, uint8_t* data, uint8_t len)
// {
// rfm70EnableSPI();
// rfm70_SPI_RW(cmd);
// while (len--)
// *data++ = rfm70_SPI_RW(0xff);
// rfm70DisableSPI();
// }
// inline void rfm70SPISendCommand(uint8_t cmd, const uint8_t* data, uint8_t len)
// {
// rfm70EnableSPI();
// rfm70_SPI_RW(cmd);
// while (len--)
// rfm70_SPI_RW(*data++);
// rfm70DisableSPI();
// }

uint8_t rfm70WriteRegister(uint8_t addr, const uint8_t* data, uint8_t len)
{
	RFM70_ER(rfm70SPISendCommand(RFM70_WRITE_REG | addr, data, len));
	return RFM70_SUCCESS;
}
uint8_t rfm70WriteRegisterValue(uint8_t addr, uint8_t value)
{
	RFM70_ER(rfm70SPISendCommand(RFM70_WRITE_REG | addr, (const uint8_t*)&value, 1));
	return RFM70_SUCCESS;
	// rfm70EnableSPI();
	// rfm70_SPI_RW(RFM70_WRITE_REG | addr);
	// rfm70_SPI_RW(value);
	// rfm70DisableSPI();
}
uint8_t rfm70ReadRegister(uint8_t addr, uint8_t* data, uint8_t len)
{
	RFM70_ER(rfm70SPIReadCommand(RFM70_READ_REG | addr, data, len));
	return RFM70_SUCCESS;
}
uint8_t rfm70ReadRegisterValue(uint8_t addr, uint8_t* val)
{
	RFM70_ER(rfm70SPIReadCommand(RFM70_READ_REG | addr, val, 1));
	return RFM70_SUCCESS;
}

uint8_t rfm70PowerDown()
{
	uint8_t val;
	rfm70EnableChip();
	RFM70_ER(rfm70ReadRegisterValue(RFM70_CONFIG, &val));
	val &= ~RFM70_CONFIG_PWR_UP;
	RFM70_ER(rfm70WriteRegisterValue(RFM70_CONFIG, val));
	rfm70DisableChip();
	return RFM70_SUCCESS;
}
uint8_t rfm70PowerUp()
{
	uint8_t val;
	rfm70EnableChip();
	RFM70_ER(rfm70ReadRegisterValue(RFM70_CONFIG, &val));
	val |= RFM70_CONFIG_PWR_UP;
	RFM70_ER(rfm70WriteRegisterValue(RFM70_CONFIG, val));
	rfm70DisableChip();
	return RFM70_SUCCESS;
}

uint8_t rfm70SwitchToRxMode()
{
	uint8_t val;
	
	RFM70_ER(rfm70SPISendCommand(RFM70_FLUSH_RX, 0, 0));
	
	RFM70_ER(rfm70ReadRegisterValue(RFM70_STATUS, &val));
	RFM70_ER(rfm70WriteRegisterValue(RFM70_STATUS, val));
	
	RFM70_ER(rfm70ReadRegisterValue(RFM70_CONFIG, &val));
	val |= RFM70_CONFIG_PRIM_RX;
	RFM70_ER(rfm70WriteRegisterValue(RFM70_CONFIG, val));
	return RFM70_SUCCESS;
}
uint8_t rfm70SwitchToTxMode()
{
	uint8_t val;
	
	RFM70_ER(rfm70SPISendCommand(RFM70_FLUSH_TX, 0, 0));
	
	RFM70_ER(rfm70ReadRegisterValue(RFM70_STATUS, &val));
	RFM70_ER(rfm70WriteRegisterValue(RFM70_STATUS, val));
	
	RFM70_ER(rfm70ReadRegisterValue(RFM70_CONFIG, &val));
	val &= ~RFM70_CONFIG_PRIM_RX;
	RFM70_ER(rfm70WriteRegisterValue(RFM70_CONFIG, val));
	return RFM70_SUCCESS;
}

//DEBUG
#ifdef RFM70_DEBUG
static char* bin(uint8_t v)
{
	int i;
	static char b[9];
	b[8] = 0;
	for (i = 0; i < 8; i++)
	{
		b[7 - i] = (v & 0x01) | '0';
		v >>= 1;
	}
	return b;
}
void rfm70PrintStatus()
{
	int i;
	uint8_t data;
	uint8_t data2[10];
	
	uint8_t status;
	rfm70ReadStatus(&status);
	RFM70_DEBUG("status: %s\r\n", bin(status));
	RFM70_DEBUG("RBANK: %d  ", (status >> 7) & 0x01);
	RFM70_DEBUG("RX_DR: %d  ", (status >> 6) & 0x01);
	RFM70_DEBUG("TX_DR: %d  ", (status >> 5) & 0x01);
	RFM70_DEBUG("MAX_RT: %d  ", (status >> 4) & 0x01);
	RFM70_DEBUG("RX_P_NO: %3s  ", bin((status >> 1) & 0x07));
	RFM70_DEBUG("TX_FULL: %d  ", (status >> 0) & 0x01);
	RFM70_DEBUG("\r\n");
	
	rfm70ReadRegisterValue(RFM70_CONFIG, &data);
	RFM70_DEBUG("config: %s\r\n", bin(data));
	RFM70_DEBUG("EN_CRC: %d  CRCO: %d  PWR_UP: %d  PRIM_RX: %d\r\n",
	            (data >> 3) & 0x01,
	            (data >> 2) & 0x01,
	            (data >> 1) & 0x01,
	            (data >> 0) & 0x01);
	            
	rfm70ReadRegister(RFM70_RX_ADDR_P0, data2, 5);
	RFM70_DEBUG("RX_ADDR_P0: ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			RFM70_DEBUG(":");
		RFM70_DEBUG("%02x", data2[i]);
	}
	RFM70_DEBUG("\r\n");
	rfm70ReadRegister(RFM70_RX_ADDR_P1, data2, 5);
	RFM70_DEBUG("RX_ADDR_P1: ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			RFM70_DEBUG(":");
		RFM70_DEBUG("%02x", data2[i]);
	}
	RFM70_DEBUG("\r\n");
	rfm70ReadRegister(RFM70_TX_ADDR, data2, 5);
	RFM70_DEBUG("TX_ADDR:    ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			RFM70_DEBUG(":");
		RFM70_DEBUG("%02x", data2[i]);
	}
	RFM70_DEBUG("\r\n");
	
	rfm70ReadRegisterValue(RFM70_EN_AA, &data);
	RFM70_DEBUG("EN_AA: %s  ", bin(data));
	rfm70ReadRegisterValue(RFM70_EN_RXADDR, &data);
	RFM70_DEBUG("EN_RXADDR: %s\r\n", bin(data));
	
	rfm70ReadRegisterValue(RFM70_RF_CH, &data);
	RFM70_DEBUG("RF_CH: %d\r\n", data);
	
	rfm70ReadRegisterValue(RFM70_RF_SETUP, &data);
	RFM70_DEBUG("RF_DR: %d\r\n", (data >> 3) & 0x01);
	
	rfm70ReadRegisterValue(RFM70_CD, &data);
	RFM70_DEBUG("CD: %d\r\n", data);
	
	rfm70ReadRegisterValue(RFM70_RX_PW_P0, &data);
	RFM70_DEBUG("RX_PW_P0: %d\r\n", data);
	
	rfm70ReadRegisterValue(RFM70_FEATURE, &data);
	RFM70_DEBUG("FEATURE: %s\r\n", bin(data));
	
	rfm70ReadRegisterValue(RFM70_OBSERVE_TX, &data);
	RFM70_DEBUG("PLOS_CNT: %d  ARC_CNT: %d\r\n",
	            (data >> 4) & 0x0f,
	            (data >> 0) & 0x0f);
	// RFM70_DEBUG ("0x%02x\r\n", data[0]);
	//
	// rfm70SetBank (1);
	
	// data = rfm70ReadRegister (0x08, 4);
	// RFM70_DEBUG ("Chip ID: %02x%02x%02x%02x\r\n", data[0], data[1], data[2], data[3]);
	
	// rfm70SetBank (0);
}
#endif
