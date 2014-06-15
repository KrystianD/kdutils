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
/*
TKDNETHeader kdnet_sendHeader;
uint8_t kdnet_sendLen;
uint8_t *kdnet_sendData;
uint8_t kdnet_retries, kdnet_syncs = 0;

TKDNETHeader kdnet_recvHeader;
uint8_t kdnet_recvLen;
uint8_t kdnet_recvData[64];
uint8_t kdnet_recvAvail;
*/
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

TKDNETConnection* kdnet_connByHeaderRev(TKDNETHeader* header)
{
	int i;
	if (header->addrTo == 0xff)
	{
		for (i = 0; i < kdnet_connSize; i++)
		{
			TKDNETConnection *c = kdnet_connetions[i];
			if (c->addrTo == 0xff)
				return c;
		}
	}
	else
	{
		for (i = 0; i < kdnet_connSize; i++)
		{
			TKDNETConnection *c = kdnet_connetions[i];
			if (c->addrFrom == header->addrTo && c->addrTo == header->addrFrom)
				return c;
		}
	}
	return 0;
}


// uint8_t kdnet_readPacket();
// uint8_t kdnet_writePacket();
uint8_t kdnet_writeACK(TKDNETConnection *conn, uint32_t idToAck);
uint8_t kdnet_startListening();

uint8_t kdnet_connHasSpace(TKDNETConnection* conn, uint16_t len);
void kdnet_connAppend(TKDNETConnection* conn, const uint8_t* data, uint16_t len);
void kdnet_connPeek(TKDNETConnection* conn, uint8_t* data, uint16_t len);
void kdnet_connRelease(TKDNETConnection* conn, uint16_t len);

uint8_t kdnetInit()
{
	kdnet_sendId = 0;
	kdnet_recvId = 0;
	//kdnet_recvAvail = 0;
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
		if (0 && getTicks() - kdnet_sendTime > KDNET_TYPICAL_SEND_TIME * 2)
		{
			KDNET_DEBUG("Sending timeouted");
			for (;;);
			// ER(kdnet_driver_setIdleMode());
			// //kdnet_retries++;
			// ER(kdnet_writePacket());
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
			if (c->state == CONN_IDLE)
			{
				KDNET_MUTEX_LOCK();
				if (c->outUsed > 0 && kdnetIsChannelClear())
				{
					kdnet_writeConnPacket(c);
					KDNET_MUTEX_UNLOCK();
					break;
				}
				KDNET_MUTEX_UNLOCK();
			}
			else if (c->state == CONN_WAIT_ACK)
			{
				if (getTicks() - c->sendTime > 2000 && kdnetIsChannelClear())
				{
					KDNET_MUTEX_LOCK();
					kdnet_writeConnPacket(c);
					KDNET_MUTEX_UNLOCK();
					break;
				}
			}
		}
		break;
	}

	ER(kdnet_driver_process());
	return KDNET_SUCCESS;
}
uint8_t kdnetProcessInterrupt()
{
	ER(kdnet_driver_processInterruptPacketSent());
	return KDNET_SUCCESS;
}

