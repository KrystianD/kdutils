#ifndef __ADVINPUTMANAGER_AVR__
#define __ADVINPUTMANAGER_AVR__

#include <public.h>

struct TInputAddr
{
	uint8_t portAddress;
	uint8_t pinNumber;
};

static uint8_t ADVIM_getPinState (struct TInputAddr* addr)
{
	return __IO_IS_HIGH(addr->portAddress, addr->pinNumber);
}

#define ADVIM_SETPORTADDR(s,x) _ADVIM_SETPORTADDR(s,x)
#define _ADVIM_SETPORTADDR(s,port,pin) { s.addr.portAddress = _SFR_IO_ADDR(port); s.addr.pinNumber = pin; }

#endif
