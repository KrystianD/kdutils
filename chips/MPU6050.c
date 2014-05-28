#include "MPU6050.h"

#include <i2c.h>
#include <myprintf.h>
#include <delay.h>

#define SWAP16(x) (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8))

uint8_t mpu6050WriteBit(uint8_t reg, uint8_t bit, uint8_t enabled)
{
	uint8_t val;
	
	if (mpu6050I2CReadCommand(reg, &val, 1))
		return MPU6050_ERROR;
		
	if (enabled)
		val |= bit;
	else
		val &= ~bit;
		
	if (mpu6050I2CSendCommand(reg, &val, 1))
		return MPU6050_ERROR;
	if (mpu6050I2CReadCommand(reg, &val, 1))
		return MPU6050_ERROR;
		
	if (enabled && !(val & bit))
		return MPU6050_VERIFYERROR;
	if (!enabled && (val & bit))
		return MPU6050_VERIFYERROR;
		
	return MPU6050_SUCCESS;
}
uint8_t mpu6050WriteBits(uint8_t reg, uint8_t bit, uint8_t len, uint8_t value)
{
	uint8_t val;
	
	if (mpu6050I2CReadCommand(reg, &val, 1))
		return MPU6050_ERROR;
		
	uint8_t mask = ((1 << len) - 1) << bit;
	val &= ~mask;
	val |= value << bit;
	
	if (mpu6050I2CSendCommand(reg, &val, 1))
		return MPU6050_ERROR;
	if (mpu6050I2CReadCommand(reg, &val, 1))
		return MPU6050_ERROR;
		
	if ((val & mask) != (value << bit))
		return MPU6050_VERIFYERROR;
		
	return MPU6050_SUCCESS;
}
uint8_t mpu6050WriteByte(uint8_t reg, uint8_t val)
{
	uint8_t newval;
	
	if (mpu6050I2CSendCommand(reg, &val, 1))
		return MPU6050_ERROR;
	if (mpu6050I2CReadCommand(reg, &newval, 1))
		return MPU6050_ERROR;
		
	if (val != newval)
		return MPU6050_VERIFYERROR;
		
	return MPU6050_SUCCESS;
}

