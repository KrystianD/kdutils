#include "kdnet.h"
#include "kdnet_driver.h"
#include <settings.h>

#ifndef KDNET_DEBUG
#define KDNET_DEBUG(x,...)
#endif

#define ER(x) if (x) return KDNET_ERROR;

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

//conn
TKDNETConnection* kdnet_connetions[] = KDNET_CONN;
const int kdnet_connSize = sizeof(kdnet_connetions) / sizeof(kdnet_connetions[0]);

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

TKDNETConnection* kdnet_connByHeader(TKDNETHeader* header)
{
	int i;
	for (i = 0; i < kdnet_connSize; i++)
	{
		TKDNETConnection *c = kdnet_connetions[i];
		if (c->addrFrom == header->addrFrom && c->addrTo == header->addrTo)
			return c;
	}
	return 0;
}
TKDNETConnection* kdnet_connByHeaderRev(TKDNETHeader* header)
{
	int i;
	for (i = 0; i < kdnet_connSize; i++)
	{
		TKDNETConnection *c = kdnet_connetions[i];
		if (c->addrFrom == header->addrTo && c->addrTo == header->addrFrom)
			return c;
	}
	return 0;
}


uint8_t kdnet_readPacket();
uint8_t kdnet_writePacket();
uint8_t kdnet_writeACK(TKDNETConnection *conn, uint32_t idToAck);
uint8_t kdnet_startListening();
uint8_t kdnet_startListeningACK();

