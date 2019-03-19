/*
 * Copyright (c) 1994, 1995, 1996
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>

#include <string.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_packet.h>
#include <poll.h>
#include <netif/etharp.h>
#include <net/if_ether.h>
#include <netinet/if_ether.h>


#include "pcap-int.h"

#include <errno.h>

#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ sizeof
#endif

#define SA_LEN(addr)    (sizeof (struct sockaddr))

#define SIOCGIFCONF_BUF_MAX (4096)

#define SUPPORT_PF_PACKET_RCVTIMEO 0
/*
 * When capturing on all interfaces we use this as the buffer size.
 * Should be bigger then all MTUs that occur in real life.
 * 64kB should be enough for now.
 */
#define BIGGER_THAN_ALL_MTUS    (64*1024)

struct pcap_liteos {
    char    *device;    /* device name */
    int    timeout;    /* timeout for buffering */
    int    sock_packet;    /* using Linux 2.0 compatible interface */
    int    must_do_on_close; /* stuff we must do when we close */
    int    ifindex;    /* interface index of device we're bound to */
    int    lo_ifindex;    /* interface index of the loopback device */
};

#define pcap_snprintf snprintf
/*
 * Stuff to do when we close.
 */
#define MUST_CLEAR_PROMISC     (0x00000001)    /* clear promiscuous mode */

extern bpf_u_int32 if_flags_to_pcap_flags(const char *name _U_, u_int if_flags);

/*
 *  Query the kernel for the MTU of the given interface.
 */
static int
iface_get_mtu(int fd, const char *device, char *ebuf)
{
    struct ifreq    ifr;

    if (!device)
        return BIGGER_THAN_ALL_MTUS;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
        pcap_snprintf(ebuf, PCAP_ERRBUF_SIZE,
             "SIOCGIFMTU: %s", pcap_strerror(errno));
        return -1;
    }

    return ifr.ifr_mtu;
}

/*
 *  Get the hardware type of the given interface as ARPHRD_xxx constant.
 */
static int
iface_get_arptype(int fd, const char *device, char *ebuf)
{
    struct ifreq    ifr;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        pcap_snprintf(ebuf, PCAP_ERRBUF_SIZE,
             "SIOCGIFHWADDR: %s", pcap_strerror(errno));
        if (errno == ENODEV) {
            /*
             * No such device.
             */
            return PCAP_ERROR_NO_SUCH_DEVICE;
        }
        return PCAP_ERROR;
    }

    return ifr.ifr_hwaddr.sa_family;
}

/*
 *  Bind the socket associated with FD to the given device.
 *  Return 1 on success, 0 if we should try a SOCK_PACKET socket,
 *  or a PCAP_ERROR_ value on a hard error.
 */
static int
iface_bind(int fd, int ifindex, char *ebuf)
{
    struct sockaddr_ll    sll;
    int            err;
    socklen_t        errlen = sizeof(err);

    memset(&sll, 0, sizeof(sll));
    sll.sll_family        = AF_PACKET;
    sll.sll_ifindex        = ifindex;
    sll.sll_protocol    = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1) {
        if (errno == ENETDOWN) {
            /*
             * Return a "network down" indication, so that
             * the application can report that rather than
             * saying we had a mysterious failure and
             * suggest that they report a problem to the
             * libpcap developers.
             */
            return PCAP_ERROR_IFACE_NOT_UP;
        } else {
            pcap_snprintf(ebuf, PCAP_ERRBUF_SIZE,
                 "bind: %s", pcap_strerror(errno));
            return PCAP_ERROR;
        }
    }

    return 1;
}


/*
 *  Return the index of the given device name. Fill ebuf and return
 *  -1 on failure.
 */
static int
iface_get_id(int fd, const char *device, char *ebuf)
{
    struct ifreq    ifr;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        pcap_snprintf(ebuf, PCAP_ERRBUF_SIZE,
             "SIOCGIFINDEX: %s", pcap_strerror(errno));
        return -1;
    }

    return ifr.ifr_ifindex;
}


static int pcap_inject_liteos(pcap_t *handle, const void *buf _U_, size_t size _U_)
{
    strlcpy(handle->errbuf, "Sending packets isn't supported on liteos",
        PCAP_ERRBUF_SIZE);
    return (-1);
}

static int pcap_setdirection_liteos(pcap_t *handle, pcap_direction_t d)
{
    struct pcap_liteos *handlep = handle->priv;

    if (!handlep->sock_packet) {
        handle->direction = d;
        return 0;
    }
    pcap_snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
        "Setting direction is not supported on SOCK_PACKET sockets");
    return -1;

}

