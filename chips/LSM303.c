#include "LSM303.h"
#include <myprintf.h>

uint8_t lsm303_writeBit(uint8_t reg, uint8_t bit, uint8_t enabled)
{
	uint8_t val;
	
	if (lsm303I2CReadCommand(reg, &val, 1))
		return LSM303_ERROR;
		
	if (enabled)
		val |= bit;
	else
		val &= ~bit;
		
	if (lsm303I2CSendCommand(reg, &val, 1))
		return LSM303_ERROR;
	if (lsm303I2CReadCommand(reg, &val, 1))
		return LSM303_ERROR;
		
	if (enabled && !(val & bit))
		return LSM303_VERIFYERROR;
	if (!enabled && (val & bit))
		return LSM303_VERIFYERROR;
		
	return LSM303_SUCCESS;
}
uint8_t lsm303_writeBits(uint8_t reg, uint8_t bit, uint8_t len, uint8_t value)
{
	uint8_t val;
	
	if (lsm303I2CReadCommand(reg, &val, 1))
		return LSM303_ERROR;
		
	uint8_t mask = ((1 << len) - 1) << bit;
	val &= ~mask;
	val |= value << bit;
	
	if (lsm303I2CSendCommand(reg, &val, 1))
		return LSM303_ERROR;
	if (lsm303I2CReadCommand(reg, &val, 1))
		return LSM303_ERROR;
		
	if ((val & mask) != (value << bit))
		return LSM303_VERIFYERROR;
		
	return LSM303_SUCCESS;
}
uint8_t lsm303_writeByte(uint8_t reg, uint8_t val)
{
	uint8_t newval;
	
	if (lsm303I2CSendCommand(reg, &val, 1))
		return LSM303_ERROR;
	if (lsm303I2CReadCommand(reg, &newval, 1))
		return LSM303_ERROR;
		
	if (val != newval)
		return LSM303_VERIFYERROR;
		
	return LSM303_SUCCESS;
}

uint8_t lsm303Presence()
{
	uint8_t d[1];
	if (lsm303I2CReadCommand(LSM303_WHO_AM_I, d, 1))
		return 0;
	return d[0] == 0b01001001 ? 1 : 0;
}
uint8_t lsm303PerformDeviceReset()
{
	uint8_t reg;
	reg = LSM303_CTRL0_BOOT;
	
	if (lsm303I2CSendCommand(LSM303_CTRL0, &reg, 1))
		return LSM303_ERROR;
		
	return LSM303_SUCCESS;
}
uint8_t lsm303IsBeingDeviceReset(uint8_t* res)
{
	uint8_t reg;
	
	if (lsm303I2CReadCommand(LSM303_CTRL0, &reg, 1))
		return LSM303_ERROR;
		
	uint8_t v = reg & LSM303_CTRL0_BOOT;
	*res = v ? 1 : 0;
	
	return LSM303_SUCCESS;
}

uint8_t lsm303EnableBDU()
{
	return lsm303_writeBit(LSM303_CTRL1, LSM303_CTRL1_BDU, 1);
}
uint8_t lsm303DisableBDU()
{
	return lsm303_writeBit(LSM303_CTRL1, LSM303_CTRL1_BDU, 0);
}
uint8_t lsm303SetMagnetResolution(uint8_t res)
{
	return lsm303_writeBits(LSM303_CTRL5, LSM303_CTRL5_M_RES_BIT, LSM303_CTRL5_M_RES_LEN, res);
}
uint8_t lsm303SetMagnetDataRate(uint8_t rate)
{
	return lsm303_writeBits(LSM303_CTRL5, LSM303_CTRL5_M_ODR_BIT, LSM303_CTRL5_M_ODR_LEN, rate);
}
uint8_t lsm303SetMagnetScale(uint8_t scale)
{
	return lsm303_writeByte(LSM303_CTRL6, scale);
}

uint8_t lsm303SetMagnetContinuousMode()
{
	return lsm303_writeBits(LSM303_CTRL7, LSM303_CTRL7_MD_BIT, LSM303_CTRL7_MD_LEN, LSM303_MAGNET_CONTINOUS);
}
uint8_t lsm303SetMagnetSingleMode()
{
	return lsm303_writeBits(LSM303_CTRL7, LSM303_CTRL7_MD_BIT, LSM303_CTRL7_MD_LEN, LSM303_MAGNET_SINGLE);
}
uint8_t lsm303SetMagnetPowerDownMode()
{
	return lsm303_writeBits(LSM303_CTRL7, LSM303_CTRL7_MD_BIT, LSM303_CTRL7_MD_LEN, LSM303_MAGNET_POWERDOWN);
}

uint8_t lsm303ReadMagnet(LSM303_MAGNET_DATA* data)
{
	// in order to read multiple bytes, it is necessary to set most significant bit of the register address
	return lsm303I2CReadCommand(LSM303_OUT_X_L_M | 0x80, (uint8_t*)data, 6);
}
