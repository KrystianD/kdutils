#include "HP203B.h"

#define HP203B_STATE_NONE          0
#define HP203B_STATE_WAIT_FOR_CONV 1
#define HP203B_STATE_RESULT_VALID  2

uint8_t hp203b_state = HP203B_STATE_NONE;
uint32_t hp203b_convEndTime = 0;

uint8_t hp203bInit()
{

	return HP203B_SUCCESS;
}
uint8_t hp203BStartConversion(uint8_t OSR)
{
	switch (OSR)
	{
	case HP203B_OSR_128:
		hp203b_convEndTime = getTicks() + 5;
		break;
	case HP203B_OSR_256:
		hp203b_convEndTime = getTicks() + 9;
		break;
	case HP203B_OSR_512:
		hp203b_convEndTime = getTicks() + 17;
		break;
	case HP203B_OSR_1024:
		hp203b_convEndTime = getTicks() + 33;
		break;
	case HP203B_OSR_2048:
		hp203b_convEndTime = getTicks() + 66;
		break;
	case HP203B_OSR_4096:
		hp203b_convEndTime = getTicks() + 170;
		break;
	default:
		return HP203B_ERROR;
	}
	if (hp203bI2CSendCommand(HP203B_ADC_CVT | OSR | HP203B_CH_PRESSTEMP, 0, 0))
		return HP203B_ERROR;

	hp203b_state = HP203B_STATE_WAIT_FOR_CONV;
		
	return HP203B_SUCCESS;
}
uint8_t hp203bIsResultReady()
{
	if (hp203b_state == HP203B_STATE_RESULT_VALID)
	{
		return 1;
	}
	else if (hp203b_state == HP203B_STATE_WAIT_FOR_CONV && getTicks() >= hp203b_convEndTime)
	{
		hp203b_state = HP203B_STATE_RESULT_VALID;
		return 1;
	}
	else
	{
		return 0;
	}
}
uint8_t hp203bReadPressure(uint32_t* pressure)
{
	uint8_t d[3];
	if (hp203bI2CReadCommand(HP203B_READ_P, d, 3))
		return HP203B_ERROR;
	
	*pressure = (d[0] << 16) | (d[1] << 8) | d[2];

	return HP203B_SUCCESS;
}
uint8_t hp203bReadAltitude(uint32_t* altitude)
{
	uint8_t d[3];
	if (hp203bI2CReadCommand(HP203B_READ_A, d, 3))
		return HP203B_ERROR;
	
	*altitude = (d[0] << 16) | (d[1] << 8) | d[2];

	return HP203B_SUCCESS;
}
uint8_t hp203bProcess()
{
	return HP203B_SUCCESS;
}
