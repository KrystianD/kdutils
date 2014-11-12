/*
 * RFM70_defs.h
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __RFM70_DEFS_H__
#define __RFM70_DEFS_H__

// Commands
#define RFM70_READ_REG       0x00
#define RFM70_WRITE_REG      0x20
#define RFM70_READ_RX_PL     0x61
#define RFM70_WRITE_TX_PL    0xa0
#define RFM70_FLUSH_TX       0xe1
#define RFM70_FLUSH_RX       0xe2
#define RFM70_REUSE_TX_PL    0xe3
#define RFM70_ACTIVATE       0x50
#define RFM70_READ_RX_PL_WID 0x60
#define RFM70_WRITE_ACK_PL   0xa8
#define RFM70_WRITE_TX_PL_NOACK 0xb0
#define RFM70_NOP            0xff

// Registers
#define RFM70_CONFIG         0x00  // Config
#    define RFM70_CONFIG_PWR_UP  0x02
#    define RFM70_CONFIG_PRIM_RX 0x01
#define RFM70_EN_AA          0x01  // Enable Auto Acknowledgment
#define RFM70_EN_RXADDR      0x02  // Enabled RX addresses
#define RFM70_SETUP_AW       0x03  // Setup address width
#define RFM70_SETUP_RETR     0x04  // Setup Auto. Retrans
#define RFM70_RF_CH          0x05  // RF channel
#define RFM70_RF_SETUP       0x06  // RF setup
#define RFM70_STATUS         0x07  // Status
#    define RFM70_STATUS_RX_DR 	       (1 << 6)
#    define RFM70_STATUS_TX_DS 	       (1 << 5)
#    define RFM70_STATUS_MAX_RT        (1 << 4)
#    define RFM70_STATUS_IRQ_MASK      (0x03 << 4)
#    define RFM70_STATUS_RX_P_NO_0     (0 << 1)
#    define RFM70_STATUS_RX_P_NO_1     (1 << 1)
#    define RFM70_STATUS_RX_P_NO_2     (2 << 1)
#    define RFM70_STATUS_RX_P_NO_3     (3 << 1)
#    define RFM70_STATUS_RX_P_NO_4     (4 << 1)
#    define RFM70_STATUS_RX_P_NO_5     (5 << 1)
#    define RFM70_STATUS_RX_P_NO_EMPTY (0x07 << 1)
#define RFM70_OBSERVE_TX     0x08  // Observe TX
#define RFM70_CD             0x09  // Carrier Detect
#define RFM70_RX_ADDR_P0     0x0a  // RX address pipe0
#define RFM70_RX_ADDR_P1     0x0b  // RX address pipe1
#define RFM70_RX_ADDR_P2     0x0c  // RX address pipe2
#define RFM70_RX_ADDR_P3     0x0d  // RX address pipe3
#define RFM70_RX_ADDR_P4     0x0e  // RX address pipe4
#define RFM70_RX_ADDR_P5     0x0f  // RX address pipe5
#define RFM70_TX_ADDR        0x10  // TX address
#define RFM70_RX_PW_P0       0x11  // RX payload width, pipe0
#define RFM70_RX_PW_P1       0x12  // RX payload width, pipe1
#define RFM70_RX_PW_P2       0x13  // RX payload width, pipe2
#define RFM70_RX_PW_P3       0x14  // RX payload width, pipe3
#define RFM70_RX_PW_P4       0x15  // RX payload width, pipe4
#define RFM70_RX_PW_P5       0x16  // RX payload width, pipe5
#define RFM70_FIFO_STATUS    0x17  // FIFO Status Register
#define RFM70_FEATURE        0x1d  // Feature Register


#endif
