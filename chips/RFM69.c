#include <RFM69.h>
#include <RFM69_defs.h>
#include <settings.h>

#ifndef RFM69_DEBUG
#define RFM69_DEBUG(x,...)
#endif
#ifndef RFM69_DEBUGNNL
#define RFM69_DEBUGNNL(x,...)
#endif

#define ER(x) if (x) return RFM69_ERROR;

uint8_t rfm69_setHighPower(uint8_t on);

static uint8_t rfWriteRegData(uint8_t addr, const uint8_t* data, uint8_t len)
{
	return rfm69SPISendCommand(addr, data, len);
}

uint8_t rfm69SetFrequency(uint32_t freq)
{
	freq /= RFM69_FSTEP;
	// RFM69_DEBUG ("freq 0x%04x\r\n", freq);
	ER(rfm69WriteRegister(RFM69_FRFMSB, (freq >> 16) & 0xff));
	ER(rfm69WriteRegister(RFM69_FRFMID, (freq >> 8) & 0xff));
	ER(rfm69WriteRegister(RFM69_FRFLSB, (freq >> 0) & 0xff));
	return 0;
}
uint8_t rfm69SetFrequencyByte(uint32_t freq)
{
	ER(rfm69WriteRegister(RFM69_FRFMSB, freq >> 16));
	ER(rfm69WriteRegister(RFM69_FRFMID, freq >> 8));
	ER(rfm69WriteRegister(RFM69_FRFLSB, freq >> 0));
	return 0;
}
uint8_t rfm69SetBitRate(uint32_t bitrate)
{
	bitrate = (2 * RFM69_FXOSC / bitrate + 1) >> 1;
	// RFM69_DEBUG ("bitrate 0x%04x\r\n", bitrate);
	
	ER(rfm69WriteRegister(RFM69_BITRATEMSB, (bitrate >> 8) & 0xff));
	ER(rfm69WriteRegister(RFM69_BITRATELSB, (bitrate >> 0) & 0xff));
	return 0;
}
uint8_t rfm69SetSyncWord(uint8_t len, uint8_t* data)
{
	if (len == 0)
	{
		ER(rfm69WriteRegister(RFM69_SYNCCONFIG, RFM69_SYNC_OFF));
	}
	else
	{
		int i;
		ER(rfm69WriteRegister(RFM69_SYNCCONFIG, RFM69_SYNC_ON | ((len - 1) << 3)));
		for (i = 0; i < len; i++)
			ER(rfm69WriteRegister(RFM69_SYNCVALUE1 + i, *data++));
	}
	return 0;
}
uint8_t rfm69SetPreambleSize(uint8_t len)
{
	ER(rfm69WriteRegister(RFM69_PREAMBLEMSB, (len >> 8) & 0xff));
	ER(rfm69WriteRegister(RFM69_PREAMBLELSB, (len >> 0) & 0xff));
	return 0;
}
uint8_t rfm69SetDeviation(uint32_t deviation)
{
	deviation /= RFM69_FSTEP;
	ER(rfm69WriteRegister(RFM69_FDEVMSB, (deviation >> 8) & 0xff));
	ER(rfm69WriteRegister(RFM69_FDEVLSB, (deviation >> 0) & 0xff));
	return 0;
}
uint8_t rfm69SetOutputPower(uint8_t power)
{
	uint8_t val;
	ER(rfm69ReadRegister(RFM69_PALEVEL, &val));
	ER(rfm69WriteRegister(RFM69_PALEVEL, (val & 0xe0) | power));
	return 0;
}
uint8_t rfm69SetRSSIThreshold(int16_t rssi)
{
	ER(rfm69WriteRegister(RFM69_RSSITHRESH, -rssi * 2));
	return 0;
}

uint8_t rfm69SwitchToSleep()
{
	return rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_SLEEP);
}
uint8_t rfm69SwitchToStandby()
{
	return rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_STANDBY);
}
uint8_t rfm69SwitchToTx()
{
	ER(rfm69_setHighPower(1));
	ER(rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_TRANSMITTER));
	return RFM69_SUCCESS;
}
uint8_t rfm69SwitchToRx()
{
	ER(rfm69_setHighPower(0));
	ER(rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_RECEIVER));
	return RFM69_SUCCESS;
}
uint8_t rfm69IsReady(uint8_t* ready)
{
	uint8_t val;
	ER(rfm69ReadRegister(RFM69_IRQFLAGS1, &val));
	*ready = val & RFM69_IRQFLAGS1_MODEREADY;
	return RFM69_SUCCESS;
}

