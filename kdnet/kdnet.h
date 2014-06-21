#ifndef __KDNET_H__
#define __KDNET_H__

#include <stdint.h>

#define KDNET_SUCCESS         0
#define KDNET_ERROR           1

#define KDNET_STATE_NONE      0
#define KDNET_STATE_TOSEND    1
#define KDNET_STATE_SENDING   2
#define KDNET_STATE_SENDING_ACK   3
#define KDNET_STATE_RECEIVING     4
#define KDNET_STATE_RECEIVING_PRE 5
#define KDNET_STATE_RECEIVING_ACK 6
#define KDNET_STATE_RECEIVING_ACK_PRE 7

#define KDNET_TYPE_NORMAL 0
#define KDNET_TYPE_NOACK  1
#define KDNET_TYPE_ACK    2

#ifndef TKDNETHEADER
#define TKDNETHEADER
#pragma pack(1)
typedef struct
{
	uint8_t addrFrom, addrTo;
	uint32_t id;
	uint8_t type;
	// uint8_t len;
} TKDNETHeader;
#pragma pack()
#endif

struct _TKDNETConnection;
typedef void (*funcread_t)(struct _TKDNETConnection* conn);
typedef void (*funcsent_t)(struct _TKDNETConnection* conn);
typedef void (*funcreset_t)(struct _TKDNETConnection* conn);
typedef void (*funcerror_t)(struct _TKDNETConnection* conn, uint8_t type);
#define CONN_IDLE            0
#define CONN_TO_SEND         1
#define CONN_WAIT_TO_RCV_ACK 2
#define CONN_WAIT_TO_SND_ACK 3
#define CONN_SENT            4
struct _TKDNETConnection
{
	uint8_t addrFrom, addrTo, bcastSender;
	uint32_t id, inId;

	uint8_t *outBuf, *inBuf;
	uint16_t outCapacity, inCapacity;

	uint16_t outWrIdx, outRdIdx, outUsed;

	uint8_t state;

	uint32_t sendTime;
	uint16_t inDataLen;

	funcread_t onRead;
	funcsent_t onSent;
	funcreset_t onReset;
	funcerror_t onError;

	// time when to send ACK
	uint32_t ackSendTime;
	uint32_t ackId;

	// stats
	uint32_t lastStatsReset;
	uint16_t readPerSec, sentPerSec;
	uint16_t tmpReadPerSec, tmpSentPerSec;

	void* userdata;
};
typedef struct _TKDNETConnection TKDNETConnection;

extern uint32_t getTicks();
extern int randMinMaxInt(int min, int max);

extern TKDNETConnection kdnetConnections[];
extern TKDNETHeader kdnet_recvHeader;
extern uint8_t kdnet_recvLen;
extern uint8_t kdnet_recvData[64];

//stats
extern uint32_t kdnet_syncNoPayload;

uint8_t kdnetInit();
uint8_t kdnetProcess();
uint8_t kdnetProcessInterrupt();

uint8_t kdnetSendTo(uint8_t addrFrom, uint8_t addrTo, uint8_t* data, uint8_t len);
uint8_t kdnetSend(TKDNETConnection* conn, const uint8_t *data, uint16_t len);
uint8_t kdnetIsSending();
uint8_t kdnetWaitToSend();
uint8_t kdnetIsChannelClear();
uint8_t kdnetIsAvail();
uint8_t kdnetClear();

uint16_t kdnetCRC16Update(uint16_t crc, uint8_t* data, uint8_t len);

// connections
void kdnetConnectionInit(TKDNETConnection* conn);
void kdnetConnectionSetBuffers(TKDNETConnection* conn, uint8_t* outBuf, uint16_t outBufSize,
		uint8_t* inBuf, uint16_t inBufSize);

#endif
