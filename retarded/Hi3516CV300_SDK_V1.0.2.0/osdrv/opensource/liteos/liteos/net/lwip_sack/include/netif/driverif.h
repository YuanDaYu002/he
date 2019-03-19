/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

 /*
 *********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 *********************************************************************************
 */

#ifndef __DRIVERIF_H__
#define __DRIVERIF_H__

#include "lwip/opt.h"
#include "netif/etharp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Type of link layer, these macros should be used for link_layer_type of struct netif*/
#define LOOPBACK_IF         772
#define ETHERNET_DRIVER_IF  1
#define USBNET_DRIVER_IF  2
#define WIFI_DRIVER_IF           6

/* Short names of link layer */
#define LOOPBACK_IFNAME "lo"
#define ETHERNET_IFNAME "eth"
#define USBNET_IFNAME "usb"
#define WIFI_IFNAME "wlan"

/**
* @defgroup Driver_Interfaces
* This section contains the Network Driver related interfaces.
*/


/* Function pointer of driver send function */
typedef void (*drv_send_fn)(struct netif *netif, struct pbuf *p);

/* Function pointer of driver set hw address function */
/* This callback function should return 0 in case of success */
typedef u8_t (*drv_set_hwaddr_fn)(struct netif *netif, u8_t *addr, u8_t len);

#if LWIP_NETIF_PROMISC
/* Function pointer of driver set/unset promiscuous mode on interface */
typedef void (*drv_config_fn)(struct netif *netif, u32_t config_flags, u8_t setBit);
#endif  /* LWIP_NETIF_PROMISC */

err_t driverif_init(struct netif *netif);



/*
Func Name:  driverif_input
*/
/**
* @defgroup driverif_input
* @ingroup Driver_Interfaces
* @par Prototype
* @code
* void  driverif_input(struct netif *netif, struct pbuf *p);
* @endcode
*
* @par Purpose
* This function should be called by network driver to pass the input packet to LwIP.
*
* @par Description
* This function should be called by network driver to pass the input packet to LwIP.
* Before calling this API, driver has to keep the packet in pbuf structure. Driver has to
* call pbuf_alloc() with type as PBUF_RAM to create pbuf structure. Then driver
* has to pass the pbuf structure to this API. This will add the pbuf into the TCPIP thread.
* Once this packet is processed by TCPIP thread, pbuf will be freed. Driver is not required to
* free the pbuf.
*
* @param[in]    netif                The lwIP network interface [N/A]
* @param[in]    p                     packet in pbuf structure [N/A]
*
* @par Return values
*  None
*
* @par Required Header File
* driverif.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
void  driverif_input(struct netif *netif, struct pbuf *p);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

