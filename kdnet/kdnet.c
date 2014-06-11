#include "kdnet.h"
#include "kdnet_driver.h"
#include <settings.h>

#ifndef KDNET_DEBUG
#define KDNET_DEBUG(x,...)
#endif

uint32_t kdnet_sendId, kdnet_recvId;
uint32_t kdnet_sendTime, kdnet_packetSendTime, kdnet_lastRead = 0;
uint32_t kdnet_preambleTime;
uint32_t kdnet_sendDelayTime;
uint8_t kdnet_channelFree = 1;
uint8_t enableStatus = 0;

//sync
uint8_t kdnet_syncReceived = 0;
uint32_t kdnet_syncReceivedTime;

uint8_t kdnet_state;
TKDNETHeader kdnet_sendHeader;
uint8_t kdnet_sendLen;
uint8_t *kdnet_sendData;
uint8_t kdnet_retries, kdnet_syncs = 0;

TKDNETHeader kdnet_recvHeader;
uint8_t kdnet_recvLen;
uint8_t kdnet_recvData[64];
uint8_t kdnet_recvAvail;

//stats
uint32_t kdnet_syncNoPayload = 0;

void* memcpy(void* s1, const void* s2, unsigned int n)
{
	char *dst = (char*)s1;
	const char *src = (const char*)s2;
	while (n-- != 0)
		*dst++ = *src++;
	return s1;
}

uint8_t kdnet_readPacket();
uint8_t kdnet_writePacket();
uint8_t kdnet_writeACK(uint32_t idToAck);
uint8_t kdnet_startListening();
uint8_t kdnet_startListeningACK();

uint8_t kdnetInit()
{
	kdnet_sendId = 0;
	kdnet_recvId = 0;
	kdnet_recvAvail = 0;
	kdnet_state = KDNET_STATE_NONE;
	kdnet_sendDelayTime = 0;
	
	if (kdnet_startListening())
		return KDNET_ERROR;
		
	return KDNET_SUCCESS;
}


void kdnet_setChannelFree()
{
	kdnet_channelFree = 1;
	kdnet_sendDelayTime = getTicks() + randMinMaxInt(5, 15);
	// myprintf("ch free\r\n");
}
void kdnet_setChannelBusy()
{
	kdnet_channelFree = 0;
	// myprintf("ch busy\r\n");
}

uint8_t kdnetProcess()
{
	switch (kdnet_state)
	{
	case KDNET_STATE_NONE:
		break;
	case KDNET_STATE_SENDING:
		if (getTicks() - kdnet_sendTime > (KDNET_TYPICAL_SEND_TIME) * 2)
		{
			kdnet_driver_setIdleMode();
			KDNET_DEBUG("wait...\r\n");
			kdnet_retries++;
			kdnet_writePacket();
		}
		break;
	// case KDNET_STATE_SENDING_ACK:
	// if (getTicks() - kdnet_sendTime > 10)
	// {
	// KDNET_DEBUG ("Unable to send ACK");
	// kdnet_startListening ();
	// }
	// break;
	case KDNET_STATE_RECEIVING:
		if (kdnet_syncReceived && getTicks() - kdnet_syncReceivedTime >= (KDNET_TYPICAL_SEND_TIME) * 2)
		{
			kdnet_syncNoPayload++;
			kdnet_driver_setIdleMode();
			kdnet_startListening();
		}
		break;
	}
	
	kdnet_driver_process();
}

