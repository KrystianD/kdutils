/*
 * nmea.c
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "nmea.h"
#include <settings.h>

#define STATE_BEGIN   0
#define STATE_COMMAND 1
#define STATE_ARG     2
#define STATE_CHK     3
#define STATE_CR      4
#define STATE_LF      5

#ifndef NMEA_DEBUG
#define NMEA_DEBUG(x,...)
#endif

uint8_t nmea_state;
char nmea_cmd[7];
uint8_t nmea_idx;
char nmea_args[80];
uint8_t nmea_chk, nmea_val;

void nmeaParse();

void nmeaReset()
{
	nmea_state = STATE_BEGIN;
}
void nmeaProcess(char c)
{
	switch (nmea_state)
	{
	case STATE_BEGIN:
		if (c == '$')
		{
			nmea_idx = 0;
			nmea_chk = 0;
			nmea_state = STATE_COMMAND;
			NMEA_DEBUG("go STATE_COMMAND");
		}
		break;
	case STATE_COMMAND:
		if (c == ',')
		{
			nmea_state = STATE_ARG;
			nmea_idx = 0;
			NMEA_DEBUG("go STATE_ARG");
		}
		else
		{
			nmea_cmd[nmea_idx] = c;
			nmea_idx++;
			if (nmea_idx == sizeof(nmea_cmd) + 1)
			{
				nmea_state = STATE_BEGIN;
				NMEA_DEBUG("long cmd! go STATE_BEGIN");
			}
		}
		nmea_chk ^= c;
		break;
	case STATE_ARG:
		if (c == '*')
		{
			nmea_args[nmea_idx] = ',';
			nmea_val = 0;
			nmea_idx = 0;
			nmea_state = STATE_CHK;
			NMEA_DEBUG("go STATE_CHK");
		}
		else
		{
			nmea_args[nmea_idx] = c;
			nmea_idx++;
			if (nmea_idx == sizeof(nmea_args) + 1)
			{
				nmea_state = STATE_BEGIN;
				NMEA_DEBUG("long arg! go STATE_BEGIN");
			}
			nmea_chk ^= c;
		}
		break;
	case STATE_CHK:
		if (c == '\r')
		{
			NMEA_DEBUG("cal chk %02x", nmea_chk);
			if (nmea_chk == nmea_val)
			{
				nmea_state = STATE_CR;
				NMEA_DEBUG("go STATE_CR");
			}
			else
			{
				nmea_state = STATE_BEGIN;
				NMEA_DEBUG("inv chk! go STATE_BEGIN");
			}
		}
		else
		{
			nmea_val *= 16;
			if (c <= 'F' && c >= 'A')
				nmea_val += c - 'A' + 10;
			else if (c <= 'f' && c >= 'a')
				nmea_val += c - 'a' + 10;
			else
				nmea_val += c - '0';
			nmea_idx++;
			
			if (nmea_idx == 3)
			{
				nmea_state = STATE_BEGIN;
				NMEA_DEBUG("long chk! go STATE_BEGIN");
			}
			else
			{
				NMEA_DEBUG("chk %02x", nmea_val);
			}
		}
		break;
	case STATE_CR:
		if (c == '\n')
		{
			nmea_state = STATE_BEGIN;
			NMEA_DEBUG("go STATE_BEGIN");
			nmeaParse();
		}
		else
		{
			nmea_state = STATE_BEGIN;
			NMEA_DEBUG("no lf! go STATE_BEGIN");
		}
		break;
		// case STATE_LF:
		// break;
	}
}

static inline uint8_t nmeaParseIsNull()
{
	return nmea_args[nmea_idx] == ',';
}
static inline void nmeaParseSkip()
{
	while (nmea_args[nmea_idx++] != ',');
}
char nmeaParseGetChar()
{
	char c = nmea_args[nmea_idx];
	if (c != ',')
	{
		nmea_idx++;
	}
	else
	{
		c = 'x';
	}
	nmea_idx++;
	return c;
}
int16_t nmeaParseGetInt16()
{
	int16_t val = 0;
	uint8_t neg = 0;
	for (;;)
	{
		char c = nmea_args[nmea_idx];
		nmea_idx++;
		if (c == '-')
		{
			neg = 1;
		}
		else if (c == ',')
		{
			break;
		}
		else if (c == '.')
		{
			nmeaParseSkip();
			break;
		}
		else if (c >= '0' && c <= '9')
		{
			val *= 10;
			val += c - '0';
		}
	}
	return neg ? -val : val;
}
int32_t nmeaParseGetInt32()
{
	int32_t val = 0;
	uint8_t neg = 0;
	for (;;)
	{
		char c = nmea_args[nmea_idx];
		nmea_idx++;
		if (c == '-')
		{
			neg = 1;
		}
		else if (c == ',')
		{
			break;
		}
		else if (c == '.')
		{
			nmeaParseSkip();
			break;
		}
		else if (c >= '0' && c <= '9')
		{
			val *= 10;
			val += c - '0';
		}
	}
	return neg ? -val : val;
}
float nmeaParseGetFloat()
{
	float val = 0.0f, val2 = 0.0f;
	uint8_t neg = 0;
	uint8_t dec = 1;
	float div = 0.1f;
	for (;;)
	{
		char c = nmea_args[nmea_idx];
		nmea_idx++;
		if (c == '-')
		{
			neg = 1;
		}
		else if (c == ',')
		{
			break;
		}
		else if (c == '.')
		{
			dec = 0;
		}
		else if (c >= '0' && c <= '9')
		{
			if (dec)
			{
				val *= 10;
				val += c - '0';
			}
			else
			{
				val2 += (c - '0') * div;
				div *= 0.1f;
			}
		}
	}
	val += val2;
	return neg ? -val : val;
}

void nmeaParsePMTK();
void nmeaParseGPGSA();
void nmeaParseGPGGA();
void nmeaParseGPRMC();

void nmeaParse()
{
	nmea_idx = 0;
	if (nmea_cmd[0] == 'G' && nmea_cmd[1] == 'P')
	{
		if (nmea_cmd[2] == 'R')
		{
			if (nmea_cmd[3] == 'M')
			{
				if (nmea_cmd[4] == 'C') nmeaParseGPRMC();
			}
		}
		if (nmea_cmd[2] == 'G')
		{
			if (nmea_cmd[3] == 'S')
			{
				if (nmea_cmd[4] == 'A') nmeaParseGPGSA();
			}
			else if (nmea_cmd[3] == 'G')
			{
				if (nmea_cmd[4] == 'A') nmeaParseGPGGA();
			}
		}
	}
	else if (
			nmea_cmd[0] == 'P' &&
			nmea_cmd[1] == 'M' &&
			nmea_cmd[2] == 'T' &&
			nmea_cmd[3] == 'K')
	{
		nmeaParsePMTK();
	}
}

void nmeaParseGPGSA()
{
	NMEA_DEBUG("GPGSA");
	char mode = nmeaParseGetChar();
	int16_t type = nmeaParseGetInt16();
	
	int i;
	int16_t vals[12];
	for (i = 0; i < 12; i++)
	{
		if (nmeaParseIsNull())
		{
			nmeaParseSkip();
			vals[i] = -1;
		}
		else
		{
			vals[i] = nmeaParseGetInt16();
		}
	}
	
	nmeaGPGSA(mode, type, vals);
}
void nmeaParseGPGGA()
{
	NMEA_DEBUG("GPGGA");
	
	int32_t time = nmeaParseGetInt32();
	float latVal = nmeaParseGetFloat();
	char latSign = nmeaParseGetChar();
	float lonVal = nmeaParseGetFloat();
	char lonSign = nmeaParseGetChar();
	uint8_t fix = nmeaParseGetInt16();
	uint8_t sats = nmeaParseGetInt16();
	nmeaParseSkip();
	float alt = nmeaParseGetFloat();
	
	nmeaGPGGA(time, latVal, latSign, lonVal, lonSign, fix, sats, alt);
}
void nmeaParseGPRMC()
{
	NMEA_DEBUG("GPRMC");
	
	uint16_t time = nmeaParseGetInt16();
	char status = nmeaParseGetChar();
	float latVal = nmeaParseGetFloat();
	char latSign = nmeaParseGetChar();
	float lonVal = nmeaParseGetFloat();
	char lonSign = nmeaParseGetChar();
	float speed = nmeaParseGetFloat();
	float track = nmeaParseGetFloat();
	uint32_t date =nmeaParseGetInt16();
	float magVar = nmeaParseGetFloat();
	char magVarSign = nmeaParseGetChar();
	
	nmeaGPRMC(time, status, latVal, latSign, lonVal, lonSign, speed);
}
void nmeaParsePMTK()
{
	NMEA_DEBUG("PMTK");

	char *cmdStr = nmea_cmd + 4;

	int res = 0;
	res = (*cmdStr++ - '0') * 100;
	res = (*cmdStr++ - '0') * 10;
	res = (*cmdStr++ - '0') * 1;

	uint16_t cmd = nmeaParseGetInt16();
	uint16_t flag = nmeaParseGetInt16();

	nmeaPMTK(res, cmd, flag);
}
