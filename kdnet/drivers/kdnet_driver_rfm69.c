#include "kdnet_driver.h"

#include <RFM69.h>
#include <RFM69_defs.h>
#include <settings.h>

#ifndef KDNET_DEBUG
#define KDNET_DEBUG(x,...)
#endif

uint16_t kdnet_driver_getMaxPayloadLength()
{
	return 64;
}
void kdnet_driver_setIdleMode()
{
	rfm69SwitchToStandby();
}
void kdnet_driver_setTxMode()
{
	rfm69SwitchToTx();
}
void kdnet_driver_setRxMode()
{
	rfm69SwitchToRx();
}
void kdnet_driver_writePayload(const uint8_t* data, uint16_t len)
{
	rfm69WritePayload(data, len);
}
void kdnet_driver_readPayload(uint8_t* data, uint16_t* len)
{
	*len = kdnet_driver_getMaxPayloadLength();
	
	rfm69ReadPayload(data, *len);
}
void kdnet_driver_processInterrupt()
{
}
void kdnet_driver_process()
{
	uint8_t st1, st2;
	
	st1 = rfm69ReadRegister(RFM69_IRQFLAGS1);
	st2 = rfm69ReadRegister(RFM69_IRQFLAGS2);
	
	if (st1 & RFM69_IRQFLAGS1_SYNCADDRESSMATCH)
	{
		kdnet_cb_onChannelBusy();
	}
	
	// if (st1 & RFM69_IRQFLAGS1_TIMEOUT)
	// {
	// if (!(st1 & RFM69_IRQFLAGS1_SYNCADDRESSMATCH))
	// {
	// KDNET_DEBUG ("restart");
	// kdnet_driver_setIdleModkdnet_driver_setTxModee ();
	// kdnet_startListening ();
	// }
	// }
	
	// if (enableStatus)
	// {
	// rfm69PrintStatus ();
	// if (!kdnet_channelFree)
	// myprintf ("NOT FREE\r\n");
	// _delay_ms (200);
	// }
	
	if (st2 & RFM69_IRQFLAGS2_PACKETSENT)
	{
		kdnet_cb_onPacketSent();
	}
	
	if (st2 & RFM69_IRQFLAGS2_PAYLOADREADY)
	{
		kdnet_cb_onPacketReceived();
	}
	
}
