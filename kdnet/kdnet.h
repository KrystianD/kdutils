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

extern uint32_t getTicks();
extern int randMinMaxInt(int min, int max);

extern uint32_t kdnetAddr;
extern uint32_t kdnet_sendId, kdnet_recvId;

extern uint8_t kdnet_state;
extern TKDNETHeader kdnet_sendHeader;
extern uint8_t kdnet_sendLen;
extern uint8_t *kdnet_sendData;
extern uint8_t kdnet_retries, kdnet_syncs;

extern TKDNETHeader kdnet_recvHeader;
extern uint8_t kdnet_recvLen;
extern uint8_t kdnet_recvData[64];
extern uint8_t kdnet_recvAvail;

extern uint8_t kdnet_syncReceived;
extern uint32_t kdnet_syncReceivedTime;

extern uint32_t kdnet_sendDelayTime;
extern uint8_t kdnet_channelFree;
extern uint8_t enableStatus;

//stats
extern uint32_t kdnet_syncNoPayload;

uint8_t kdnetInit();
uint8_t kdnetProcess();

uint8_t kdnetSendTo(uint8_t addrFrom, uint8_t addrTo, uint8_t* data, uint8_t len);
static inline TKDNETHeader* kdnetGetHeader()
{
	return &kdnet_recvHeader;
}
static inline uint8_t kdnetGetLen()
{
	return kdnet_recvLen;
}
static inline uint8_t* kdnetGetData()
{
	return kdnet_recvData;
}
uint8_t kdnetIsSending();
uint8_t kdnetWaitToSend();
uint8_t kdnetIsChannelClear();
uint8_t kdnetIsAvail();
uint8_t kdnetClear();

uint16_t kdnetCRC16Update(uint16_t crc, uint8_t* data, uint8_t len);

#endif
