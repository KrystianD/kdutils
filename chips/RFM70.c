/*
 * RFM70.c
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "RFM70.h"

void rfm70Init()
{
	rfm70DisableChip();
	rfm70DisableSPI();
	rfm70InitRegisters();
}

uint8_t rfm70ReadStatus()
{
	rfm70EnableSPI();
	uint8_t status = rfm70_SPI_RW(0xff);
	rfm70DisableSPI();
	return status;
}

uint8_t rfm70GetBank()
{
	rfm70EnableSPI();
	uint8_t status = rfm70_SPI_RW(0xff);
	rfm70DisableSPI();
	return status & 0x80 ? 1 : 0;
}
void rfm70SetBank(uint8_t bank)
{
	uint8_t curBank = rfm70GetBank();
	if (curBank != bank)
		rfm70SendCommand(RFM70_ACTIVATE, "\x53", 1);
		
	// while (curBank != bank)
	// {
	// RFM70_sendCommand_P (RFM70_ACTIVATE, PSTR("\x53"), 1);
	// curBank = RFM70_getBank ();
	// if (curBank == bank)
	// return;
	// _delay_ms (10);
	// }
}

inline void rfm70ReadCommand(uint8_t cmd, uint8_t* data, uint8_t len)
{
	rfm70EnableSPI();
	rfm70_SPI_RW(cmd);
	while (len--)
		*data++ = rfm70_SPI_RW(0xff);
	rfm70DisableSPI();
}
inline void rfm70SendCommand(uint8_t cmd, const uint8_t* data, uint8_t len)
{
	rfm70EnableSPI();
	rfm70_SPI_RW(cmd);
	while (len--)
		rfm70_SPI_RW(*data++);
	rfm70DisableSPI();
}

void rfm70WriteRegister(uint8_t addr, const uint8_t* data, uint8_t len)
{
	rfm70SendCommand(RFM70_WRITE_REG | addr, data, len);
}
void rfm70WriteRegisterValue(uint8_t addr, uint8_t value)
{
	rfm70EnableSPI();
	rfm70_SPI_RW(RFM70_WRITE_REG | addr);
	rfm70_SPI_RW(value);
	rfm70DisableSPI();
}
void rfm70ReadRegister(uint8_t addr, uint8_t* data, uint8_t len)
{
	rfm70ReadCommand(RFM70_READ_REG | addr, data, len);
}
uint8_t rfm70ReadRegisterValue(uint8_t addr)
{
	uint8_t val;
	rfm70EnableSPI();
	rfm70_SPI_RW(RFM70_READ_REG | addr);
	val = rfm70_SPI_RW(0xff);
	rfm70DisableSPI();
	return val;
}

void rfm70PowerDown()
{
	uint8_t val;
	// rfm70EnableChip ();
	val = rfm70ReadRegisterValue(RFM70_CONFIG);
	val &= ~RFM70_CONFIG_PWR_UP;
	rfm70WriteRegisterValue(RFM70_CONFIG, val);
	// rfm70DisableChip ();
}
void rfm70PowerUp()
{
	uint8_t val;
	// rfm70EnableChip ();
	val = rfm70ReadRegisterValue(RFM70_CONFIG);
	val |= RFM70_CONFIG_PWR_UP;
	rfm70WriteRegisterValue(RFM70_CONFIG, val);
	// rfm70DisableChip ();
}

void rfm70SwitchToRxMode()
{
	uint8_t val;
	
	rfm70SendCommand(RFM70_FLUSH_RX, 0, 0);
	
	val = rfm70ReadRegisterValue(RFM70_STATUS);
	rfm70WriteRegisterValue(RFM70_STATUS, val);
	
	// rfm70EnableChip ();
	
	val = rfm70ReadRegisterValue(RFM70_CONFIG);
	val |= RFM70_CONFIG_PRIM_RX;
	rfm70WriteRegisterValue(RFM70_CONFIG, val);
	
	// rfm70DisableChip ();
}
void rfm70SwitchToTxMode()
{
	uint8_t val;
	
	rfm70SendCommand(RFM70_FLUSH_TX, 0, 0);
	
	val = rfm70ReadRegisterValue(RFM70_STATUS);
	rfm70WriteRegisterValue(RFM70_STATUS, val);
	
	// rfm70EnableChip ();
	
	val = rfm70ReadRegisterValue(RFM70_CONFIG);
	val &= ~RFM70_CONFIG_PRIM_RX;
	rfm70WriteRegisterValue(RFM70_CONFIG, val);
	
	// rfm70DisableChip ();
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
	
	uint8_t status = rfm70ReadStatus();
	myprintf("status: %s\r\n", bin(status));
	myprintf("RBANK: %d  ", (status >> 7) & 0x01);
	myprintf("RX_DR: %d  ", (status >> 6) & 0x01);
	myprintf("TX_DR: %d  ", (status >> 5) & 0x01);
	myprintf("MAX_RT: %d  ", (status >> 4) & 0x01);
	myprintf("RX_P_NO: %3s  ", bin((status >> 1) & 0x07));
	myprintf("TX_FULL: %d  ", (status >> 0) & 0x01);
	myprintf("\r\n");
	
	data = rfm70ReadRegisterValue(RFM70_CONFIG);
	myprintf("config: %s\r\n", bin(data));
	myprintf("EN_CRC: %d  CRCO: %d  PWR_UP: %d  PRIM_RX: %d\r\n",
	         (data >> 3) & 0x01,
	         (data >> 2) & 0x01,
	         (data >> 1) & 0x01,
	         (data >> 0) & 0x01);
	         
	rfm70ReadRegister(RFM70_RX_ADDR_P0, data2, 5);
	myprintf("RX_ADDR_P0: ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			myprintf(":");
		myprintf("%02x", data2[i]);
	}
	myprintf("\r\n");
	rfm70ReadRegister(RFM70_RX_ADDR_P1, data2, 5);
	myprintf("RX_ADDR_P1: ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			myprintf(":");
		myprintf("%02x", data2[i]);
	}
	myprintf("\r\n");
	rfm70ReadRegister(RFM70_TX_ADDR, data2, 5);
	myprintf("TX_ADDR:    ");
	for (i = 0; i < 5; i++)
	{
		if (i)
			myprintf(":");
		myprintf("%02x", data2[i]);
	}
	myprintf("\r\n");
	
	data = rfm70ReadRegisterValue(RFM70_EN_AA);
	myprintf("EN_AA: %s  ", bin(data));
	data = rfm70ReadRegisterValue(RFM70_EN_RXADDR);
	myprintf("EN_RXADDR: %s\r\n", bin(data));
	
	data = rfm70ReadRegisterValue(RFM70_RF_CH);
	myprintf("RF_CH: %d\r\n", data);
	
	data = rfm70ReadRegisterValue(RFM70_RF_SETUP);
	myprintf("RF_DR: %d\r\n", (data >> 3) & 0x01);
	
	data = rfm70ReadRegisterValue(RFM70_CD);
	myprintf("CD: %d\r\n", data);
	
	data = rfm70ReadRegisterValue(RFM70_RX_PW_P0);
	myprintf("RX_PW_P0: %d\r\n", data);
	
	data = rfm70ReadRegisterValue(RFM70_FEATURE);
	myprintf("FEATURE: %s\r\n", bin(data));
	
	data = rfm70ReadRegisterValue(RFM70_OBSERVE_TX);
	myprintf("PLOS_CNT: %d  ARC_CNT: %d\r\n",
	         (data >> 4) & 0x0f,
	         (data >> 0) & 0x0f);
	// myprintf ("0x%02x\r\n", data[0]);
	//
	// rfm70SetBank (1);
	
	// data = rfm70ReadRegister (0x08, 4);
	// myprintf ("Chip ID: %02x%02x%02x%02x\r\n", data[0], data[1], data[2], data[3]);
	
	// rfm70SetBank (0);
}
#endif