uint8_t kdnet_readPacket()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	uint16_t payloadLen;
	kdnet_driver_readPayload(data, &payloadLen);
	
	kdnet_recvAvail = 0;
	
	kdnet_recvLen = payloadLen - sizeof(TKDNETHeader) - 2;
	
	memcpy(&kdnet_recvHeader, data, sizeof(TKDNETHeader));
	memcpy(kdnet_recvData, data + sizeof(TKDNETHeader), payloadLen - sizeof(TKDNETHeader) - 2);
	// memset (kdnet_recvData, 0xcc, PKLEN - sizeof(TKDNETHeader) - 2);
	
	kdnet_lastRead = getTicks();
	
	return 0;
}
uint8_t kdnet_writePacket()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	
	memcpy(data, &kdnet_sendHeader, sizeof(TKDNETHeader));
	memcpy(data + sizeof(TKDNETHeader), kdnet_sendData, kdnet_sendLen);
	
	kdnet_driver_writePayload(data, KDNET_MAX_PACKET_LEN);
	kdnet_driver_setTxMode();
	
	kdnet_sendTime = getTicks();
	
	KDNET_DEBUG("Set module to send datagram from 0x%02x to 0x%02x id %d of length: %d",
	            kdnet_sendHeader.addrFrom, kdnet_sendHeader.addrTo, kdnet_sendHeader.id, kdnet_sendLen);
	kdnet_state = KDNET_STATE_SENDING;
	
	return KDNET_SUCCESS;
}
uint8_t kdnet_writeACK(uint32_t idToAck)
{
	// int i;
	// _delay_ms(10);
	// kdnet_writeReg (RFM22_OPER2, RFM22_FFCLRRX | RFM22_FFCLRTX); // reset fifo
	// kdnet_writeReg (RFM22_OPER2, 0x00);
	// kdnet_writeReg (RFM22_TRANSMIT_PACKET_LENGTH, sizeof(TKDNETHeader) + 4 + 2);
	
	// // kdnet_readReg (RFM22_INTR_STATUS1);
	// // kdnet_readReg (RFM22_INTR_STATUS2);
	
	// TKDNETHeader h;
	// h.addrFrom = kdnetAddr;
	// h.addrTo = kdnet_recvHeader.addrFrom;
	// h.id = 0;
	// h.type = 1;
	
	// rfm22SendCommand (0x80 | RFM22_FIFO, (uint8_t*)&h, sizeof(TKDNETHeader));
	// rfm22SendCommand (0x80 | RFM22_FIFO, (uint8_t*)&idToAck, sizeof(idToAck));
	
	// uint16_t crc = 0;
	// crc = kdnetCRC16Update (crc, (uint8_t*)&h, sizeof(TKDNETHeader));
	// crc = kdnetCRC16Update (crc, (uint8_t*)&idToAck, sizeof(idToAck));
	// rfm22SendCommand (0x80 | RFM22_FIFO, (uint8_t*)&crc, sizeof(crc));
	
	// kdnet_writeReg (RFM22_OPER1, RFM22_TXON | RFM22_PLLON | RFM22_XTON);
	// kdnet_sendTime = getTicks();
	
	// KDNET_DEBUG ("Set module to send ACK");
	// kdnet_state = KDNET_STATE_SENDING_ACK;
}
uint8_t kdnet_startListening()
{
	kdnet_driver_setRxMode();
	
	kdnet_setChannelFree();
	kdnet_syncReceived = 0;
	kdnet_state = KDNET_STATE_RECEIVING;
	KDNET_DEBUG("RX mode");
	
	return KDNET_SUCCESS;
}
uint8_t kdnet_startListeningACK()
{
	kdnet_driver_setRxMode();
	
	kdnet_state = KDNET_STATE_RECEIVING_ACK;
	KDNET_DEBUG("RX mode for ACK");
	
	return KDNET_SUCCESS;
}

uint8_t kdnetSendTo(uint8_t addrFrom, uint8_t addrTo, uint8_t* data, uint8_t len)
{
	kdnet_sendHeader.addrFrom = addrFrom;
	kdnet_sendHeader.addrTo = addrTo;
	kdnet_sendHeader.id = kdnet_sendId++;
	kdnet_sendHeader.type = 2;
	kdnet_sendData = data;
	kdnet_sendLen = len;
	kdnet_retries = 0;
	kdnet_state = KDNET_STATE_TOSEND;
	
	kdnet_writePacket();
	
	return KDNET_SUCCESS;
}

uint8_t kdnetIsSending()
{
	return
	  kdnet_state != KDNET_STATE_NONE
	  && kdnet_state != KDNET_STATE_RECEIVING
	  && kdnet_state != KDNET_STATE_RECEIVING_PRE
	  ;
}
uint8_t kdnetWaitToSend()
{
	while (kdnetIsSending())
		kdnetProcess();
}
uint8_t kdnetIsChannelClear()
{
	return kdnet_channelFree && getTicks() >= kdnet_sendDelayTime;
}
uint8_t kdnetIsAvail()
{
	return kdnet_recvAvail != 0;
}
uint8_t kdnetClear()
{
	kdnet_recvAvail = 0;
}