uint8_t kdnet_readPacket()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	uint16_t payloadLen;
	ER(kdnet_driver_readPayload(data, &payloadLen));

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

	if (header.addrTo == 0xff)
		header.type = KDNET_TYPE_NOACK;
	else
		header.type = KDNET_TYPE_NORMAL;

	uint16_t len;
	kdnet_connPeek(conn, (uint8_t*)&len, sizeof(len));

	uint8_t queueData[len + sizeof(len)];
	kdnet_connPeek(conn, queueData, sizeof(queueData));

	if (conn->addrTo == 0xff)
		kdnet_connRelease(conn, sizeof(queueData));

	uint8_t data[1 + len + sizeof(header)];
	data[0] = sizeof(header) + len;
	memcpy(data + 1, (uint8_t*)&header, sizeof(header));
	memcpy(data + 1 + sizeof(header), queueData + sizeof(len), len);

	ER(kdnet_driver_setIdleMode());
	ER(kdnet_driver_writePayload(data, sizeof(data)));
	ER(kdnet_driver_setTxMode());

	kdnet_setChannelBusy();

	conn->sendTime = getTicks();

	KDNET_DEBUG("Set module to send packet from 0x%02x to 0x%02x id %d of length: %d",
			header.addrFrom, header.addrTo, header.id, len);
	kdnet_state = KDNET_STATE_SENDING;

	if (header.addrTo == 0xff)
	{
		conn->state = CONN_IDLE;
		conn->id++;
	}
	else
	{
		conn->state = CONN_WAIT_ACK;
	}

	KDNET_MUTEX_UNLOCK();
	return KDNET_SUCCESS;
}
uint8_t kdnet_writeACK(TKDNETConnection* conn, uint32_t idToAck)
{
	TKDNETHeader header;
	header.addrFrom = conn->addrFrom;
	header.addrTo = conn->addrTo;
	header.id = idToAck;
	header.type = KDNET_TYPE_ACK;

	uint8_t data[1 + sizeof(header)];
	data[0] = sizeof(header);
	memcpy(data + 1, (uint8_t*)&header, sizeof(header));

	ER(kdnet_driver_setIdleMode());
	//ER(kdnet_driver_waitForMode());
	ER(kdnet_driver_writePayload(data, sizeof(data)));
	ER(kdnet_driver_setTxMode());
	//ER(kdnet_driver_waitForMode());

	kdnet_sendTime = getTicks();

	KDNET_DEBUG("Set module to send ACK from 0x%02x to 0x%02x id %d of length: %d",
			header.addrFrom, header.addrTo, header.id, sizeof(data));
	kdnet_state = KDNET_STATE_SENDING_ACK;

	return KDNET_SUCCESS;
}
uint8_t kdnet_startListening()
{
	ER(kdnet_driver_setRxMode());
	//ER(kdnet_driver_waitForMode());

	kdnet_setChannelFree();
	kdnet_syncReceived = 0;
	kdnet_state = KDNET_STATE_RECEIVING;
	KDNET_DEBUG("RX mode");

	return KDNET_SUCCESS;
}

uint8_t kdnetSend(TKDNETConnection* conn, const uint8_t* data, uint16_t len)
{
	KDNET_MUTEX_LOCK();
	if (!kdnet_connHasSpace(conn, sizeof(len) + len))
	{
		KDNET_DEBUG("No space in output buffer (%d used of %d, adding %d)",
				conn->outUsed, conn->outCapacity, len + sizeof(len));
		KDNET_MUTEX_UNLOCK();
		return KDNET_ERROR;
	}
	kdnet_connAppend(conn, (uint8_t*)&len, sizeof(len));
	kdnet_connAppend(conn, data, len);
	KDNET_MUTEX_UNLOCK();

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
	return kdnet_channelFree;// && getTicks() >= kdnet_sendDelayTime;
}/*
		uint8_t kdnetIsAvail()
		{
		return kdnet_recvAvail != 0;
		}
		uint8_t kdnetClear()
		{
//kdnet_recvAvail = 0;
}*/

