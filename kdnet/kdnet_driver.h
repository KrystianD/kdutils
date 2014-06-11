#ifndef __KDNET_DRIVER_H__
#define __KDNET_DRIVER_H__

#include <stdint.h>

uint16_t kdnet_driver_getMaxPayloadLength();
void kdnet_driver_setIdleMode();
void kdnet_driver_setTxMode();
void kdnet_driver_setRxMode();
void kdnet_driver_writePayload(const uint8_t* data, uint16_t len);
void kdnet_driver_readPayload(uint8_t* data, uint16_t* len);
void kdnet_driver_processInterrupt();
void kdnet_driver_process();

// callback
extern void kdnet_cb_onChannelBusy();
extern void kdnet_cb_onPacketSent();
extern void kdnet_cb_onPacketReceived();

#endif
