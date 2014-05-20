#include "HMC5883.h"

#define SWAP16(x) (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8))

uint8_t hmc5883l_writeReg(uint8_t reg, uint8_t val)
{
	uint8_t newval;
	
	if (hmc5883I2CSendCommand(reg, &val, 1))
		return HMC5883_ERROR;
	if (hmc5883I2CReadCommand(reg, &newval, 1))
		return HMC5883_ERROR;
		
	if (val != newval)
		return HMC5883_VERIFYERROR;
		
	return HMC5883_SUCCESS;
}

uint8_t hmc5883SetConfig(uint8_t ca, uint8_t cb)
{
	if (hmc5883l_writeReg(HMC5883_CONFA, ca))
		return HMC5883_ERROR;
	if (hmc5883l_writeReg(HMC5883_CONFB, cb))
		return HMC5883_ERROR;
		
	return HMC5883_SUCCESS;
}
uint8_t hmc5883SetContinousMode()
{
	uint8_t reg = HMC5883_MODE_CONTINUOUS;
	
	if (hmc5883l_writeReg(HMC5883_MODE, reg))
		return HMC5883_ERROR;
		
	return HMC5883_SUCCESS;
}
uint8_t hmc5883GetData(HMC5883_Data* data)
{
	if (hmc5883I2CReadCommand(HMC5883_DATA_X_MSB, (uint8_t*)data, 6))
		return HMC5883_ERROR;
	data->mx = SWAP16(data->mx);
	data->my = SWAP16(data->my);
	data->mz = SWAP16(data->mz);
	
	return HMC5883_SUCCESS;
}