static void pcap_cleanup_liteos(pcap_t *handle)
{
    struct pcap_liteos *handlep = handle->priv;
    struct ifreq    ifr;

    if (handlep->must_do_on_close != 0) {
        /*
         * There's something we have to do when closing this
         * pcap_t.
         */
        if (handlep->must_do_on_close & MUST_CLEAR_PROMISC) {
            /*
             * We put the interface into promiscuous mode;
             * take it out of promiscuous mode.
             *
             * XXX - if somebody else wants it in promiscuous
             * mode, this code cannot know that, so it'll take
             * it out of promiscuous mode.  That's not fixable
             * in 2.0[.x] kernels.
             */
            memset(&ifr, 0, sizeof(ifr));
            strlcpy(ifr.ifr_name, handlep->device,
                sizeof(ifr.ifr_name));
            if (ioctl(handle->fd, SIOCGIFFLAGS, &ifr) == -1) {
                PRINTK("Can't restore interface %s flags (SIOCGIFFLAGS failed: %s).\n"
                    "Please adjust manually.\n"
                    "Hint: This can't happen with Linux >= 2.2.0.\n",
                    handlep->device, strerror(errno));
            } else {
                if (ifr.ifr_flags & IFF_PROMISC) {
                    /*
                     * Promiscuous mode is currently on;
                     * turn it off.
                     */
                    ifr.ifr_flags &= ~IFF_PROMISC;
                    if (ioctl(handle->fd, SIOCSIFFLAGS,
                        &ifr) == -1) {
                        PRINTK("Can't restore interface %s flags (SIOCSIFFLAGS failed: %s).\n"
                            "Please adjust manually.\n"
                            "Hint: This can't happen with Linux >= 2.2.0.\n",
                            handlep->device,
                            strerror(errno));
                    }
                }
            }
        }
    }

    if (handlep->device != NULL) {
        free(handlep->device);
        handlep->device = NULL;
    }

    pcap_cleanup_live_common(handle);

}


static int wait_frame(pcap_t *handle)
{
    struct pcap_liteos *handlep = handle->priv;
    int timeout;
    char c;
    struct pollfd pollinfo;
    int ret;

    pollinfo.fd = handle->fd;
    pollinfo.events = POLLIN;
    pollinfo.revents = 0;

    if (handlep->timeout == 0) {
        timeout = -1;    /* block forever */
    }
    else if (handlep->timeout > 0)
        timeout = handlep->timeout;    /* block for that amount of time */
    else
        timeout = 0;    /* non-blocking mode - poll to pick up errors */

    do {
            /* check for break loop condition on interrupted syscall*/
            if (handle->break_loop) {
                handle->break_loop = 0;
                return PCAP_ERROR_BREAK;
            }

            ret = poll(&pollinfo, 1, timeout);
            if (ret < 0 && errno != EINTR) {
                snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
                    "can't poll on packet socket: %s",
                    pcap_strerror(errno));
                return PCAP_ERROR;
            } else if (ret > 0 &&
                (pollinfo.revents & (POLLHUP|POLLRDHUP|POLLERR|POLLNVAL))) {
                /*
                 * There's some indication other than
                 * "you can read on this descriptor" on
                 * the descriptor.
                 */
                if (pollinfo.revents & (POLLHUP | POLLRDHUP)) {
                    snprintf(handle->errbuf,
                        PCAP_ERRBUF_SIZE,
                        "Hangup on packet socket");
                    return PCAP_ERROR;
                }
                if (pollinfo.revents & POLLERR) {
                    /*
                     * A recv() will give us the
                     * actual error code.
                     *
                     * XXX - make the socket non-blocking?
                     */
                    if (recv(handle->fd, &c, sizeof c,
                        MSG_PEEK) != -1)
                        continue;    /* what, no error? */
                    if (errno == ENETDOWN) {
                        /*
                         * The device on which we're
                         * capturing went away.
                         *
                         * XXX - we should really return
                         * PCAP_ERROR_IFACE_NOT_UP,
                         * but pcap_dispatch() etc.
                         * aren't defined to return
                         * that.
                         */
                        snprintf(handle->errbuf,
                            PCAP_ERRBUF_SIZE,
                            "The interface went down");
                    } else {
                        snprintf(handle->errbuf,
                            PCAP_ERRBUF_SIZE,
                            "Error condition on packet socket: %s",
                            strerror(errno));
                    }
                    return PCAP_ERROR;
                }
                if (pollinfo.revents & POLLNVAL) {
                    snprintf(handle->errbuf,
                        PCAP_ERRBUF_SIZE,
                        "Invalid polling request on packet socket");
                    return PCAP_ERROR;
                }
            }

    }while (ret < 0);

    return ret;
}

