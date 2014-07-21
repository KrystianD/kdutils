#ifndef __KDNET_DRIVER_H__
#define __KDNET_DRIVER_H__

#include <stdint.h>

#include "kdnet.h"

uint16_t kdnet_driver_getMaxPayloadLength();
uint8_t kdnet_driver_setIdleMode();
uint8_t kdnet_driver_setTxMode();
uint8_t kdnet_driver_setRxMode();
uint8_t kdnet_driver_waitForMode();
uint8_t kdnet_driver_writePayload(const uint8_t* data, uint16_t len);
uint8_t kdnet_driver_readPayload(uint8_t* data, uint16_t* len);
uint8_t kdnet_driver_process();
uint8_t kdnet_driver_processInterruptPacketSent();

// callback
extern uint8_t kdnet_cb_onChannelBusy();
extern uint8_t kdnet_cb_onChannelFree();
extern uint8_t kdnet_cb_onPacketSent();
extern uint8_t kdnet_cb_onPacketReceived();
extern void kdnet_cb_setConnectionRssi(int8_t rssi);

#endif
