#ifndef __MPU6050_H__
#define __MPU6050_H__

#include <stdint.h>

#define MPU6050_SUCCESS     0
#define MPU6050_ERROR       1
#define MPU6050_VERIFYERROR 2

#define MPU6050_ADDR         0b01101000

#define MPU6050_SMPRT_DIV    0x19
#define MPU6050_CONFIG       0x1a
#define MPU6050_GYRO_CONFIG  0x1b
#define MPU6050_ACCEL_CONFIG 0x1c

#define MPU6050_FIFO_EN      0x23

#define MPU6050_INT_PIN_CFG  0x37
#define MPU6050_INT_ENABLE   0x38

#define MPU6050_ACCEL_XOUT_H 0x3b
#define MPU6050_ACCEL_XOUT_L 0x3c
#define MPU6050_ACCEL_YOUT_H 0x3d
#define MPU6050_ACCEL_YOUT_L 0x3e
#define MPU6050_ACCEL_ZOUT_H 0x3f
#define MPU6050_ACCEL_ZOUT_L 0x40

#define MPU6050_GYRO_XOUT_H  0x43
#define MPU6050_GYRO_XOUT_L  0x44
#define MPU6050_GYRO_YOUT_H  0x45
#define MPU6050_GYRO_YOUT_L  0x46
#define MPU6050_GYRO_ZOUT_H  0x47
#define MPU6050_GYRO_ZOUT_L  0x48

#define MPU6050_USER_CTRL    0x6a
#define MPU6050_PWR1         0x6b
#define MPU6050_PWR2         0x6c

#define MPU6050_FIFO_COUNT_H 0x72
#define MPU6050_FIFO_COUNT_L 0x73
#define MPU6050_FIFO_R_W     0x74

// bits
#define MPU6050_CONFIG_DLPF_BIT 0
#define MPU6050_CONFIG_DLPF_LEN 3

#define MPU6050_GYRO_CONFIG_FS_SEL_BIT 3
#define MPU6050_GYRO_CONFIG_FS_SEL_LEN 3

#define MPU6050_GYRO_SCALE_250  0
#define MPU6050_GYRO_SCALE_500  1
#define MPU6050_GYRO_SCALE_1000 2
#define MPU6050_GYRO_SCALE_2000 3

#define MPU6050_ACCEL_CONFIG_XA_ST 0x80
#define MPU6050_ACCEL_CONFIG_YA_ST 0x40
#define MPU6050_ACCEL_CONFIG_ZA_ST 0x20
#define MPU6050_ACCEL_CONFIG_AFS_SEL_BIT 3
#define MPU6050_ACCEL_CONFIG_AFS_SEL_LEN 3

#define MPU6050_ACCEL_SCALE_2  0
#define MPU6050_ACCEL_SCALE_4  1
#define MPU6050_ACCEL_SCALE_8  2
#define MPU6050_ACCEL_SCALE_16 3

#define MPU6050_FIFO_EN_TEMP_FIFO_EN  0x80
#define MPU6050_FIFO_EN_XG_FIFO_EN    0x40
#define MPU6050_FIFO_EN_YG_FIFO_EN    0x20
#define MPU6050_FIFO_EN_ZG_FIFO_EN    0x10
#define MPU6050_FIFO_EN_ACCEL_FIFO_EN 0x08
#define MPU6050_FIFO_EN_SLV2_FIFO_EN  0x04
#define MPU6050_FIFO_EN_SLV1_FIFO_EN  0x02
#define MPU6050_FIFO_EN_SLV0_FIFO_EN  0x01

#define MPU6050_INT_PIN_CFG_INT_LEVEL       0x80
#define MPU6050_INT_PIN_CFG_INT_OPEN        0x40
#define MPU6050_INT_PIN_CFG_LATCH_INT_EN    0x20
#define MPU6050_INT_PIN_CFG_INT_RD_CLEAR    0x10
#define MPU6050_INT_PIN_CFG_FSYNC_INT_LEVEL 0x08
#define MPU6050_INT_PIN_CFG_FSYNC_INT_EN    0x04
#define MPU6050_INT_PIN_CFG_I2C_BYPASS_EN   0x02

#define MPU6050_INT_ENABLE_MOT_EN         0x40
#define MPU6050_INT_ENABLE_FIFO_OFLOW_EN  0x10
#define MPU6050_INT_ENABLE_I2C_MST_INT_EN 0x08
#define MPU6050_INT_ENABLE_DATA_RDY_EN    0x01

