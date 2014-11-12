/*
 * spi.cpp
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "spi.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

using namespace std;

bool SPI::open(const string& path)
{
	fd = ::open(path.c_str(), O_RDWR);
	if (fd == -1)
	{
		perror("i2cOpen");
		return false;
	}
	return true;
}
void SPI::close()
{
	::close(fd);
}

bool SPI::writeData(const void* data, int len)
{
	return rwData(data, len, 0, 0);
}
bool SPI::readData(void* data, int len)
{
	return rwData(0, 0, data, len);
}

bool SPI::rwData(const void* txData, int txLen, void* rxData, int rxLen)
{
	struct spi_ioc_transfer	xfer[2];
	int status;
	
	memset(xfer, 0, sizeof(xfer));
	
	int freq = 10 * 1000 * 1000;
	
	xfer[0].speed_hz = freq;
	xfer[0].tx_buf = (unsigned long)txData;
	xfer[0].rx_buf = 0;
	xfer[0].len = txLen;
	
	xfer[1].speed_hz = freq;
	xfer[1].tx_buf = 0;
	xfer[1].rx_buf = (unsigned long)rxData;
	xfer[1].len = rxLen;
	
	status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (status < 0)
	{
		perror("SPI_IOC_MESSAGE");
		return false;
	}
	return true;
}