uint8_t rfm69ReadPayload(uint8_t* data, uint8_t len)
{
	return rfm69SPIReadCommand(RFM69_FIFO, data, len);
}
uint8_t rfm69ReadPayloadVarLen(uint8_t* data, uint8_t* len)
{
	ER(rfm69SPIReadCommand(RFM69_FIFO, len, sizeof(*len)));
	//RFM69_DEBUG("pl: %d", *len);
	return rfm69SPIReadCommand(RFM69_FIFO, data, *len);
}
uint8_t rfm69WritePayload(const uint8_t* data, uint8_t len)
{
	return rfm69SPISendCommand(RFM69_FIFO, data, len);
}

int16_t rfm69GetRSSI()
{
	uint8_t val;
	rfm69ReadRegister(RFM69_RSSIVALUE, &val);
	return -val / 2;
}

void rfm69PrintStatus()
{
	uint8_t op, irq1, irq2;
	rfm69ReadRegister(RFM69_OPMODE, &op);
	rfm69ReadRegister(RFM69_IRQFLAGS1, &irq1);
	rfm69ReadRegister(RFM69_IRQFLAGS2, &irq2);
	
	RFM69_DEBUG("op: 0x%02x: ", op);
	
	switch (op & (0x07 << 2))
	{
	case RFM69_OPMODE_SLEEP:
		RFM69_DEBUG("Sleep");
		break;
	case RFM69_OPMODE_STANDBY:
		RFM69_DEBUG("Standby");
		break;
	case RFM69_OPMODE_SYNTHESIZER:
		RFM69_DEBUG("Fs");
		break;
	case RFM69_OPMODE_TRANSMITTER:
		RFM69_DEBUG("Tx");
		break;
	case RFM69_OPMODE_RECEIVER:
		RFM69_DEBUG("Rx");
		break;
	}
	
	RFM69_DEBUGNNL("     RSSI: %d\r\n", rfm69GetRSSI());
	
	RFM69_DEBUGNNL("irq1: 0x%02x: ", irq1);
	
	if (irq1 & RFM69_IRQFLAGS1_MODEREADY) RFM69_DEBUGNNL("ModeReady, ");
	if (irq1 & RFM69_IRQFLAGS1_RXREADY) RFM69_DEBUGNNL("RxReady, ");
	if (irq1 & RFM69_IRQFLAGS1_TXREADY) RFM69_DEBUGNNL("TxReady, ");
	if (irq1 & RFM69_IRQFLAGS1_PLLLOCK) RFM69_DEBUGNNL("PllLock, ");
	if (irq1 & RFM69_IRQFLAGS1_RSSI) RFM69_DEBUGNNL("Rssi, ");
	if (irq1 & RFM69_IRQFLAGS1_TIMEOUT) RFM69_DEBUGNNL("Timeout, ");
	if (irq1 & RFM69_IRQFLAGS1_AUTOMODE) RFM69_DEBUGNNL("AutoMode, ");
	if (irq1 & RFM69_IRQFLAGS1_SYNCADDRESSMATCH) RFM69_DEBUGNNL("SyncAddrMatch, ");
	
	RFM69_DEBUGNNL("\r\nirq2: 0x%02x: ", irq2);
	
	if (irq2 & RFM69_IRQFLAGS2_FIFOFULL) RFM69_DEBUGNNL("FifoFull, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFONOTEMPTY) RFM69_DEBUGNNL("FifoNotEmpty, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFOLEVEL) RFM69_DEBUGNNL("FifoLevel, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFOOVERRUN) RFM69_DEBUGNNL("FifoOverrun, ");
	if (irq2 & RFM69_IRQFLAGS2_PACKETSENT) RFM69_DEBUGNNL("PacketSent, ");
	if (irq2 & RFM69_IRQFLAGS2_PAYLOADREADY) RFM69_DEBUGNNL("PayloadReady, ");
	if (irq2 & RFM69_IRQFLAGS2_CRCOK) RFM69_DEBUGNNL("CrcOk, ");
	
	RFM69_DEBUGNNL("\r\n");
}

uint8_t rfm69WriteRegister(uint8_t addr, uint8_t value)
{
	return rfm69SPISendCommand(addr, &value, 1);
}
uint8_t rfm69ReadRegister(uint8_t addr, uint8_t* value)
{
	return rfm69SPIReadCommand(addr, value, 1);
}

uint8_t rfm69_setHighPower(uint8_t on)
{
	ER(rfm69WriteRegister(RFM69_OCP, on ? 0x0f : 0x10 | 0b1010));  // default
	ER(rfm69WriteRegister(RFM69_TESTPA1, on ? 0x5d : 0x55));
	ER(rfm69WriteRegister(RFM69_TESTPA2, on ? 0x7c : 0x70));
}
