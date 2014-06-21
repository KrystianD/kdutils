#include "kdnet_driver.h"

#include <RFM69.h>
#include <RFM69_defs.h>
#include <settings.h>

#ifndef KDNET_DEBUG
#define KDNET_DEBUG(x,...)
#endif

#define ER(x) if (x) return KDNET_ERROR

uint16_t kdnet_driver_getMaxPayloadLength()
{
	return 64;
}
uint8_t kdnet_driver_setIdleMode()
{
	return rfm69SwitchToStandby();
}
uint8_t kdnet_driver_setTxMode()
{
	return rfm69SwitchToTx();
}
uint8_t kdnet_driver_setRxMode()
{
	return rfm69SwitchToRx();
}
uint8_t kdnet_driver_waitForMode()
{
	uint8_t ready;
	do
	{
		ER(rfm69IsReady(&ready));
	} while (ready == 0);
	return KDNET_SUCCESS;
}
uint8_t kdnet_driver_writePayload(const uint8_t* data, uint16_t len)
{
	return rfm69WritePayload(data, len);
}
uint8_t kdnet_driver_readPayload(uint8_t* data, uint16_t* len)
{
	// *len = kdnet_driver_getMaxPayloadLength();
	uint8_t len2;
	ER(rfm69ReadPayloadVarLen(data, &len2));
	*len = len2;
	return KDNET_SUCCESS;
}
uint8_t kdnet_driver_process()
{
	uint8_t op, st1, st2;

	ER(rfm69ReadRegister(RFM69_OPMODE, &op));
	ER(rfm69ReadRegister(RFM69_IRQFLAGS1, &st1));
	ER(rfm69ReadRegister(RFM69_IRQFLAGS2, &st2));

	if (op & RFM69_OPMODE_TRANSMITTER)
	{
		if (st2 & RFM69_IRQFLAGS2_PACKETSENT)
		{
			ER(kdnet_cb_onPacketSent());
		}
	}
	else if (op & RFM69_OPMODE_RECEIVER)
	{
		if (st2 & RFM69_IRQFLAGS2_PAYLOADREADY)
		{
			if (st2 & RFM69_IRQFLAGS2_CRCOK)
			{
				ER(kdnet_cb_onPacketReceived());
			}
			else
			{
				kdnet_driver_setIdleMode();
				kdnet_driver_setRxMode();
				KDNET_DEBUG("CRC ERROR!");
			}
			ER(kdnet_cb_onChannelFree());
		}
		else
		{
			if (st1 & RFM69_IRQFLAGS1_SYNCADDRESSMATCH)
			{
				ER(kdnet_cb_onChannelBusy());
			}
			else
			{
				ER(kdnet_cb_onChannelFree());
			}
		}
	}
	
	return KDNET_SUCCESS;
}
uint8_t kdnet_driver_processInterruptPacketSent()
{
	uint8_t st1, st2;

	ER(rfm69ReadRegister(RFM69_IRQFLAGS1, &st1));
	ER(rfm69ReadRegister(RFM69_IRQFLAGS2, &st2));

	if (st2 & RFM69_IRQFLAGS2_PACKETSENT)
	{
		ER(kdnet_cb_onPacketSent());
	}

	return KDNET_SUCCESS;
}
