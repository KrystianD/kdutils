/*
 * MAX7456.c
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "MAX7456.h"

#include <public.h>

#define MAX7456_TIME_CLR 20
#define MAX7456_TIME_RST 110
#define MAX7456_TIME_PWR 50000

#define MAX7456_HOS_ZERO 0x20  // Horizontal Offset Zero point
#define MAX7456_HOS_WID  6     // Horizontal Offset bit Width
#define MAX7456_VOS_ZERO 0x10  // Vertical Offset Zero point
#define MAX7456_VOS_WID  5     // Vertical Offset bit Width

#define MAX7456_VM0   0x00  // Video Mode 0 write
#define MAX7456_VM1   0x01  // Video Mode 1 write
#define MAX7456_HOS   0x02  // Horizontal Offset write
#define MAX7456_VOS   0x03  // Vertical Offset write
#define MAX7456_DMM   0x04  // Display Memory Mode write
#define MAX7456_DMAH  0x05  // Display Memory Address High write
#define MAX7456_DMAL  0x06  // Display Memory Address Low write
#define MAX7456_DMDI  0x07  // Display Memory Data In write
#define MAX7456_CMM   0x08  // Character Memory Mode write
#define MAX7456_CMAH  0x09  // Character Memory Address High write
#define MAX7456_CMAL  0x0A  // Character Memory Address Low write
#define MAX7456_CMDI  0x0B  // Character Memory Data In write
#define MAX7456_OSDM  0x0C  // OSD Insertion Mux write
#define MAX7456_RB0   0x10  // Row 0 Brightness write
#define MAX7456_RB1   0x11  // Row 1 Brightness write
#define MAX7456_RB2   0x12  // Row 2 Brightness write
#define MAX7456_RB3   0x13  // Row 3 Brightness write
#define MAX7456_RB4   0x14  // Row 4 Brightness write
#define MAX7456_RB5   0x15  // Row 5 Brightness write
#define MAX7456_RB6   0x16  // Row 6 Brightness write
#define MAX7456_RB7   0x17  // Row 7 Brightness write
#define MAX7456_RB8   0x18  // Row 8 Brightness write
#define MAX7456_RB9   0x19  // Row 9 Brightness write
#define MAX7456_RB10  0x1A  // Row 10 Brightness write
#define MAX7456_RB11  0x1B  // Row 11 Brightness write
#define MAX7456_RB12  0x1C  // Row 12 Brightness write
#define MAX7456_RB13  0x1D  // Row 13 Brightness write
#define MAX7456_RB14  0x1E  // Row 14 Brightness write
#define MAX7456_RB15  0x1F  // Row 15 Brightness write
#define MAX7456_OSDBL 0x6C  // OSD Black Level write
#define MAX7456_STAT  0xA0  // Status read
#define MAX7456_DMDO  0xB0  // Display Memory Data Out read
#define MAX7456_CMDO  0xC0  // Character Memory Data Out read

#define MAX7456_VM0_PAL      (1 << 6)
#define MAX7456_VM0_FIXSYNC  (1 << 5)
#define MAX7456_VM0_INTSYNC  (1 << 4)
#define MAX7456_VM0_OSDON    (1 << 3)
#define MAX7456_VM0_SYNCV    (1 << 2)
#define MAX7456_VM0_SRESET   (1 << 1)
#define MAX7456_VM0_BUFOFF   (1 << 0)

#define MAX7456_VM1_GRAYBG   (1 << 7)
#define MAX7456_VM1_GLVL_BIT 4
#define MAX7456_VM1_GLVL_LEN 3
#define MAX7456_VM1_BT_BIT   2
#define MAX7456_VM1_BT_LEN   2
#define MAX7456_VM1_BDC_BIT  0
#define MAX7456_VM1_BDC_LEN  2

#define MAX7456_DMM_OM       (1 << 6)
#define MAX7456_DMM_LBC      (1 << 5)
#define MAX7456_DMM_BLK      (1 << 4)
#define MAX7456_DMM_INV      (1 << 3)
#define MAX7456_DMM_CDM      (1 << 2)
#define MAX7456_DMM_VSC      (1 << 1)
#define MAX7456_DMM_AI       (1 << 0)

#define MAX7456_RBn_CBL_BIT  2
#define MAX7456_RBn_CBL_LEN  2
#define MAX7456_RBn_CWL_BIT  0
#define MAX7456_RBn_CWL_LEN  2

#define MAX7456_OSDBL_MAN    4

#define MAX7456_STAT_RBUSY   (1 << 6)
#define MAX7456_STAT_CMBUSY  (1 << 5)
#define MAX7456_STAT_NVSYNC  (1 << 4)
#define MAX7456_STAT_NHSYNC  (1 << 3)
#define MAX7456_STAT_LOS     (1 << 2)
#define MAX7456_STAT_NTSC    (1 << 1)
#define MAX7456_STAT_PAL     (1 << 0)

void max7456WriteRegister(uint8_t reg, uint8_t val)
{
	max7456SPIEnableChip();
	max7456SPIRW(reg & ~0x80);
	max7456SPIRW(val);
	max7456SPIDisableChip();
}
uint8_t max7456ReadRegister(uint8_t reg)
{
	max7456SPIEnableChip();
	max7456SPIRW(reg | 0x80);
	uint8_t b = max7456SPIRW(0xff);
	max7456SPIDisableChip();
	return b;
}
void max7456_setBitmask(uint8_t reg, uint8_t bitmask)
{
	uint8_t val = max7456ReadRegister(reg);
	val |= bitmask;
	max7456WriteRegister(reg, val);
}
void max7456_setBits(uint8_t reg, uint8_t bit, uint8_t len, uint8_t setVal)
{
	uint8_t val = max7456ReadRegister(reg);
	
	uint8_t mask = ((1 << len) - 1) << bit;
	val &= ~mask;
	val |= setVal << bit;
	
	max7456WriteRegister(reg, val);
}
void max7456_clearBitmask(uint8_t reg, uint8_t bitmask)
{
	uint8_t val = max7456ReadRegister(reg);
	val &= ~bitmask;
	max7456WriteRegister(reg, val);
}

void max7456Reset()
{
	max7456WriteRegister(MAX7456_VM0, MAX7456_VM0_SRESET);
	_delay_ms(100);
}
void max7456SetPAL()
{
	max7456_setBitmask(MAX7456_VM0, MAX7456_VM0_PAL);
}
void max7456SetNTSC()
{
	max7456_clearBitmask(MAX7456_VM0, MAX7456_VM0_PAL);
}
void max7456EnableOSD()
{
	max7456_setBitmask(MAX7456_VM0, MAX7456_VM0_OSDON);
}
void max7456DisableOSD()
{
	max7456_clearBitmask(MAX7456_VM0, MAX7456_VM0_OSDON);
}
void max7456SetBackgroundBrightness(uint8_t val)
{
	if (val > 49) val = 49;
	val /= 7;
	max7456_setBits(MAX7456_VM1, MAX7456_VM1_GLVL_BIT, MAX7456_VM1_GLVL_LEN, val);
}
void max7456SetRowBrightness(uint8_t row, uint8_t black, uint8_t white)
{
	if (row >= MAX7456_ROWS)
		return;
	if (black > 3) black = 3;
	if (white > 3) white = 3;
	max7456_setBits(MAX7456_RB0 + row, MAX7456_RBn_CBL_BIT, MAX7456_RBn_CBL_LEN, black);
	max7456_setBits(MAX7456_RB0 + row, MAX7456_RBn_CWL_BIT, MAX7456_RBn_CWL_LEN, white);
}
void max7456SetAllRowsBrightness(uint8_t black, uint8_t white)
{
	int i;
	for (i = 0; i < MAX7456_ROWS; i++)
		max7456SetRowBrightness(i, black, white);
}
void max7456ClearDisplay()
{
	max7456_setBitmask(MAX7456_DMM, MAX7456_DMM_CDM);
	_delay_us(30);
}
void max7456SetHOffset(int8_t offset)
{
	if (offset < -32) offset = -32;
	if (offset > 31) offset = 31;
	max7456WriteRegister(MAX7456_HOS, (offset + 32) & 0x3f);
}
void max7456SetVOffset(int8_t offset)
{
	if (offset < -16) offset = -16;
	if (offset > 15) offset = 15;
	max7456WriteRegister(MAX7456_VOS, (offset + 16) & 0x1f);
}
void max7456SetDMA(uint16_t addr)
{
	max7456WriteRegister(MAX7456_DMAH, (addr >> 8) & 0x01);
	max7456WriteRegister(MAX7456_DMAL, addr & 0xff);
}
void max7456SetCursorXY(uint8_t x, uint8_t y)
{
	uint16_t addr = y * MAX7456_COLS + x;
	max7456SetDMA(addr);
}

const uint8_t defaultCodePage[128] =
{
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,
	0x00,    0, 0x48,    0,    0,    0,    0, 0x46,
	0x3F, 0x40,    0,    0, 0x45, 0x49, 0x41, 0x47,
	0x0A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x44, 0x43, 0x4A,    0, 0x4B, 0x42,
	0x4C, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
	0x22, 0x23, 0x24,    0,    0,    0,    0,    0,
	0, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
	0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E,    0,    0,    0,    0,    0
};
void max7456WriteText(const char* str)
{
	max7456_setBitmask(MAX7456_DMM, MAX7456_DMM_AI);
	while (*str)
	{
		max7456SPIEnableChip();
		max7456SPIRW(defaultCodePage[*str++]);
		max7456SPIDisableChip();
	}
	max7456SPIEnableChip();
	max7456SPIRW(0xff);
	max7456SPIDisableChip();
}
void max7456WriteTextXY(uint8_t x, uint8_t y, const char* str)
{
	max7456SetCursorXY(x, y);
	max7456WriteText(str);
}
void max7456WriteTextAlignRight(int x, int y, const char* str)
{
	max7456SetCursorXY(x - strlen(str), y);
	max7456WriteText(str);
}
void max7456WriteTextAlignCenter(int x, int y, const char* str)
{
	max7456SetCursorXY(x - strlen(str) / 2, y);
	max7456WriteText(str);
}

void max7456SetChar(uint8_t num, const uint8_t* data)
{
	int i;
	max7456WriteRegister(MAX7456_CMAH, num);
	for (i = 0; i < 54; i++)
	{
		max7456WriteRegister(MAX7456_CMAL, i);
		max7456WriteRegister(MAX7456_CMDI, *data++);
	}
	
	max7456WriteRegister(MAX7456_CMM, 0xa0);
	
	uint8_t s;
	do
	{
		s	 = max7456ReadRegister(MAX7456_STAT);
	}
	while (s & MAX7456_STAT_CMBUSY);
}
