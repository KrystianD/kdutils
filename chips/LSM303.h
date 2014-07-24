#ifndef __LSM303_H__
#define __LSM303_H__

#include <stdint.h>

#define LSM303_SUCCESS     0
#define LSM303_ERROR       1
#define LSM303_VERIFYERROR 2

#define LSM303_ADDR_SA0_VCC 0b0011101
#define LSM303_ADDR_SA0_GND 0b0011110

#define LSM303_TEMP_OUT_L   0x05
#define LSM303_TEMP_OUT_H   0x06

#define LSM303_STATUS_M     0x07

#define LSM303_OUT_X_L_M    0x08
#define LSM303_OUT_X_H_M    0x09
#define LSM303_OUT_Y_L_M    0x0a
#define LSM303_OUT_Y_H_M    0x0b
#define LSM303_OUT_Z_L_M    0x0c
#define LSM303_OUT_Z_H_M    0x0d

#define LSM303_WHO_AM_I     0x0F

#define LSM303_INT_CTRL_M   0x12
#define LSM303_INT_SRC_M    0x13
#define LSM303_INT_THS_L_M  0x14
#define LSM303_INT_THS_H_M  0x15

#define LSM303_OFFSET_X_L_M 0x16
#define LSM303_OFFSET_X_H_M 0x17
#define LSM303_OFFSET_Y_L_M 0x18
#define LSM303_OFFSET_Y_H_M 0x19
#define LSM303_OFFSET_Z_L_M 0x1A
#define LSM303_OFFSET_Z_H_M 0x1B
#define LSM303_REFERENCE_X  0x1C
#define LSM303_REFERENCE_Y  0x1D
#define LSM303_REFERENCE_Z  0x1E

#define LSM303_CTRL0        0x1F
#define LSM303_CTRL1        0x20
#define LSM303_CTRL2        0x21
#define LSM303_CTRL3        0x22
#define LSM303_CTRL4        0x23
#define LSM303_CTRL5        0x24
#define LSM303_CTRL6        0x25
#define LSM303_CTRL7        0x26
#define LSM303_STATUS_A     0x27

#define LSM303_OUT_X_L_A    0x28
#define LSM303_OUT_X_H_A    0x29
#define LSM303_OUT_Y_L_A    0x2A
#define LSM303_OUT_Y_H_A    0x2B
#define LSM303_OUT_Z_L_A    0x2C
#define LSM303_OUT_Z_H_A    0x2D

#define LSM303_FIFO_CTRL    0x2E
#define LSM303_FIFO_SRC     0x2F

#define LSM303_IG_CFG1      0x30
#define LSM303_IG_SRC1      0x31
#define LSM303_IG_THS1      0x32
#define LSM303_IG_DUR1      0x33
#define LSM303_IG_CFG2      0x34
#define LSM303_IG_SRC2      0x35
#define LSM303_IG_THS2      0x36
#define LSM303_IG_DUR2      0x37

#define LSM303_CLICK_CFG    0x38
#define LSM303_CLICK_SRC    0x39
#define LSM303_CLICK_THS    0x3A
#define LSM303_TIME_LIMIT   0x3B
#define LSM303_TIME_LATENCY 0x3C
#define LSM303_TIME_WINDOW  0x3D

#define LSM303_ACT_THS      0x3E
#define LSM303_ACT_DUR      0x3F

//bits
#define LSM303_CTRL0_BOOT   0x80

#define LSM303_CTRL1_AXEN   0x01
#define LSM303_CTRL1_AYEN   0x02
#define LSM303_CTRL1_AZEN   0x04
#define LSM303_CTRL1_BDU    0x08

#define LSM303_ACCEL_RATE_POWERDOWN 0x00
#define LSM303_ACCEL_RATE_3_125HZ   0x01
#define LSM303_ACCEL_RATE_6_25HZ    0x02
#define LSM303_ACCEL_RATE_12_5HZ    0x03
#define LSM303_ACCEL_RATE_25HZ      0x04
#define LSM303_ACCEL_RATE_50HZ      0x05
#define LSM303_ACCEL_RATE_100HZ     0x06
#define LSM303_ACCEL_RATE_200HZ     0x07
#define LSM303_ACCEL_RATE_400HZ     0x08
#define LSM303_ACCEL_RATE_800HZ     0x09
#define LSM303_ACCEL_RATE_1600HZ    0x0a
#define LSM303_CTRL1_AODR_BIT 4
#define LSM303_CTRL1_AODR_LEN 4