uint8_t kdnetInit()
{
	kdnet_sendId = 0;
	kdnet_recvId = 0;
	kdnet_recvAvail = 0;
	kdnet_state = KDNET_STATE_NONE;
	kdnet_sendDelayTime = 0;
	
	ER(kdnet_startListening());
	
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

uint8_t kdnet_writeConnPacket(TKDNETConnection* conn);
uint8_t kdnetProcess()
{
	int i;
	switch (kdnet_state)
	{
	case KDNET_STATE_NONE:
	
		break;
	case KDNET_STATE_SENDING:
		if (0&&getTicks() - kdnet_sendTime > KDNET_TYPICAL_SEND_TIME * 2)
		{
			KDNET_DEBUG("Sending timeouted");
			for (;;);
			ER(kdnet_driver_setIdleMode());
			kdnet_retries++;
			ER(kdnet_writePacket());
		}
		break;
	case KDNET_STATE_RECEIVING:
		if (kdnet_syncReceived && getTicks() - kdnet_syncReceivedTime >= KDNET_TYPICAL_SEND_TIME * 2)
		{
			kdnet_syncNoPayload++;
			ER(kdnet_driver_setIdleMode());
			ER(kdnet_startListening());
		}

		for (i = 0; i < kdnet_connSize; i++)
		{
			TKDNETConnection *c = kdnet_connetions[i];
			if (c->state == CONN_TO_SEND)
			{
				kdnet_writeConnPacket(c);
				c->state = CONN_WAIT_ACK;
			}
			else if (c->state == CONN_WAIT_ACK)
			{
				if (getTicks() - c->sendTime > 500)
				{
					kdnet_writeConnPacket(c);
					c->state = CONN_WAIT_ACK;
				}
			}
		}		
		break;
	}	
	
	ER(kdnet_driver_process());
	return KDNET_SUCCESS;
}

uint8_t kdnet_readPacket()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	uint16_t payloadLen;
	ER(kdnet_driver_readPayload(data, &payloadLen));
	
	kdnet_recvAvail = 0;
	
	kdnet_recvLen = payloadLen - sizeof(TKDNETHeader) - 2;
	
	TKDNETHeader *header = (TKDNETHeader*)data;
	TKDNETConnection* conn = kdnet_connByHeaderRev(header);
	if (!conn)
		return KDNET_SUCCESS;

	conn->inDataLen = payloadLen - sizeof(TKDNETHeader) - 2;
	memcpy(conn->inBuf, data + sizeof(TKDNETHeader), conn->inDataLen);
	
	kdnet_lastRead = getTicks();

	return KDNET_SUCCESS;
}
uint8_t kdnet_writeConnPacket(TKDNETConnection* conn)
{
	TKDNETHeader header;
	header.addrFrom = conn->addrFrom;
	header.addrTo = conn->addrTo;
	header.id = conn->id;
	header.type = 0;

	ER(kdnet_driver_setIdleMode());
	ER(kdnet_driver_writePayload((uint8_t*)&header, sizeof(header)));
	ER(kdnet_driver_writePayload(conn->buf, conn->dataLen));
	ER(kdnet_driver_setTxMode());

	conn->sendTime = getTicks();
	
	KDNET_DEBUG("Set module to send packet from 0x%02x to 0x%02x id %d of length: %d",
	            header.addrFrom, header.addrTo, header.id, conn->dataLen);
	kdnet_state = KDNET_STATE_SENDING;
	
	return KDNET_SUCCESS;
}
/*uint8_t kdnet_writePacket()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	
	memcpy(data, &kdnet_sendHeader, sizeof(TKDNETHeader));
	memcpy(data + sizeof(TKDNETHeader), kdnet_sendData, kdnet_sendLen);
	
	ER(kdnet_driver_setIdleMode());
	ER(kdnet_driver_writePayload(data, KDNET_MAX_PACKET_LEN));
	ER(kdnet_driver_setTxMode());
	
	kdnet_sendTime = getTicks();
	
	KDNET_DEBUG("Set module to send datagram from 0x%02x to 0x%02x id %d of length: %d",
	            kdnet_sendHeader.addrFrom, kdnet_sendHeader.addrTo, kdnet_sendHeader.id, kdnet_sendLen);
	kdnet_state = KDNET_STATE_SENDING;
	
	return KDNET_SUCCESS;
}*/
uint8_t kdnet_writeACK(TKDNETConnection* conn, uint32_t idToAck)
{
	uint8_t data[KDNET_MAX_PACKET_LEN];

	TKDNETHeader header;
	header.addrFrom = conn->addrFrom;
	header.addrTo = conn->addrTo;
	header.id = idToAck;
	header.type = KDNET_TYPE_ACK;
	memcpy(data, &header, sizeof(TKDNETHeader));

	ER(kdnet_driver_setIdleMode());
	ER(kdnet_driver_writePayload(data, KDNET_MAX_PACKET_LEN));
	ER(kdnet_driver_setTxMode());

	kdnet_sendTime = getTicks();

	KDNET_DEBUG("Set module to send ACK from 0x%02x to 0x%02x id %d of length: %d",
							header.addrFrom, header.addrTo, header.id, sizeof(data));
	kdnet_state = KDNET_STATE_SENDING_ACK;

	return KDNET_SUCCESS;
}
uint8_t kdnet_startListening()
{
	ER(kdnet_driver_setRxMode());
	
	kdnet_setChannelFree();
	kdnet_syncReceived = 0;
	kdnet_state = KDNET_STATE_RECEIVING;
	KDNET_DEBUG("RX mode");
	
	return KDNET_SUCCESS;
}

