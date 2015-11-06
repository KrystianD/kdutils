/*
 * MAX7456.h
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __MAX7456_H__
#define __MAX7456_H__

#include <stdint.h>

#define MAX7456_COLS  30    // Display Memory number of columns
#define MAX7456_ROWS  16    // Display Memory number of rows

extern void max7456SPIEnableChip();
extern void max7456SPIDisableChip();
extern uint8_t max7456SPIRW(uint8_t val);

void max7456WriteRegister(uint8_t reg, uint8_t val);
uint8_t max7456ReadRegister(uint8_t reg);

void max7456Reset();
void max7456SetPAL();
void max7456SetNTSC();
void max7456EnableOSD();
void max7456DisableOSD();
void max7456SetBackgroundBrightness(uint8_t val);
void max7456SetRowBrightness(uint8_t row, uint8_t black, uint8_t white);
void max7456SetAllRowsBrightness(uint8_t black, uint8_t white);
void max7456ClearDisplay();
void max7456SetHOffset(int8_t offset);
void max7456SetVOffset(int8_t offset);
void max7456SetDMA(uint16_t addr);
void max7456SetCursorXY(uint8_t x, uint8_t y);
void max7456WriteText(const char* str);
void max7456WriteTextXY(uint8_t x, uint8_t y, const char* str);
void max7456WriteTextAlignRight(int x, int y, const char* str);
void max7456WriteTextAlignCenter(int x, int y, const char* str);

// Character Memory
void max7456SetChar(uint8_t num, const uint8_t* data);

#endif