#define LSM303_ACCEL_FILTER_773HZ 0b00
#define LSM303_ACCEL_FILTER_194HZ 0b01
#define LSM303_ACCEL_FILTER_362HZ 0b10
#define LSM303_ACCEL_FILTER_50HZ  0b11
#define LSM303_CTRL2_ABW_BIT 6
#define LSM303_CTRL2_ABW_LEN 2

#define LSM303_ACCEL_SCALE_2  0x00
#define LSM303_ACCEL_SCALE_4  0x01
#define LSM303_ACCEL_SCALE_6  0x02
#define LSM303_ACCEL_SCALE_8  0x03
#define LSM303_ACCEL_SCALE_16 0x04
#define LSM303_CTRL2_AFS_BIT 3
#define LSM303_CTRL2_AFS_LEN 3

#define LSM303_RESOLUTION_0    0x00
#define LSM303_RESOLUTION_1    0x01
#define LSM303_RESOLUTION_2    0x02
#define LSM303_RESOLUTION_3    0x03
#define LSM303_CTRL5_M_RES_BIT 5
#define LSM303_CTRL5_M_RES_LEN 2

#define LSM303_MAGNET_RATE_3_125HZ 0x00
#define LSM303_MAGNET_RATE_6_25HZ  0x01
#define LSM303_MAGNET_RATE_12_5HZ  0x02
#define LSM303_MAGNET_RATE_25HZ    0x03
#define LSM303_MAGNET_RATE_50HZ    0x04
#define LSM303_MAGNET_RATE_100HZ   0x05
#define LSM303_CTRL5_M_ODR_BIT     2
#define LSM303_CTRL5_M_ODR_LEN     3

#define LSM303_MAGNET_SCALE_2  (0x00 << 5)
#define LSM303_MAGNET_SCALE_4  (0x01 << 5)
#define LSM303_MAGNET_SCALE_8  (0x02 << 5)
#define LSM303_MAGNET_SCALE_12 (0x03 << 5)

#define LSM303_MAGNET_CONTINOUS 0x00
#define LSM303_MAGNET_SINGLE    0x01
#define LSM303_MAGNET_POWERDOWN 0x02
#define LSM303_CTRL7_MD_BIT     0
#define LSM303_CTRL7_MD_LEN     2

extern uint8_t lsm303I2CReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
extern uint8_t lsm303I2CSendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

#pragma pack(1)
typedef struct
{
	int16_t mx, my, mz;
} LSM303_MAGNET_DATA;
typedef struct
{
	int16_t ax, ay, az;
} LSM303_ACCEL_DATA;
#pragma pack()

uint8_t lsm303Presence();
uint8_t lsm303PerformDeviceReset();
uint8_t lsm303IsBeingDeviceReset(uint8_t* res);

uint8_t lsm303EnableBDU();
uint8_t lsm303DisableBDU();
uint8_t lsm303SetMagnetResolution(uint8_t res);
uint8_t lsm303SetMagnetDataRate(uint8_t rate);
uint8_t lsm303SetMagnetScale(uint8_t scale);

uint8_t lsm303SetMagnetContinuousMode();
uint8_t lsm303SetMagnetSingleMode();
uint8_t lsm303SetMagnetPowerDownMode();

uint8_t lsm303SetAccelDataRate(uint8_t rate);
uint8_t lsm303SetAccelBandwidth(uint8_t bandwidth);
uint8_t lsm303SetAccelScale(uint8_t scale);

uint8_t lsm303ReadMagnet(LSM303_MAGNET_DATA* data);
uint8_t lsm303ReadAccel(LSM303_ACCEL_DATA* data);

#endif