/*uint8_t kdnetSendTo(uint8_t addrFrom, uint8_t addrTo, uint8_t* data, uint8_t len)
{
	kdnet_sendHeader.addrFrom = addrFrom;
	kdnet_sendHeader.addrTo = addrTo;
	kdnet_sendHeader.id = kdnet_sendId++;
	kdnet_sendHeader.type = 2;
	kdnet_sendData = data;
	kdnet_sendLen = len;
	kdnet_retries = 0;
	kdnet_state = KDNET_STATE_TOSEND;
	
	ER(kdnet_writePacket());
	
	return KDNET_SUCCESS;
}*/
uint8_t kdnetSend(TKDNETConnection* conn, uint8_t* data, uint16_t len)
{
	if (len > conn->capacity)
	{
		KDNET_DEBUG("Invalid len (%d > %d", len, conn->capacity);
		return KDNET_ERROR;
	}
	memcpy(conn->buf, data, len);
	conn->dataLen = len;
	conn->state = CONN_TO_SEND;
	
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
		ER(kdnetProcess());
	return KDNET_SUCCESS;
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

uint8_t kdnet_cb_onChannelBusy()
{
	kdnet_syncReceived = 1;
	kdnet_syncReceivedTime = getTicks();
	// kdnet_syncs++;
	// myprintf ("sync!...\r\n");
	//KDNET_DEBUG("busy");
	kdnet_setChannelBusy();
	return KDNET_SUCCESS;
}
uint8_t kdnet_cb_onPacketSent()
{
	switch (kdnet_state)
	{
	case KDNET_STATE_SENDING:
		kdnet_packetSendTime = getTicks();
		// if (kdnet_sendHeader.type == 0)
		// {
			// ER(kdnet_startListeningACK());
			// KDNET_DEBUG("Sent successfuly, waiting for ACK...");
		// }
		// else
		// {
			ER(kdnet_startListening());
			KDNET_DEBUG("Sent successfuly");
			// myprintf ("time: %d\r\n", getTicks() - kdnet_sendTime);
		// }
		break;
	case KDNET_STATE_SENDING_ACK:
		// KDNET_DEBUG ("Waiting for ack send... %02x", val);
		ER(kdnet_startListening());
		KDNET_DEBUG("ACK sent successfuly");
		break;
	}

	return KDNET_SUCCESS;
}
uint8_t kdnet_cb_onPacketReceived()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	uint16_t payloadLen;
	ER(kdnet_driver_readPayload(data, &payloadLen));

	kdnet_recvAvail = 0;

	kdnet_recvLen = payloadLen - sizeof(TKDNETHeader) - 2;

	TKDNETHeader *header = (TKDNETHeader*)data;
	TKDNETConnection* conn = kdnet_connByHeaderRev(header);
	if (!conn)
		return KDNET_SUCCESS;

	conn->inDataLen = payloadLen - sizeof(TKDNETHeader) - 2;
	memcpy(conn->inBuf, data + sizeof(TKDNETHeader), conn->inDataLen);

	kdnet_lastRead = getTicks();

	KDNET_DEBUG("Valid packet received from: 0x%02x to: 0x%02x id: %d type: %d",
							 header->addrFrom,
							 header->addrTo,
							 header->id,
							 header->type);

	if (header->type == KDNET_TYPE_NORMAL)
	{
		if (header->id > conn->inId) // new packet arrived
		{
			conn->inId = header->id;
			ER(kdnet_writeACK(conn, header->id));
			conn->onRead();
		}
		else // old packet arrived but we'll ack it
		{
			ER(kdnet_writeACK(conn, header->id));
		}
	}
	else if (header->type == KDNET_TYPE_ACK)
	{
		if (header->id == conn->id)
		{
			KDNET_DEBUG("ACK received");
			conn->onSent();
			conn->state = CONN_SENT;
			conn->id++;
			kdnet_startListening();
		}
	}
	else
	{
		kdnet_startListening();
	}

	/*switch (kdnet_state)
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
					ER(kdnet_writeACK(kdnet_recvHeader.id));
				}
				else // old packet arrived but we'll ack it
				{
					ER(kdnet_writeACK(kdnet_recvHeader.id));
				}
			}
			else if (kdnet_recvHeader.type == 2)
			{
				kdnet_recvAvail = 1;
				ER(kdnet_startListening());
			}
			else
			{
				KDNET_DEBUG("Why did we receive ACK here?");
				ER(kdnet_startListening());
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
			ER(kdnet_startListening());
		}
		else
		{
			KDNET_DEBUG("Why did we receive non-ACK here?");
			ER(kdnet_startListening());
		}
		break;
	}
	/*}
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
			ER(kdnet_startListeningACK());
			break;
		}
	}
	else
	{
		// myprintf("res=1\r\n");
	}*/
	
	return KDNET_SUCCESS;
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