uint8_t mpu6050SetSampleRateDiv(uint8_t val)
{
	return mpu6050WriteByte(MPU6050_SMPRT_DIV, val);
}
uint8_t mpu6050SetDLPF_CFG(uint8_t val)
{
	return mpu6050WriteBits(
	         MPU6050_CONFIG,
	         MPU6050_CONFIG_DLPF_BIT,
	         MPU6050_CONFIG_DLPF_LEN,
	         val);
}
uint8_t mpu6050SetGyroScale(uint8_t val)
{
	return mpu6050WriteBits(
	         MPU6050_GYRO_CONFIG,
	         MPU6050_GYRO_CONFIG_FS_SEL_BIT,
	         MPU6050_GYRO_CONFIG_FS_SEL_LEN,
	         val);
}
uint8_t mpu6050SetAccelScale(uint8_t val)
{
	return mpu6050WriteBits(
	         MPU6050_ACCEL_CONFIG,
	         MPU6050_ACCEL_CONFIG_AFS_SEL_BIT,
	         MPU6050_ACCEL_CONFIG_AFS_SEL_LEN,
	         val);
}
uint8_t mpu6050SetClockSource(uint8_t val)
{
	return mpu6050WriteBits(
	         MPU6050_PWR1,
	         MPU6050_PWR1_CLKSEL_BIT,
	         MPU6050_PWR1_CLKSEL_LEN,
	         val);
}
uint8_t mpu6050EnableSleep(uint8_t enable)
{
	return mpu6050WriteBit(MPU6050_PWR1, MPU6050_PWR1_SLEEP, enable);
}
uint8_t mpu6050EnableCycle(uint8_t enable)
{
	return mpu6050WriteBit(MPU6050_PWR1, MPU6050_PWR1_CYCLE, enable);
}
uint8_t mpu6050DisableTemp(uint8_t enable)
{
	return mpu6050WriteBit(MPU6050_PWR1, MPU6050_PWR1_TEMP_DIS, enable);
}
uint8_t mpu6050EnableAccelSelfTest(uint8_t enable)
{
	uint8_t reg;
	
	if (mpu6050I2CReadCommand(MPU6050_ACCEL_CONFIG, &reg, 1))
		return MPU6050_ERROR;
		
	if (enable)
		reg |= MPU6050_ACCEL_CONFIG_XA_ST | MPU6050_ACCEL_CONFIG_YA_ST | MPU6050_ACCEL_CONFIG_ZA_ST;
	else
		reg &= ~(MPU6050_ACCEL_CONFIG_XA_ST | MPU6050_ACCEL_CONFIG_YA_ST | MPU6050_ACCEL_CONFIG_ZA_ST);
		
	if (mpu6050I2CSendCommand(MPU6050_ACCEL_CONFIG, &reg, 1))
		return MPU6050_ERROR;
		
	return MPU6050_SUCCESS;
}
uint8_t mpu6050ResetFIFO()
{
	if (mpu6050WriteBit(MPU6050_USER_CTRL, MPU6050_USER_CTRL_FIFO_RESET, 1))    return MPU6050_ERROR;
	return MPU6050_SUCCESS;
}
uint8_t mpu6050EnableFIFO(uint8_t accel, uint8_t gyro)
{
	if (mpu6050WriteBit(MPU6050_FIFO_EN, MPU6050_FIFO_EN_XG_FIFO_EN, gyro))     return MPU6050_ERROR;
	if (mpu6050WriteBit(MPU6050_FIFO_EN, MPU6050_FIFO_EN_YG_FIFO_EN, gyro))     return MPU6050_ERROR;
	if (mpu6050WriteBit(MPU6050_FIFO_EN, MPU6050_FIFO_EN_ZG_FIFO_EN, gyro))     return MPU6050_ERROR;
	if (mpu6050WriteBit(MPU6050_FIFO_EN, MPU6050_FIFO_EN_ACCEL_FIFO_EN, accel)) return MPU6050_ERROR;

	if (mpu6050WriteBit(MPU6050_USER_CTRL, MPU6050_USER_CTRL_FIFO_EN, 0))       return MPU6050_ERROR;
	mpu6050WriteBit(MPU6050_USER_CTRL, MPU6050_USER_CTRL_FIFO_RESET, 1);
	if (mpu6050WriteBit(MPU6050_USER_CTRL, MPU6050_USER_CTRL_FIFO_EN, 1))       return MPU6050_ERROR;
	
	return MPU6050_SUCCESS;
}
int16_t mpu6050GetFIFOCount()
{
	int16_t len;
	if (mpu6050I2CReadCommand(MPU6050_FIFO_COUNT_H, (uint8_t*)&len, 2))
		return -1;
	len = SWAP16(len);
	return len;
}
uint8_t mpu6050EnableGyroSelfTest(uint8_t enable)
{
}

uint8_t mpu6050SetInterruptMode(uint8_t activeLow)
{
	return mpu6050WriteBit(MPU6050_INT_PIN_CFG, MPU6050_INT_PIN_CFG_INT_LEVEL, activeLow);
}
uint8_t mpu6050SetInterruptDrive(uint8_t openDrain)
{
	return mpu6050WriteBit(MPU6050_INT_PIN_CFG, MPU6050_INT_PIN_CFG_INT_OPEN, openDrain);
}
uint8_t mpu6050SetInterruptLatch(uint8_t latch)
{
	return mpu6050WriteBit(MPU6050_INT_PIN_CFG, MPU6050_INT_PIN_CFG_LATCH_INT_EN, latch);
}
uint8_t mpu6050SetInterruptLatchClear(uint8_t clear)
{
	return mpu6050WriteBit(MPU6050_INT_PIN_CFG, MPU6050_INT_PIN_CFG_INT_RD_CLEAR, clear);
}

uint8_t mpu6050SetIntFIFOBufferOverflowEnabled(uint8_t enabled)
{
	return mpu6050WriteBit(MPU6050_INT_ENABLE, MPU6050_INT_ENABLE_FIFO_OFLOW_EN, enabled);
}
uint8_t mpu6050SetIntDataReadyEnabled(uint8_t enabled)
{
	return mpu6050WriteBit(MPU6050_INT_ENABLE, MPU6050_INT_ENABLE_DATA_RDY_EN, enabled);
}