//static int ppp=0;
static int pcap_read_liteos(pcap_t *handle, int max_packets, pcap_handler callback, u_char *user)
{
    int    packet_len=-1, caplen=-1;
    struct bpf_insn *fcode;
    struct pcap_pkthdr    pcap_header;
    char data[1600];
    int n = 0;
    int ret;

    do
    {
        /*
         * Has "pcap_breakloop()" been called?
         */
        if (handle->break_loop) {
            /*
             * Yes - clear the flag that indicates that it has,
             * and return PCAP_ERROR_BREAK as an indication that
             * we were told to break out of the loop.
             */
            handle->break_loop = 0;
            return PCAP_ERROR_BREAK;
        }

        if(wait_frame(handle)>0)
            packet_len = recv(handle->fd, data, sizeof(data), 0);

    } while (packet_len == -1 && errno == EINTR);

    if(packet_len>0)
    {
        caplen = packet_len;
        if (caplen > handle->snapshot)
            caplen = handle->snapshot;

        pcap_header.caplen    = caplen;
        pcap_header.len        = packet_len;
        gettimeofday(&pcap_header.ts, NULL);

        if ((fcode = handle->fcode.bf_insns) == NULL ||
            bpf_filter(fcode, data, pcap_header.len, pcap_header.caplen)) {
            ret = (*callback)(user, &pcap_header, data);
            if(ret < 0)
            {
                return PCAP_ERROR;
            }
            return 1;
        }

        return 0;
    }
    else
    {
        return 0;
    }
}

static int pcap_stats_liteos(pcap_t *handle, struct pcap_stat *stats)
{
    return 0;
}

static int
pcap_set_datalink_liteos(pcap_t *handle, int dlt)
{
    handle->linktype = dlt;
    return 0;
}

static int pcap_activate_liteos(pcap_t *handle)
{
    struct pcap_liteos *handlep = handle->priv;
    const char    *device;
    struct ifreq    ifr;
    int     status = 0;
    int     ret;
    int         sock_fd = -1;
    int         err = 0;
#if SUPPORT_PF_PACKET_RCVTIMEO
    struct timeval nNetTimeout = {.tv_sec = 2, .tv_usec = 0};
#endif

    device = handle->opt.source;

    /* copy timeout value */
    handlep->timeout = handle->opt.timeout;

    handle->inject_op = pcap_inject_liteos;
    handle->setfilter_op = install_bpf_program;
    handle->setdirection_op = pcap_setdirection_liteos;
    handle->set_datalink_op = pcap_set_datalink_liteos;
    handle->getnonblock_op = pcap_getnonblock_fd;
    handle->setnonblock_op = pcap_setnonblock_fd;
    handle->cleanup_op = pcap_cleanup_liteos;
    handle->read_op = pcap_read_liteos;
    handle->stats_op = pcap_stats_liteos;

    handlep->sock_packet = 0;

    /* Low level socket */
    sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd == -1) {
        pcap_snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
             "socket: %s", pcap_strerror(errno));
        return -1;
    }

    handlep->device = strdup(device);
    if (handlep->device == NULL) {
        pcap_snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "strdup: %s",
             pcap_strerror(errno) );
        return PCAP_ERROR;
    }

    handlep->ifindex = iface_get_id(sock_fd, device, handle->errbuf);
    if (handlep->ifindex == -1) {
        close(sock_fd);
        return PCAP_ERROR;
    }

    if ((err = iface_bind(sock_fd, handlep->ifindex,
                handle->errbuf)) != 1)
    {
        close(sock_fd);
        return err;
    }

    //set recv timeout
#if SUPPORT_PF_PACKET_RCVTIMEO
    if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &nNetTimeout, sizeof(struct timeval)) < 0)
    {
        pcap_snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
             "setsockopt: %s", pcap_strerror(errno));
        close(sock_fd);
        return -1;
    }
