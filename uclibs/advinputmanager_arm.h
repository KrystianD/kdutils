#ifndef __ADVINPUTMANAGER_ARM__
#define __ADVINPUTMANAGER_ARM__

#include <public.h>

struct TInputAddr
{
	GPIO_TypeDef* portAddress;
	int pinNumber;
};

static uint8_t ADVIM_getPinState (struct TInputAddr* addr)
{
	return IO_IS_HIGH(addr->portAddress, addr->pinNumber);
}

#define ADVIM_SETPORTADDR(s,x) _ADVIM_SETPORTADDR(s,x)
#define _ADVIM_SETPORTADDR(s,port,pin) { s.addr.portAddress = port; s.addr.pinNumber = pin; }

#endif
