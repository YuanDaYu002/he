#ifndef _PROXY_ARP_H
#define _PROXY_ARP_H

#include "lwip/opt.h"

#if BRIDGE_SUPPORT /* don't build if not configured for use in lwipopts.h */

#include "netif/bridge.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct proxy_arp_info
{
    unsigned char macAddr[6];
    unsigned char ipAddr[4];
};

extern struct pbuf *proxy_arp_rcv_req(struct lwip_bridge *priv, struct pbuf *p, struct eth_arphdr *arp_hdr);
extern int proxy_arp_add(struct proxy_arp_info *arp_info);
extern int proxy_arp_delete(struct proxy_arp_info *arp_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

#endif /* _PROXY_ARP_H */
