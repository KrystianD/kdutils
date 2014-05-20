#ifndef __HMC5883_H__
#define __HMC5883_H__

#include <stdint.h>

#define HMC5883_SUCCESS     0
#define HMC5883_ERROR       1
#define HMC5883_VERIFYERROR 2
#define HMC5883_NOTREADY    3

#define HMC5883_ADDR       0x1e

#define HMC5883_CONFA 0x00
#    define HMC5883_CONFA_SAMPLES_1 (0b00 << 5)
#    define HMC5883_CONFA_SAMPLES_2 (0b01 << 5)
#    define HMC5883_CONFA_SAMPLES_4 (0b10 << 5)
#    define HMC5883_CONFA_SAMPLES_8 (0b11 << 5)
#    define HMC5883_CONFA_RATE_0_75 (0b000 << 2)
#    define HMC5883_CONFA_RATE_1_5  (0b001 << 2)
#    define HMC5883_CONFA_RATE_3    (0b010 << 2)
#    define HMC5883_CONFA_RATE_7_5  (0b011 << 2)
#    define HMC5883_CONFA_RATE_15   (0b100 << 2)
#    define HMC5883_CONFA_RATE_30   (0b101 << 2)
#    define HMC5883_CONFA_RATE_75   (0b000 << 2)
#    define HMC5883_CONFA_MODE_NORMAL  (0b00)
#    define HMC5883_CONFA_MODE_POSBIAS (0b01)
#    define HMC5883_CONFA_MODE_NEGBIAS (0b10)
#define HMC5883_CONFB 0x01
#    define HMC5883_CONFB_GAIN_1370 (0b000 << 5)
#    define HMC5883_CONFB_GAIN_1090 (0b001 << 5)
#    define HMC5883_CONFB_GAIN_820  (0b010 << 5)
#    define HMC5883_CONFB_GAIN_660  (0b011 << 5)
#    define HMC5883_CONFB_GAIN_440  (0b100 << 5)
#    define HMC5883_CONFB_GAIN_390  (0b101 << 5)
#    define HMC5883_CONFB_GAIN_330  (0b110 << 5)
#    define HMC5883_CONFB_GAIN_230  (0b111 << 5)
#define HMC5883_MODE 0x02
#    define HMC5883_MODE_HS         0x80
#    define HMC5883_MODE_CONTINUOUS 0b00
#    define HMC5883_MODE_SINGLE     0b01
#    define HMC5883_MODE_IDLE       0b10
#define HMC5883_STATUS 0x09
#    define HMC5883_STATUS_LOCK 0x02
#    define HMC5883_STATUS_RDY  0x01
#define HMC5883_DATA_X_MSB 0x03
#define HMC5883_DATA_X_LSB 0x04
#define HMC5883_DATA_Z_MSB 0x05
#define HMC5883_DATA_Z_LSB 0x06
#define HMC5883_DATA_Y_MSB 0x07
#define HMC5883_DATA_Y_LSB 0x08

#pragma pack(1)
typedef struct
{
	int16_t mx, mz, my;
	uint8_t status;
} HMC5883_Data;
#pragma pack()

uint8_t hmc5883I2CReadCommand(uint8_t cmd, uint8_t* data, uint8_t len);
uint8_t hmc5883I2CSendCommand(uint8_t cmd, const uint8_t* data, uint8_t len);

uint8_t hmc5883SetConfig(uint8_t ca, uint8_t cb);
// uint8_t hmc5883SetSingleMode ();
uint8_t hmc5883SetContinousMode();
uint8_t hmc5883GetData(HMC5883_Data* data);

#endif