#endif


    if (handle->opt.promisc)
    {
        memset(&ifr, 0, sizeof(ifr));
        strlcpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
        if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) == -1) {
            snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
                 "SIOCGIFFLAGS: %s", pcap_strerror(errno));
            return PCAP_ERROR;
        }
        if ((ifr.ifr_flags & IFF_LOOPBACK) == 0 && (ifr.ifr_flags & IFF_PROMISC) == 0) {
            /*
             * Promiscuous mode isn't currently on,
             * so turn it on, and remember that
             * we should turn it off when the
             * pcap_t is closed.
             */

            /*
             * If we haven't already done so, arrange
             * to have "pcap_close_all()" called when
             * we exit.
             */
            if (!pcap_do_addexit(handle)) {
                /*
                 * "atexit()" failed; don't put
                 * the interface in promiscuous
                 * mode, just give up.
                 */
                return PCAP_ERROR;
            }

            ifr.ifr_flags |= IFF_PROMISC;
            if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) == -1) {
                    snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
                     "SIOCSIFFLAGS: %s",
                     pcap_strerror(errno));
                return PCAP_ERROR;
            }
            handlep->must_do_on_close |= MUST_CLEAR_PROMISC;

            /*
             * Add this to the list of pcaps
             * to close when we exit.
             */
            pcap_add_to_pcaps_to_close(handle);
        }
    }


    handle->oneshot_callback = pcap_oneshot;
    handle->bpf_codegen_flags = 0;
    handle->activated = 1;
    handle->linktype = 1;

    /*
     * We've succeeded. Save the socket FD in the pcap structure.
     */
    handle->fd = sock_fd;

    /*
     * "handle->fd" is a socket, so "select()" and "poll()"
     * should work on it.
     */
    handle->selectable_fd = handle->fd;

    return 0;
}
static int pcap_can_set_rfmon_liteos(pcap_t *p)
{

    return 0;

}
/*
 * A SOCK_PACKET or PF_PACKET socket can be bound to any network interface.
 */
static int
can_be_bound(const char *name _U_)
{
    return (1);
}

pcap_t *
pcap_create_interface(const char *device _U_, char *ebuf)
{
    pcap_t *handle;

    handle = pcap_create_common(device, ebuf, sizeof (struct pcap_liteos));
    if (handle == NULL)
        return NULL;

    handle->activate_op = pcap_activate_liteos;
    handle->can_set_rfmon_op = pcap_can_set_rfmon_liteos;

    return handle;
}

int pcap_platform_finddevs(pcap_if_t **alldevsp, char *errbuf)
{
    int ret = 0,i = 0;
    pcap_if_t *devlist = NULL;
    int socketfd;
    struct ifconf conf;
    char *data = NULL;
    struct ifreq *ifr = NULL;
    struct ifreq ifrflags = {0};

    data = (char*)zalloc(SIOCGIFCONF_BUF_MAX);
    if(data==NULL)
    {
        *alldevsp = NULL;
        return -1;
    }

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd >= 0)
    {
        conf.ifc_len = SIOCGIFCONF_BUF_MAX;
        conf.ifc_buf = (caddr_t) data;
        if (ioctl(socketfd,SIOCGIFCONF,&conf) < 0) {
            perror("ioctl SIOCGIFCONF:");
            *alldevsp = NULL;
            free(data);
            return -1;
        }

        ifr = (struct ifreq*)data;
        while ((char*)ifr < data+conf.ifc_len) {

            /*
             * Get the flags for this interface.
             */
            strncpy(ifrflags.ifr_name, ifr->ifr_name,
                sizeof(ifrflags.ifr_name));
            if (ioctl(socketfd, SIOCGIFFLAGS, (char *)&ifrflags) < 0) {
                if (errno == ENXIO)
                    continue;
                (void)pcap_snprintf(errbuf, PCAP_ERRBUF_SIZE,
                    "SIOCGIFFLAGS: %.*s: %s",
                    (int)sizeof(ifrflags.ifr_name),
                    ifrflags.ifr_name,
                    pcap_strerror(errno));
                    ret = -1;
                    break;
            }

            switch (ifr->ifr_addr.sa_family) {
              case AF_INET:
                  ++i;
                  if (add_addr_to_iflist(&devlist, ifr->ifr_name,
                          if_flags_to_pcap_flags(ifr->ifr_name, ifrflags.ifr_flags),
                          &ifr->ifr_addr, SA_LEN(&ifr->ifr_addr),
                          NULL, 0, NULL, 0,
                          NULL, 0, errbuf) < 0) {
                          ret = -1;
                          break;
                      }
                      break;

            }
            ifr = (struct ifreq*)((char*)ifr +_SIZEOF_ADDR_IFREQ(*ifr));
        }
        close(socketfd);
    }
    else
    {
        *alldevsp = NULL;
        free(data);
        return -1;
    }

    if (ret == -1)
    {
        /*
         * We had an error; free the list we've been constructing.
         */
        if (devlist != NULL)
        {
            pcap_freealldevs(devlist);
            devlist = NULL;
        }
    }
    *alldevsp = devlist;
    free(data);
    return (0);
}