uint8_t kdnet_cb_onChannelBusy()
{
	kdnet_syncReceived = 1;
	kdnet_syncReceivedTime = getTicks();
	// kdnet_syncs++;
	// myprintf ("sync!...\r\n");
	KDNET_DEBUG("busy");
	kdnet_setChannelBusy();
	return KDNET_SUCCESS;
}
uint8_t kdnet_cb_onPacketSent()
{
	switch (kdnet_state)
	{
	case KDNET_STATE_SENDING:
		ER(kdnet_startListening());
		kdnet_packetSendTime = getTicks();
		KDNET_DEBUG("Sent successfuly");
		break;
	case KDNET_STATE_SENDING_ACK:
		ER(kdnet_startListening());
		KDNET_DEBUG("ACK sent successfuly");
		break;
	}
	kdnet_setChannelFree();

	return KDNET_SUCCESS;
}
uint8_t kdnet_cb_onPacketReceived()
{
	uint8_t data[KDNET_MAX_PACKET_LEN];
	uint16_t payloadLen = 0xdeed;
	ER(kdnet_driver_readPayload(data, &payloadLen));

	kdnet_setChannelFree();

	TKDNETHeader *header = (TKDNETHeader*)data;

	KDNET_DEBUG("Valid packet received from: 0x%02x to: 0x%02x id: %d type: %d len: %d",
			header->addrFrom,
			header->addrTo,
			header->id,
			header->type,
			payloadLen);

	TKDNETConnection* conn = kdnet_connByHeaderRev(header);
	if (!conn)
		return KDNET_SUCCESS;

	if (payloadLen < sizeof(TKDNETHeader))
		return KDNET_SUCCESS;

	conn->inDataLen = payloadLen - sizeof(TKDNETHeader);
	memcpy(conn->inBuf, data + sizeof(TKDNETHeader), conn->inDataLen);

	kdnet_lastRead = getTicks();

	if (header->type == KDNET_TYPE_NORMAL)
	{
		if (header->id > conn->inId || header->id == 0) // new packet arrived
		{
			if (header->id == 0 && conn->inId != 0)
				if (conn->onReset)
					conn->onReset(conn);
			conn->inId = header->id;
			//_delay_ms(10);
			ER(kdnet_writeACK(conn, header->id));
			conn->onRead(conn);
		}
		else // old packet arrived but we'll ack it
		{
			ER(kdnet_writeACK(conn, header->id));
		}
	}
	else if (header->type == KDNET_TYPE_NOACK)
	{
		conn->bcastSender = header->addrFrom;
		kdnet_startListening();
		if (conn->onRead)
			conn->onRead(conn);
	}
	else if (header->type == KDNET_TYPE_ACK)
	{
		if (header->id == conn->id)
		{
			KDNET_DEBUG("ACK received");
			if (conn->onSent)
				conn->onSent(conn);
			conn->state = CONN_IDLE;
			conn->id++;
			kdnet_startListening();

			KDNET_MUTEX_LOCK();
			uint16_t len;
			kdnet_connPeek(conn, (uint8_t*)&len, sizeof(len));
			kdnet_connRelease(conn, len + sizeof(len));
			KDNET_MUTEX_UNLOCK();
		}
	}
	else
	{
		kdnet_startListening();
	}

	return KDNET_SUCCESS;
}

void kdnetConnectionInit(TKDNETConnection* conn)
{
	memset(conn, 0, sizeof(*conn));
	conn->state = CONN_IDLE;
}
void kdnetConnectionSetBuffers(TKDNETConnection* conn, uint8_t* outBuf, uint16_t outBufSize,
		uint8_t* inBuf, uint16_t inBufSize)
{
	conn->outBuf = outBuf;
	conn->outCapacity = outBufSize;
	conn->inBuf = inBuf;
	conn->inCapacity = inBufSize;
}

void kdnet_connPrint(TKDNETConnection* conn)
{
	KDNET_DEBUG("conn: %d of %d", conn->outUsed, conn->outCapacity);
}
uint8_t kdnet_connHasSpace(TKDNETConnection* conn, uint16_t len)
{
	return conn->outCapacity - conn->outUsed >= len ? 1 : 0;
}
void kdnet_connAppend(TKDNETConnection* conn, const uint8_t* data, uint16_t len)
{
	conn->outUsed += len;
	while (len--)
	{
		conn->outBuf[conn->outWrIdx] = *data++;
		conn->outWrIdx++;
		if (conn->outWrIdx == conn->outCapacity)
			conn->outWrIdx = 0;
	}
}
void kdnet_connPeek(TKDNETConnection* conn, uint8_t* data, uint16_t len)
{
	int outRdIdx = conn->outRdIdx;
	while (len--)
	{
		*data++ = conn->outBuf[outRdIdx];
		outRdIdx++;
		if (outRdIdx == conn->outCapacity)
			outRdIdx = 0;
	}
}
void kdnet_connRelease(TKDNETConnection* conn, uint16_t len)
{
	conn->outUsed -= len;
	while (len--)
	{
		conn->outRdIdx++;
		if (conn->outRdIdx == conn->outCapacity)
			conn->outRdIdx = 0;
	}
}
