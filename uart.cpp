#include "uart.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

SerialHandle uart_open(const char* path, int speed)
{
	struct termios options;
	
	int fd = open(path, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
	
	if (fd < 0)
		return -1;
		
	tcgetattr(fd, &options);
	
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);
	
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS | HUPCL);
	options.c_cflag |= (CS8 | 0 | CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ISIG | ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL | ECHOPRT | ECHOKE | IEXTEN);
	options.c_iflag &= ~(INPCK | IXON | IXOFF | IXANY | ICRNL);
	options.c_oflag &= ~(OPOST | ONLCR);
	
	cfmakeraw(&options);

	for (uint i = 0; i < sizeof(options.c_cc); i++)
		options.c_cc[i] = _POSIX_VDISABLE;
		
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
	
	return fd;
}
int uart_pending(SerialHandle handle)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(handle, &fds);
	timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 0;
	int r = select(handle + 1, &fds, 0, 0, &tv);
	return r == 1;
}
int uart_tx(SerialHandle handle, char *data, int len)
{
	struct termios options;
	int l = len;
	
	tcgetattr(handle, &options);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;
	tcsetattr(handle, TCSANOW, &options);
	
	while (len)
	{
		int written = write(handle, data, len);
		if (written < 0)
			return -1;
		len -= written;
		data += written;
	}
	return 0;
}
int uart_rx(SerialHandle handle, char *data, int len, int timeout_ms)
{
	struct termios options;
	int l = len;
	
	// enable timeout
	tcgetattr(handle, &options);
	options.c_cc[VTIME] = timeout_ms / 100;
	options.c_cc[VMIN] = 0;
	tcsetattr(handle, TCSANOW, &options);
	
	while (len)
	{
		int rread = read(handle, data, len);
		if (rread == 0)
			return l - len;
		if (rread < 0)
			return -1;
			
		len -= rread;
		data += rread;
	}
	return l;
}
int uart_rx_raw(SerialHandle handle, char *data, int len)
{
	struct termios options;
	
	// disable timeout
	tcgetattr(handle, &options);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;
	tcsetattr(handle, TCSANOW, &options);
	
	int rread = read(handle, data, len);
	
	return rread;
}
void uart_close(SerialHandle handle)
{
	close(handle);
}
