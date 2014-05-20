#include "BMP085.h"

#include <math.h>

#define BMP085_STATE_READING_NONE               0
#define BMP085_STATE_READING_WAIT_FOR_TEMP      1
#define BMP085_STATE_READING_WAIT_FOR_PRESSTEMP 2
#define BMP085_STATE_READING_WAIT_FOR_PRESS     3
#define BMP085_STATE_READING_RESULT_VALID       4

#define SWAP16(x) (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8))

int16_t bmp085Temperature;
int32_t bmp085Pressure;

int32_t AC1, AC2, AC3;
uint32_t AC4, AC5, AC6;
int32_t B1, B2;
int32_t MB, MC, MD;
int32_t B5; // tmp

uint8_t bmp085_state;
uint8_t bmp085_oss;
uint32_t bmp085_convEndTime;

// internal
uint8_t bmp085_readTemperature();
uint8_t bmp085_readPressure();

uint8_t bmp085Init()
{
	int i;
	
	uint16_t eeprom[11];
	bmp085I2CReadCommand(0xaa, (uint8_t*)eeprom, sizeof(eeprom));
	
	for (i = 0; i < 11; i++)
		eeprom[i] = SWAP16(eeprom[i]);
		
	uint16_t *eu16 = eeprom;
	int16_t *ei16 = (int16_t*)eeprom;
	
	AC1 = ei16[0];
	AC2 = ei16[1];
	AC3 = ei16[2];
	AC4 = eu16[3];
	AC5 = eu16[4];
	AC6 = eu16[5];
	B1 = ei16[6];
	B2 = ei16[7];
	MB = ei16[8];
	MC = ei16[9];
	MD = ei16[10];
	
	bmp085_state = BMP085_STATE_READING_NONE;
}

uint8_t bmp085StartTempConv()
{
	bmp085_convEndTime = getTicks() + 5;
	bmp085_state = BMP085_STATE_READING_WAIT_FOR_TEMP;
	return bmp085I2CSendCommand(0xf4, (const uint8_t*)"\x2e", 1);
}
uint8_t bmp085StartPressureConv(int oss)
{
	bmp085_convEndTime = getTicks() + 5;
	bmp085_oss = oss;
	bmp085_state = BMP085_STATE_READING_WAIT_FOR_PRESSTEMP;
	return bmp085I2CSendCommand(0xf4, (const uint8_t*)"\x2e", 1);
}
uint8_t bmp085IsResultReady()
{
	return bmp085_state == BMP085_STATE_READING_RESULT_VALID;
}
uint8_t bmp085IsBusy()
{
	return
	  bmp085_state == BMP085_STATE_READING_WAIT_FOR_TEMP ||
	  bmp085_state == BMP085_STATE_READING_WAIT_FOR_PRESSTEMP ||
	  bmp085_state == BMP085_STATE_READING_WAIT_FOR_PRESS;
}
uint8_t bmp085Process()
{
	switch (bmp085_state)
	{
	case BMP085_STATE_READING_NONE:
		break;
	case BMP085_STATE_READING_WAIT_FOR_TEMP:
		if (getTicks() >= bmp085_convEndTime)
		{
			bmp085_readTemperature();
			bmp085_state = BMP085_STATE_READING_RESULT_VALID;
		}
		break;
	case BMP085_STATE_READING_WAIT_FOR_PRESSTEMP:
		if (getTicks() >= bmp085_convEndTime)
		{
			bmp085_readTemperature();
			bmp085_state = BMP085_STATE_READING_WAIT_FOR_PRESS;
			switch (bmp085_oss)
			{
			case 0:
				bmp085_convEndTime = getTicks() + 5;
				bmp085I2CSendCommand(0xf4, (const uint8_t*)"\x34", 1);
				break;
			case 1:
				bmp085_convEndTime = getTicks() + 8;
				bmp085I2CSendCommand(0xf4, (const uint8_t*)"\x74", 1);
				break;
			case 2:
				bmp085_convEndTime = getTicks() + 14;
				bmp085I2CSendCommand(0xf4, (const uint8_t*)"\xb4", 1);
				break;
			case 3:
				bmp085I2CSendCommand(0xf4, (const uint8_t*)"\xf4", 1);
				bmp085_convEndTime = getTicks() + 26;
				break;
			}
		}
		break;
	case BMP085_STATE_READING_WAIT_FOR_PRESS:
		if (getTicks() >= bmp085_convEndTime)
		{
			bmp085_readPressure();
			bmp085_state = BMP085_STATE_READING_RESULT_VALID;
		}
		break;
	}
	return BMP085_SUCCESS;
}

float bmp085GetAltitude()
{
	return 44330.0f * (1 - powf((float)bmp085Pressure / 101325.0f, 0.19029496f));
}

uint8_t bmp085_readTemperature()
{
	uint8_t data[2];
	if (bmp085I2CReadCommand(0xf6, data, 2))
		return 1;
		
	uint8_t msb, lsb;
	msb = data[0];
	lsb = data[1];
	
	int32_t ut = ((int32_t)msb << 8) | (int32_t)lsb;
	
	int32_t X1, X2;
	X1 = (ut - AC6) * AC5 >> 15;
	X2 = (MC << 11) / (X1 + MD);
	B5 = X1 + X2;
	int32_t T = (B5 + 8) >> 4;
	
	bmp085Temperature = T;
	return BMP085_SUCCESS;
}
uint8_t bmp085_readPressure()
{
	uint8_t data[3];
	if (bmp085I2CReadCommand(0xf6, data, 3))
		return 1;
		
	uint8_t msb, lsb, xlsb;
	msb = data[0];
	lsb = data[1];
	xlsb = data[2];
	
	int32_t up = (((uint32_t)msb << 16) | ((uint32_t)lsb << 8) | (uint32_t)xlsb) >> (8 - bmp085_oss);
	int32_t p;
	
	int32_t X1, X2;
	int32_t B6 = B5 - 4000l;
	X1 = (B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = (AC2 * B6) >> 11;
	int32_t X3 = X1 + X2;
	int32_t B3 = (((AC1 * 4 + X3) << bmp085_oss) + 2) >> 2;
	X1 = (AC3 * B6) >> 13;
	X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	uint32_t B4 = (AC4 * (uint32_t)(X3 + 32768l)) >> 15;
	uint32_t B7 = ((uint32_t)up - B3) * (50000l >> bmp085_oss);
	if (B7 < 0x80000000) p = (B7 << 1) / B4;
	else p = (B7 / B4) << 1;
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038l) >> 16;
	X2 = (p * -7357l) >> 16;
	p = p + ((X1 + X2 + 3791l) >> 4);
	
	bmp085Pressure = p;
	return BMP085_SUCCESS;
}
