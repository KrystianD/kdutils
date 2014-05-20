#ifndef UART_H
#define UART_H

#include <termios.h>

typedef int SerialHandle;

SerialHandle uart_open(const char* path, int speed);
int uart_pending(SerialHandle handle);
int uart_tx(SerialHandle handle, char* data, int len);
int uart_rx(SerialHandle handle, char* data, int len, int timeout_ms);
int uart_rx_raw(SerialHandle handle, char *data, int len);
void uart_close(SerialHandle handle);

#endif // UART_H
