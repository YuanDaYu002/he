
#ifndef __U_ETHER_H__
#define __U_ETHER_H__

#include USB_GLOBAL_INCLUDE_FILE
#include "gadget/skbuff.h"
#include "linux/workqueue.h"
#include "los_mac.h"
#include "net/usb_eth_drv.h"
#include "linux/list.h"
#include "asm/atomic.h"
#include "linux/timer.h"

#define NETDEV_ALIGN 32
#define ETHER_QMULT_DEFAULT 5
#define NO_DEV ENODEV
#define INVAL_ARGU EINVAL

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define ETH_IDLE_STATUS 0
#define ETH_SEND_STATUS 1
#define ETH_RECIVE_STATUS 2

/* table 62; bits in multicast filter */
#define USB_CDC_PACKET_TYPE_PROMISCUOUS (1 << 0)
#define USB_CDC_PACKET_TYPE_ALL_MULTICAST   (1 << 1) /* no filter */
#define USB_CDC_PACKET_TYPE_DIRECTED    (1 << 2)
#define USB_CDC_PACKET_TYPE_BROADCAST   (1 << 3)
#define USB_CDC_PACKET_TYPE_MULTICAST   (1 << 4) /* filtered */
#define DEFAULT_FILTER  (USB_CDC_PACKET_TYPE_BROADCAST | USB_CDC_PACKET_TYPE_ALL_MULTICAST \
    |USB_CDC_PACKET_TYPE_PROMISCUOUS | USB_CDC_PACKET_TYPE_DIRECTED)

struct gether;

struct eth_dev {
    /* lock is held while accessing port_usb
    */
    spinlock_t lock;
    struct f_rndis *rndis;

    struct sk_buff *skbuf;

    struct fether_softc *fether;
    struct los_eth_driver *sc;
    struct usbd_gadget_device *gadget;

    spinlock_t req_lock; /* guard {rx,tx}_reqs */
    struct list_head tx_reqs, rx_reqs;
    unsigned int tx_qlen;
    unsigned int tx_transferring;
    unsigned int rx_transferring;

    struct sk_buff_head rx_frames;
    unsigned qmult;
    unsigned header_len;
    struct sk_buff *(*wrap)(struct gether *port, struct sk_buff *skbuf);
    int (*unwrap)(struct gether *port, struct sk_buff *skbuf, struct sk_buff_head *list);

    unsigned long todo;
#define WORK_RX_MEMORY  0
    bool zlp;
    unsigned char host_mac[ETH_ALEN];
    unsigned char dev_mac[ETH_ALEN];

    struct timer_list eth_timer;
};

struct usb_function {
    const char *name;
    /* configuration management:  bind/unbind */
    int (*bind)(struct eth_dev *,
        struct usb_function *);
    void (*unbind)(struct eth_dev *,
        struct usb_function *);
    void (*free_func)(struct usb_function *f);

    /* runtime state management */
    int (*set_alt)(struct eth_dev *, unsigned alt);
    int (*get_alt)(struct usb_function *,
        unsigned interface);
    void (*disable)(struct eth_dev *);
    int (*setup)(struct eth_dev *,
        const struct usb_device_request *);
    void (*suspend)(struct usb_function *);
    void (*resume)(struct usb_function *);

    /* USB 3.0 additions */
    int (*get_status)(struct usb_function *);
    int (*func_suspend)(struct usb_function *,
        unsigned char suspend_opt);
    /* private: */
    /* internals */
    struct list_head list;
};

struct gether
{
    struct usb_function func;

    struct eth_dev *ioport;

    struct usbd_endpoint *in_ep;
    struct usbd_endpoint *out_ep;
    unsigned int in_ep_enable;
    unsigned int out_ep_enable;

    bool is_zlp_ok;
    unsigned int open_flag;

    unsigned short cdc_filter;

    /* hooks for added framing, as needed for RNDIS and EEM. */
    unsigned int header_len;
    unsigned ul_max_pkts_per_xfer;
    unsigned dl_max_pkts_per_xfer;
    bool multi_pkt_xfer;
    bool supports_multi_frame;
    struct sk_buff *(*wrap)(struct gether *port, struct sk_buff *skbuf);
    int (*unwrap)(struct gether *port, struct sk_buff *skbuf, struct sk_buff_head *list);

    /* called on network open/close */
    void (*open)(struct gether *);
    void (*close)(struct gether *);
};

/*
 * variant of gether_setup_default that allows customizing
 * network device name
 */
struct eth_dev *gether_setup_name_default(const char *netname);
void gether_get_host_addr_u8(struct eth_dev *dev, u8 host_mac[ETH_ALEN]);
void ether_rx(struct eth_dev *dev, struct sk_buff *skb);
struct eth_dev *gether_connect(struct gether *link);
void gether_disconnect(struct gether *link);

/* gether_setup_default - initialize one ethernet-over-usb link
 * Context: may sleep
 *
 * This sets up the single network link that may be exported by a
 * gadget driver using this framework.  The link layer addresses
 * are set to random values.
 *
 * Returns negative errno, or zero on success
 */
static inline struct eth_dev *gether_setup_default(void)
{
    struct eth_dev *usb = gether_setup_name_default("usb");
    return usb;
}

#endif