#define MPU6050_USER_CTRL_FIFO_EN        0x40
#define MPU6050_USER_CTRL_I2C_MST_EN     0x20
#define MPU6050_USER_CTRL_I2C_IF_DIS     0x10
#define MPU6050_USER_CTRL_FIFO_RESET     0x04
#define MPU6050_USER_CTRL_I2C_MST_RESET  0x02
#define MPU6050_USER_CTRL_SIG_COND_RESET 0x01

#define MPU6050_PWR1_DEVICE_RESET  0x80
#define MPU6050_PWR1_SLEEP         0x40
#define MPU6050_PWR1_CYCLE         0x20
#define MPU6050_PWR1_TEMP_DIS      0x08
#define MPU6050_PWR1_CLKSEL_BIT    0
#define MPU6050_PWR1_CLKSEL_LEN    3

#define MPU6050_CLOCK_INTERNAL     0x00
#define MPU6050_CLOCK_PLL_XGYRO    0x01
#define MPU6050_CLOCK_PLL_YGYRO    0x02
#define MPU6050_CLOCK_PLL_ZGYRO    0x03
#define MPU6050_CLOCK_PLL_EXT32K   0x04
#define MPU6050_CLOCK_PLL_EXT19M   0x05
#define MPU6050_CLOCK_KEEP_RESET   0x07

extern uint8_t mpu6050I2CReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t mpu6050I2CSendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

typedef struct
{
	int16_t ax, ay, az, temp, gx, gy, gz;
} MPU6050_Data;

static inline int16_t mpu6050GetAccelSensitivity(uint8_t opt)
{
	switch (opt)
	{
	default:
	case MPU6050_ACCEL_SCALE_2:
		return 16384;
	case MPU6050_ACCEL_SCALE_4:
		return 8192;
	case MPU6050_ACCEL_SCALE_8:
		return 4096;
	case MPU6050_ACCEL_SCALE_16:
		return 2048;
	}
}
static inline float mpu6050GetGyroSensitivity(uint8_t opt)
{
	switch (opt)
	{
	default:
	case MPU6050_GYRO_SCALE_250:
		return 131.0f;
	case MPU6050_GYRO_SCALE_500:
		return 65.5f;
	case MPU6050_GYRO_SCALE_1000:
		return 32.8f;
	case MPU6050_GYRO_SCALE_2000:
		return 16.4f;
	}
}

uint8_t mpu6050SetSampleRateDiv(uint8_t val);
uint8_t mpu6050SetDLPF_CFG(uint8_t val);
uint8_t mpu6050SetGyroScale(uint8_t val);
uint8_t mpu6050SetAccelScale(uint8_t val);
uint8_t mpu6050SetClockSource(uint8_t val);
uint8_t mpu6050EnableSleep(uint8_t enable);
uint8_t mpu6050EnableCycle(uint8_t enable);
uint8_t mpu6050DisableTemp(uint8_t enable);
uint8_t mpu6050EnableAccelSelfTest(uint8_t enable);
uint8_t mpu6050EnableGyroSelfTest(uint8_t enable);

uint8_t mpu6050SetInterruptMode(uint8_t activeLow);
uint8_t mpu6050SetInterruptDrive(uint8_t openDrain);
uint8_t mpu6050SetInterruptLatch(uint8_t latch);
uint8_t mpu6050SetInterruptLatchClear(uint8_t clear);

uint8_t mpu6050SetIntFIFOBufferOverflowEnabled(uint8_t enabled);
uint8_t mpu6050SetIntDataReadyEnabled(uint8_t enabled);

uint8_t mpu6050ResetFIFO();
uint8_t mpu6050EnableFIFO(uint8_t accel, uint8_t gyro);
int16_t mpu6050GetFIFOCount();

uint8_t mpu6050PerformDeviceReset();
uint8_t mpu6050IsBeingDeviceReset(uint8_t* res);
uint8_t mpu6050GetData(MPU6050_Data* data);
uint8_t mpu6050BufferToData(uint8_t* buf, MPU6050_Data* data);
uint8_t mpu6050GetDataFIFOGyro(MPU6050_Data* data);
uint8_t mpu6050GetDataFIFOAccelGyro(MPU6050_Data* data);
uint16_t mpu6050GetTemp(MPU6050_Data* data);  // temp in 0.1

void mpu6050Info();
void mpu6050PrintData(MPU6050_Data* data);

#endif