// uint16_t kdnetCRC16Update (uint16_t crc, uint8_t* data, uint8_t len)
// {
// int i;

// while (len--)
// {
// uint8_t a = *data++;
// crc ^= a;
// for (i = 0; i < 8; ++i)
// {
// if (crc & 1)
// crc = (crc >> 1) ^ 0xA001;
// else
// crc = (crc >> 1);
// }
// }

// return crc;
// }
//
//
void kdnet_cb_onChannelBusy()
{
	kdnet_syncReceived = 1;
	kdnet_syncReceivedTime = getTicks();
	// kdnet_syncs++;
	// myprintf ("sync!...\r\n");
	KDNET_DEBUG("busy");
	kdnet_setChannelBusy();
}
void kdnet_cb_onPacketSent()
{
	switch (kdnet_state)
	{
	case KDNET_STATE_SENDING:
		kdnet_packetSendTime = getTicks();
		if (kdnet_sendHeader.type == 0)
		{
			kdnet_startListeningACK();
			KDNET_DEBUG("Sent successfuly, waiting for ACK...");
		}
		else
		{
			kdnet_startListening();
			KDNET_DEBUG("Sent successfuly");
			// myprintf ("time: %d\r\n", getTicks() - kdnet_sendTime);
		}
		break;
	case KDNET_STATE_SENDING_ACK:
		// KDNET_DEBUG ("Waiting for ack send... %02x", val);
		kdnet_startListening();
		KDNET_DEBUG("ACK sent successfuly");
		break;
	}
}
void kdnet_cb_onPacketReceived()
{
	uint8_t res = kdnet_readPacket();
	
	if (res == 0)
	{
		switch (kdnet_state)
		{
		case KDNET_STATE_RECEIVING:
		case KDNET_STATE_RECEIVING_PRE:
			if (kdnet_recvAvail == 0)
			{
				// KDNET_DEBUG ("Valid packet received from: 0x%08x to: 0x%08x id: %d type: %d", kdnet_recvHeader.addrFrom, kdnet_recvHeader.addrTo, kdnet_recvHeader.id, kdnet_recvHeader.type);
				if (kdnet_recvHeader.type == 0)
				{
					if (kdnet_recvHeader.id > kdnet_recvId) // new packet arrived
					{
						kdnet_recvAvail = 1;
						kdnet_recvId = kdnet_recvHeader.id;
						kdnet_writeACK(kdnet_recvHeader.id);
					}
					else // old packet arrived but we'll ack it
					{
						kdnet_writeACK(kdnet_recvHeader.id);
					}
				}
				else if (kdnet_recvHeader.type == 2)
				{
					kdnet_recvAvail = 1;
					kdnet_startListening();
				}
				else
				{
					KDNET_DEBUG("Why did we receive ACK here?");
					kdnet_startListening();
				}
			}
			break;
		case KDNET_STATE_RECEIVING_ACK:
		case KDNET_STATE_RECEIVING_ACK_PRE:
			// myprintf ("pA %d\r\n", kdnet_recvHeader.id);
			KDNET_DEBUG("Valid ACK packet received from: 0x%08x to: 0x%08x id: %d type: %d",
			            kdnet_recvHeader.addrFrom, kdnet_recvHeader.addrTo, kdnet_recvHeader.id, kdnet_recvHeader.type);
			if (kdnet_recvHeader.type == 1)
			{
				KDNET_DEBUG("NICE");
				kdnet_startListening();
			}
			else
			{
				KDNET_DEBUG("Why did we receive non-ACK here?");
				kdnet_startListening();
			}
			break;
		}
	}
	else if (res == 2)
	{
		switch (kdnet_state)
		{
		case KDNET_STATE_RECEIVING:
		case KDNET_STATE_RECEIVING_PRE:
			kdnet_startListening();
			break;
		case KDNET_STATE_RECEIVING_ACK:
		case KDNET_STATE_RECEIVING_ACK_PRE:
			kdnet_startListeningACK();
			break;
		}
	}
	else
	{
		// myprintf("res=1\r\n");
	}
}
