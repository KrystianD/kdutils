#include <RFM69.h>
#include <RFM69_defs.h>
#include <settings.h>

#ifndef RFM69_DEBUG
#define RFM69_DEBUG(x,...)
#endif

void rfm69_setHighPower(uint8_t on);

static void rfWriteRegData(uint8_t addr, const uint8_t* data, uint8_t len)
{
	rfm69SPISendCommand(addr, data, len);
}

uint8_t rfm69SetFrequency(uint32_t freq)
{
	freq /= RFM69_FSTEP;
	// RFM69_DEBUG ("freq 0x%04x\r\n", freq);
	rfm69WriteRegister(RFM69_FRFMSB, (freq >> 16) & 0xff);
	rfm69WriteRegister(RFM69_FRFMID, (freq >> 8) & 0xff);
	rfm69WriteRegister(RFM69_FRFLSB, (freq >> 0) & 0xff);
	return 0;
}
uint8_t rfm69SetFrequencyByte(uint32_t freq)
{
	rfm69WriteRegister(RFM69_FRFMSB, freq >> 16);
	rfm69WriteRegister(RFM69_FRFMID, freq >> 8);
	rfm69WriteRegister(RFM69_FRFLSB, freq >> 0);
	return 0;
}
uint8_t rfm69SetBitRate(uint32_t bitrate)
{
	bitrate = (2 * RFM69_FXOSC / bitrate + 1) >> 1;
	// RFM69_DEBUG ("bitrate 0x%04x\r\n", bitrate);
	
	rfm69WriteRegister(RFM69_BITRATEMSB, (bitrate >> 8) & 0xff);
	rfm69WriteRegister(RFM69_BITRATELSB, (bitrate >> 0) & 0xff);
	return 0;
}
uint8_t rfm69SetSyncWord(uint8_t len, uint8_t* data)
{
	if (len == 0)
	{
		rfm69WriteRegister(RFM69_SYNCCONFIG, RFM69_SYNC_OFF);
	}
	else
	{
		int i;
		rfm69WriteRegister(RFM69_SYNCCONFIG, RFM69_SYNC_ON | ((len - 1) << 3));
		for (i = 0; i < len; i++)
			rfm69WriteRegister(RFM69_SYNCVALUE1 + i, *data++);
	}
	return 0;
}
uint8_t rfm69SetPreambleSize(uint8_t len)
{
	rfm69WriteRegister(RFM69_PREAMBLEMSB, (len >> 8) & 0xff);
	rfm69WriteRegister(RFM69_PREAMBLELSB, (len >> 0) & 0xff);
	return 0;
}
uint8_t rfm69SetDeviation(uint32_t deviation)
{
	deviation /= RFM69_FSTEP;
	rfm69WriteRegister(RFM69_FDEVMSB, (deviation >> 8) & 0xff);
	rfm69WriteRegister(RFM69_FDEVLSB, (deviation >> 0) & 0xff);
	return 0;
}
uint8_t rfm69SetOutputPower(uint8_t power)
{
	uint8_t val = rfm69ReadRegister(RFM69_PALEVEL);
	rfm69WriteRegister(RFM69_PALEVEL, (val & 0xe0) | power);
	return 0;
}
uint8_t rfm69SetRSSIThreshold(int16_t rssi)
{
	rfm69WriteRegister(RFM69_RSSITHRESH, -rssi * 2);
	return 0;
}

void rfm69SwitchToSleep()
{
	rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_SLEEP);
}
void rfm69SwitchToStandby()
{
	rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_STANDBY);
}
void rfm69SwitchToTx()
{
	rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_TRANSMITTER);
	rfm69_setHighPower(1);
}
void rfm69SwitchToRx()
{
	rfm69WriteRegister(RFM69_OPMODE, RFM69_OPMODE_RECEIVER);
	rfm69_setHighPower(0);
}
uint8_t rfm69IsReady()
{
	return rfm69ReadRegister(RFM69_IRQFLAGS1) & RFM69_IRQFLAGS1_MODEREADY;
}

uint8_t rfm69ReadPayload(uint8_t* data, uint8_t len)
{
	rfm69SPIReadCommand(RFM69_FIFO, data, len);
}
uint8_t rfm69WritePayload(const uint8_t* data, uint8_t len)
{
	rfm69SPISendCommand(RFM69_FIFO, data, len);
}



int16_t rfm69GetRSSI()
{
	return -rfm69ReadRegister(RFM69_RSSIVALUE) / 2;
}

void rfm69PrintStatus()
{
	uint8_t op = rfm69ReadRegister(RFM69_OPMODE);
	uint8_t irq1 = rfm69ReadRegister(RFM69_IRQFLAGS1);
	uint8_t irq2 = rfm69ReadRegister(RFM69_IRQFLAGS2);
	
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
	
	RFM69_DEBUG("     RSSI: %d\r\n", rfm69GetRSSI());
	
	RFM69_DEBUG("irq1: 0x%02x: ", irq1);
	
	if (irq1 & RFM69_IRQFLAGS1_MODEREADY) RFM69_DEBUG("ModeReady, ");
	if (irq1 & RFM69_IRQFLAGS1_RXREADY) RFM69_DEBUG("RxReady, ");
	if (irq1 & RFM69_IRQFLAGS1_TXREADY) RFM69_DEBUG("TxReady, ");
	if (irq1 & RFM69_IRQFLAGS1_PLLLOCK) RFM69_DEBUG("PllLock, ");
	if (irq1 & RFM69_IRQFLAGS1_RSSI) RFM69_DEBUG("Rssi, ");
	if (irq1 & RFM69_IRQFLAGS1_TIMEOUT) RFM69_DEBUG("Timeout, ");
	if (irq1 & RFM69_IRQFLAGS1_AUTOMODE) RFM69_DEBUG("AutoMode, ");
	if (irq1 & RFM69_IRQFLAGS1_SYNCADDRESSMATCH) RFM69_DEBUG("SyncAddrMatch, ");
	
	RFM69_DEBUG("\r\nirq2: 0x%02x: ", irq2);
	
	if (irq2 & RFM69_IRQFLAGS2_FIFOFULL) RFM69_DEBUG("FifoFull, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFONOTEMPTY) RFM69_DEBUG("FifoNotEmpty, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFOLEVEL) RFM69_DEBUG("FifoLevel, ");
	if (irq2 & RFM69_IRQFLAGS2_FIFOOVERRUN) RFM69_DEBUG("FifoOverrun, ");
	if (irq2 & RFM69_IRQFLAGS2_PACKETSENT) RFM69_DEBUG("PacketSent, ");
	if (irq2 & RFM69_IRQFLAGS2_PAYLOADREADY) RFM69_DEBUG("PayloadReady, ");
	if (irq2 & RFM69_IRQFLAGS2_CRCOK) RFM69_DEBUG("CrcOk, ");
	
	RFM69_DEBUG("\r\n");
}

void rfm69WriteRegister(uint8_t addr, uint8_t value)
{
	rfm69SPISendCommand(addr, &value, 1);
}
uint8_t rfm69ReadRegister(uint8_t addr)
{
	uint8_t data;
	rfm69SPIReadCommand(addr, &data, 1);
	return data;
}

void rfm69_setHighPower(uint8_t on)
{
	rfm69WriteRegister(RFM69_OCP, on ? 0x0f : 0x10 | 0b1010);  // default
	rfm69WriteRegister(RFM69_TESTPA1, on ? 0x5d : 0x55);
	rfm69WriteRegister(RFM69_TESTPA2, on ? 0x7c : 0x70);
}
