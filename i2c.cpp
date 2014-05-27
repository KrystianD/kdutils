#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

int fd;

void i2cOpen(const char* path)
{
	fd = open(path, O_RDWR);
	if (fd < 0)
	{
		perror("i2cOpen");
		exit(1);
	}
}
void i2cClose()
{
	close(fd);
}
void i2cSetAddress(int address)
{
	if (ioctl(fd, I2C_SLAVE, address) < 0)
	{
		perror("i2cSetAddress");
		exit(1);
	}
}

int i2cWrite(const uint8_t* data, int len)
{
	return write(fd, data, len);
}
int i2cRead(uint8_t* data, int len)
{
	return read(fd, data, len);
}
void i2cWriteReg(uint8_t reg, uint8_t val)
{
	uint8_t o[2];
	o[0] = reg;
	o[1] = val;
	write(fd, o, 2);
}
void i2cWriteReg(uint8_t reg, const uint8_t* data, int len)
{
	uint8_t o[1 + len];
	o[0] = reg;
	memcpy(o + 1, data, len);
	write(fd, o, sizeof(o));
}
uint8_t i2cReadReg(uint8_t reg)
{
	uint8_t o[1];
	o[0] = reg;
	write(fd, o, 1);
	read(fd, o, 1);
	return o[0];
}
void i2cReadReg(uint8_t reg, uint8_t* data, int len)
{
	write(fd, &reg, 1);
	read(fd, data, len);
}
