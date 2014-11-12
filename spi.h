/*
 * spi.h
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <string>

class SPI
{
public:
	bool open(const std::string& path);
	void close();
	
	bool writeData(const void* data, int len);
	bool readData(void* data, int len);
	
	bool rwData(const void* txData, int txLen, void* rxData, int rxLen);
	
private:
	int fd;
};

#endif