uint8_t mpu6050PerformDeviceReset()
{
	uint8_t reg;
	reg = MPU6050_PWR1_DEVICE_RESET;
	
	if (mpu6050I2CSendCommand(MPU6050_PWR1, &reg, 1))
		return MPU6050_ERROR;
		
	return MPU6050_SUCCESS;
}
uint8_t mpu6050IsBeingDeviceReset(uint8_t* res)
{
	uint8_t reg;
	
	if (mpu6050I2CReadCommand(MPU6050_PWR1, &reg, 1))
		return MPU6050_ERROR;
		
	uint8_t v = reg & MPU6050_PWR1_DEVICE_RESET;
	*res = v ? 1 : 0;
	
	return MPU6050_SUCCESS;
}
uint8_t mpu6050GetData(MPU6050_Data* data)
{
	if (mpu6050I2CReadCommand(MPU6050_ACCEL_XOUT_H, (uint8_t*)&data->ax, 14))
		return MPU6050_ERROR;
	data->ax = SWAP16(data->ax);
	data->ay = SWAP16(data->ay);
	data->az = SWAP16(data->az);
	data->temp = SWAP16(data->temp);
	data->gx = SWAP16(data->gx);
	data->gy = SWAP16(data->gy);
	data->gz = SWAP16(data->gz);
	return MPU6050_SUCCESS;
}
uint8_t mpu6050BufferToData(uint8_t* buf, MPU6050_Data* data)
{
	memcpy(data, buf, sizeof(*data));
	data->ax = SWAP16(data->ax);
	data->ay = SWAP16(data->ay);
	data->az = SWAP16(data->az);
	data->temp = SWAP16(data->temp);
	data->gx = SWAP16(data->gx);
	data->gy = SWAP16(data->gy);
	data->gz = SWAP16(data->gz);
	return MPU6050_SUCCESS;
}
uint8_t mpu6050GetDataFIFOGyro(MPU6050_Data* data)
{
	if (mpu6050I2CReadCommand(MPU6050_FIFO_R_W, (uint8_t*)&data->gx, 6))
		return MPU6050_ERROR;
	data->gx = SWAP16(data->gx);
	data->gy = SWAP16(data->gy);
	data->gz = SWAP16(data->gz);
	return MPU6050_SUCCESS;
}
uint8_t mpu6050GetDataFIFOAccelGyro(MPU6050_Data* data)
{
	int16_t buffer[6];
	if (mpu6050I2CReadCommand(MPU6050_FIFO_R_W, (uint8_t*)buffer, 12))
		return MPU6050_ERROR;
	data->ax = SWAP16(buffer[0]);
	data->ay = SWAP16(buffer[1]);
	data->az = SWAP16(buffer[2]);
	data->gx = SWAP16(buffer[3]);
	data->gy = SWAP16(buffer[4]);
	data->gz = SWAP16(buffer[5]);
	return MPU6050_SUCCESS;
}
uint16_t mpu6050GetTemp(MPU6050_Data* data)
{
	uint16_t temp = data->temp / 34 + 365;
	return temp;
}

void mpu6050Info()
{
	uint8_t reg;
	
	if (mpu6050I2CReadCommand(MPU6050_CONFIG, &reg, 1))
		return;
		
	myprintf("CONFIG: 0x%02x\r\n", reg);
	
	if (mpu6050I2CReadCommand(MPU6050_ACCEL_CONFIG, &reg, 1))
		return;
		
	myprintf("ACCEL_CONFIG: 0x%02x\r\n", reg);
	
	if (mpu6050I2CReadCommand(MPU6050_PWR1, &reg, 1))
		return;
		
	myprintf("PWM_MGMT_1: 0x%02x\r\n", reg);
}

void mpu6050PrintData(MPU6050_Data* data)
{
	myprintf("aX: %6d aY: %6d aZ: %6d gX: %6d gY: %6d gZ: %6d temp: %6d\r\n",
	         data->ax, data->ay, data->az, data->gx, data->gy, data->gz, mpu6050GetTemp(data));
}
