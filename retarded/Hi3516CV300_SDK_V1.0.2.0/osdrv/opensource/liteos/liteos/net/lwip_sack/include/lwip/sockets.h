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

#ifndef __LWIP_SOCKETS_H__
#define __LWIP_SOCKETS_H__

#include "lwip/opt.h"

#if LWIP_SOCKET /* don't build if not configured for use in lwipopts.h */

#include <stddef.h> /* for size_t */

#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include <poll.h>
#include <netinet/tcp.h>
#include <net/if_packet.h>
#ifdef __cplusplus
extern "C" {
#endif


#if LWIP_UDP && LWIP_UDPLITE
/*
 * Options for level IPPROTO_UDPLITE
 */
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif /* LWIP_UDP && LWIP_UDPLITE*/

#define lwip_accept         accept
#define lwip_bind           bind
#define lwip_shutdown       shutdown
#define lwip_close          closesocket
#define lwip_connect        connect
#define lwip_getsockname    getsockname
#define lwip_getpeername    getpeername
#define lwip_setsockopt     setsockopt
#define lwip_getsockopt     getsockopt
#define lwip_listen         listen
#define lwip_recv           recv
#define lwip_recvfrom       recvfrom
#define lwip_send           send
#define lwip_sendto         sendto
#define lwip_socket         socket

/*
Func Name:  lwip_accept
*/
/**
* @defgroup lwip_accept
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
* @endcode
*
* @par Purpose
*  This API is used to accept a connection on socket.
*
* @par Description
*  This API extracts the first connection request on the queue of pending connections
*  for the listening socket 's', creates a new connected socket, and returns a new
*  file descriptor  referring to that  socket. The newly created socket is not in the
*  the listening state. The original socket 's' is unaffected by this call.
*
* @param[in]    s           File descriptor referring to original input socket. [N/A]
* @param[in]    addr     Pointer to sockaddr structure that identify connection. [N/A]
* @param[in]    addrlen     Size name structure. [N/A]
*
* @par Return values
*  New socket file descriptor: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_connect
*/
int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);



/*
Func Name:  lwip_bind
*/
/**
* @defgroup lwip_bind
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
* @endcode
*
* @par Purpose
*  This API is used to associate a local address or name with a socket.
*
* @par Description
*  This API assigns the address specified by name to the socket referred to
*  by the file descriptor 's'. namelen specifies the size, in bytes, of the address
*  structure pointed to by name.
*
* @param[in]    s             file descriptor referring to original input socket [N/A]
* @param[in]    name       Pointer to sockaddr structure that identify connection [N/A]
* @param[in]    namelen  Size name structure [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_bind(int s, const struct sockaddr *name, int namelen);



/*
Func Name:  lwip_shutdown
*/
/**
* @defgroup lwip_shutdown
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_shutdown(int s, int how);
* @endcode
*
* @par Purpose
*  This is used to shut down socket send and receive operations.
*
* @par Description
*  This API is used to shut down the send and receive operations. lwip_bind() assigns the address specified by name to the socket referred to
*  by the file descriptor 's'. namelen specifies the size, in bytes, of the address
*  structure pointed to by name.
*
* @param[in]    s         file descriptor referring to socket [N/A]
* @param[in]    how     type of shutdown [SHUT_RD|SHUT_WR|SHUT_RDWR|N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Only "SHUT_RDWR" is supported for "how" parameter in this API. Closing one
* end of the full-duplex connection is not supported in LwIP.\n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_shutdown(int s, int how);



/*
Func Name:  lwip_getpeername
*/
/**
* @defgroup lwip_getpeername
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
* @endcode
*
* @par Purpose
*  This API is used to get name of connected peer socket.
*
* @par Description
*  This API returns the address of the peer connected to the socket 's', in the buffer pointed to by name.
*  The namelen argument should be initialized  to  indicate the amount of space pointed to by name.
*  On return it contains the actual size of the name returned (in bytes). The name is truncated if the
*  buffer provided is too small.
*
* @param[in]    s             file descriptor referring to connected socket [N/A]
* @param[in]    name       Pointer to sockaddr structure that identify connection [N/A]
* @param[in]    namelen  Size name structure [N/A]
*
* @retval int   On success [0|N/A]
* @retval int   On failure [-1|N/A]
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_getsockname
*/
int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);



/*
Func Name:  lwip_getsockname
*/
/**
* @defgroup lwip_getsockname
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
* @endcode
*
* @par Purpose
*  This API is used to get name of socket.
*
* @par Description
*  This API returns the current address to which the socket 's'  is bound, in the buffer pointed to by name.
*  The namelen argument should be initialized to indicate the amount of space(in bytes) pointed to by
*  name.The returned address is truncated if the buffer provided is too small; in this case, namelen will
*  return a value greater than was supplied to the call.
*
* @param[in]    s             file descriptor referring to connected socket [N/A]
* @param[in]    name	   Pointer to sockaddr structure that identify connection [N/A]
* @param[in]    namelen  Size name structure [N/A]
*
* @retval int   On success [0|N/A]
* @retval int   On failure [-1|N/A]
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_getpeername
*/
int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);



/*
Func Name:  lwip_getsockopt
*/
/**
* @defgroup lwip_getsockopt
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
* @endcode
*
* @par Purpose
*  This API is used to get options set on socket.
*
* @par Description
*  This API retrieves the value for the option specified by the optname argument for the socket
*  specified by 's'. If the size of the optval is greater than optlen, the value stored in the object
*  pointed to by the optval argument shall be silently truncated.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    level	   protocol level at which the option resides [N/A]
* @param[in]    optname  a single option to be retrieved [N/A]
* @param[in]    optval      address to store option value [N/A]
* @param[in]    optlen      size of option value [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Supported protocol levels are: SOL_SOCKET, IPPROTO_IP, IPPROTO_TCP.\n
* 2. Under SOL_SOCKET the options supported are: SO_ACCEPTCONN, SO_BROADCAST,
* SO_ERROR, SO_KEEPALIVE, SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF, SO_REUSEADDR,
* SO_TYPE, SO_NO_CHECK, SO_BINDTODEVICE, SO_DONTROUTE, SO_SNDBUF. 
* For SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF,
* the macros LWIP_SO_SNDTIMEO, LWIP_SO_RCVTIMEO and LWIP_SO_RCVBUF should have
* been defined at compile time. For SO_REUSEADDR, the macro SO_REUSE
* should have been defined at compile time. For SO_BINDTODEVICE, the macro 
* LWIP_SO_BINDTODEVCE should have been defined at compile time.
* For SO_SNDBUF, the macro LWIP_SO_SNDBUF should have been defined at compile time.\n
* 3. Under IPPROTO_IP the options supported are: IP_TTL, IP_TOS.\n
* 4. Under IPPROTO_TCP the options supported are: TCP_NODELAY, TCP_KEEPALIVE,
* TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT, TCP_QUEUE_SEQ. For TCP_KEEPIDLE,
* TCP_KEEPINTVL, TCP_KEEPCNT, the macro LWIP_TCP_KEEPALIVE should have been
* defined at compile time.\n
*
* @par Related Topics
* lwip_setsockopt
*/
int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);



/*
Func Name:  lwip_setsockopt
*/
/**
* @defgroup lwip_setsockopt
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
* @endcode
*
* @par Purpose
*  This API is used to set options on socket.
*
* @par Description
*  This API sets the option specified by the optname, at the protocol level specified by the level,
*  to the value pointed to by the optval for the socket associated with the file descriptor specified by 's'.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    level	   protocol level at which the option resides [N/A]
* @param[in]    optname  a single option to set [N/A]
* @param[in]    optval      address to store option value [N/A]
* @param[in]    optlen      size of option value [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Supported protocol levels are: SOL_SOCKET, IPPROTO_IP, IPPROTO_TCP\n
* 2. Under SOL_SOCKET the options supported are: SO_brcast, SO_KEEPALIVE,
* SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF, SO_REUSEADDR, SO_SNDBUF,
* SO_NO_CHECK. For SO_SNDTIMEO, SO_RCVTIMEO, SO_RCVBUF, SO_ATTACH_FILTER,
* SO_DETACH_FILTER, SO_DONTROUTE, SO_BINDTODEVICE. The macros LWIP_SO_SNDTIMEO,
* LWIP_SO_RCVTIMEO and LWIP_SO_RCVBUF should have been defined at compile time.
* For SO_REUSEADDR, the macro SO_REUSE should have been
* defined at compile time. For SO_ATTACH_FILTER, SO_DETACH_FILTER,
* the macro LWIP_SOCKET_FILTER should have been defined at compile time.
* Only PF_PACKET RAW socket supports SO_ATTACH_FILTER and SO_DETACH_FILTER now.
* For SO_BINDTODEVICE, the macro LWIP_SO_BINDTODEVICE should have been defined at compile time.
* For SO_SNDBUF, the macro LWIP_SO_SNDBUF should have been defined at compile time.
* Only TCP socket in listen or close stated supports SO_SNDBUF.\n
* 3. Under IPPROTO_IP the options supported are: IP_TTL, IP_TOS.\n
* 4. Under IPPROTO_TCP the options supported are: TCP_NODELAY, TCP_KEEPALIVE,
* TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT. For TCP_KEEPIDLE, TCP_KEEPINTVL,
* TCP_KEEPCNT, the macro LWIP_TCP_KEEPALIVE should have been defined at compile time.\n
*
* @par Related Topics
* lwip_getsockopt
*/
int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);



/*
Func Name:  lwip_close
*/
/**
* @defgroup lwip_close
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_close(int s);
* @endcode
*
* @par Purpose
*  This API is used to close the socket.
*
* @par Description
*  This API closes the socket file descriptor.
*
* @param[in]    s      socket file descriptor [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_close(int s);



/*
Func Name:  lwip_connect
*/
/**
* @defgroup lwip_connect
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
* @endcode
*
* @par Purpose
*  This is used to initiate a connection on the socket.
*
* @par Description
*  This API connects the socket referred to by the file descriptor 's' to the address specified by name.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    name	   Pointer to sockaddr structure that identify connection [N/A]
* @param[in]    namelen  Size name structure [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);



/*
Func Name:  lwip_listen
*/
/**
* @defgroup lwip_listen
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_listen(int s, int backlog);
* @endcode
*
* @par Purpose
*  This API is used to set a socket into listen mode.
*
* @par Description
*  This API marks the socket referred to by 's' as a passive socket, that is, as a socket that will be used
*  to accept incoming connection requests using lwip_accept().
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    backlog	   number of connections in listen queue [N/A]
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_listen(int s, int backlog);



/*
Func Name:  lwip_recv
*/
/**
* @defgroup lwip_recv
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_recv(int s, void *mem, size_t len, int flags);
* @endcode
*
* @par Purpose
*  This API is used to recieve a message from connected socket.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented sockets only
*  because it doesnot permits the application to retrieve the source address of received data.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    mem	   buffer to store recieved data  [N/A]
* @param[in]    len	          number of bytes of data to be recieved [N/A]
* @param[in]    flags	   types of message reception [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. TCP receive buffer is a list, which holds the segement received from peer.
* If application is calling lwip_recv to get the data, then it just tries to get the
* first entry from the list and gives back to application. This doesn't get recursively
* from list to fill the complete user buffer. \n
* 2. LwIP updates this receive buffer list, once it Gets the next expected segment.
* If there is any out of order segment which is next to the received segment, means
* it merges and puts that as one segemnt into receive buffer list.\n
* 3. if the apps's not read the packet form the socket and the recv buffered
* packets up to MAX_MBOX_SIZE, the incoming packet may be discard and the tcp
* connection may rst by the remote.
*
* @par Related Topics
* lwip_read
* lwip_recvfrom
*/
ssize_t lwip_recv(int s, void *mem, size_t len, int flags);



/*
Func Name:  lwip_read
*/
/**
* @defgroup lwip_read
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_read(int s, void *mem, size_t len);
* @endcode
*
* @par Purpose
*  This API is used to read bytes from a socket file descriptor.
*
* @par Description
*  This API is used on a connection-oriented socket to receive data. If the received message is larger than
*  the supplied memory area, the excess data is silently discarded.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    mem	   buffer to store recieved data  [N/A]
* @param[in]    len	          number of bytes of data to be recieved [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_recv
* lwip_recvfrom
*/
int lwip_read(int s, void *mem, size_t len);



/*
Func Name:  lwip_recvfrom
*/
/**
* @defgroup lwip_recvfrom
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
* @endcode
*
* @par Purpose
*  This API is used to recieve a message from a connected and non-connected sockets.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented and connectionless sockets
*  because it permits the application to retrieve the source address of received data.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    mem	   buffer to store recieved data  [N/A]
* @param[in]    len	          number of bytes of data to be recieved [N/A]
* @param[in]    flags	   types of message reception [N/A]
* @param[in]    from        Pointer to sockaddr structure that contains source address of recieved data [N/A]
* @param[in]    fromlen    size of from structure [N/A]
*
* @par Return values
*  Number of bytes recieved: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. if the apps's not read the packet form the socket and the recv buffered
* packets up to MAX_MBOX_SIZE, the incoming packet may be discard by lwIP.
* \n
* N/A
*
* @par Related Topics
* lwip_read
* lwip_recv
*/
ssize_t lwip_recvfrom(int s, void *mem, size_t len, int flags,
      struct sockaddr *from, socklen_t *fromlen);



/*
Func Name:  lwip_send
*/
/**
* @defgroup lwip_send
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_send(int s, const void *dataptr, size_t size, int flags);
* @endcode
*
* @par Purpose
*  This API is used to send a message to connected socket.
*
* @par Description
*  This API initiate transmission of a message from the specified socket to its peer.
*  This API will send a message only when the socket is connected.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    dataptr	   buffer containing message to be sent  [N/A]
* @param[in]    size	   length of message to be sent [N/A]
* @param[in]    flags	   types of message transmission [N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. UDPI & RAW connection can send only a maximum data of length 65000. Sending more data would
* return -1 and errno set to EMSGSIZE. \n
* N/A
*
* @par Related Topics
* lwip_write
* lwip_sendto
*/
ssize_t lwip_send(int s, const void *dataptr, size_t size, int flags);



/*
Func Name:  lwip_sendto
*/
/**
* @defgroup lwip_sendto
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
* @endcode
*
* @par Purpose
*  This API is used to send a message to a connected and non-connected sockets.
*
* @par Description
*  This API can be used to receive messages from a connection-oriented and connectionless sockets
*  If the socket is connectionless-mode, the message shall be sent to the address specified by 'to'.
*  If the socket is connection-mode, destination address in 'to' shall be ignored.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    dataptr	   buffer containing message to be sent  [N/A]
* @param[in]    size	   length of message to be sent [N/A]
* @param[in]    flags	   types of message transmission [N/A]
* @param[in]    to            Pointer to sockaddr structure that contains destination address [N/A]
* @param[in]    tolen        size of to structure [N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. UDP & RAW connection can send only a maximum data of length 65000. Sending more data would
* return -1 and errno set to EMSGSIZE. \n
*
* @par Related Topics
* lwip_write
* lwip_send
*/
ssize_t lwip_sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen);



/*
Func Name:  lwip_socket
*/
/**
* @defgroup lwip_socket
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_socket(int domain, int type, int protocol);
* @endcode
*
* @par Purpose
*  This API is used to allocate a socket .
*
* @par Description
*  This API is used to create an endpoint for communication and returns a file descriptor.
*
* @param[in]    domain    protocol family [N/A]
* @param[in]    type	   socket type  [SOCK_RAW|SOCK_DGRAM|SOCK_STREAM|N/A]
* @param[in]    protocol   protocol to be used with the socket [N/A]
*
* @par Return values
* Valid socket file descriptor: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1: For AF_INET socket, type SOCK_RAW |SOCK_DGRAM|SOCK_STREAM is supported.
* 2: For AF_PACKET socket, only type SOCK_RAW is supported.
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_socket(int domain, int type, int protocol);



/*
Func Name:  lwip_write
*/
/**
* @defgroup lwip_write
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_write(int s, const void *dataptr, size_t size);
* @endcode
*
* @par Purpose
*  This API is used to write data bytes to a socket file descriptor.
*
* @par Description
*  This API is used on a connection-oriented sockets to send data.
*
* @param[in]    s             socket file descriptor [N/A]
* @param[in]    dataptr	   buffer containing message to be sent  [N/A]
* @param[in]    size	   length of message to be sent [N/A]
*
* @par Return values
*  Number of bytes sent: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* lwip_send
* lwip_sendto
*/
int lwip_write(int s, const void *dataptr, size_t size);

/*
 * Func Name:  lwip_poll
 * */
/**
 * * @defgroup lwip_poll
 * * @ingroup Socket_Interfaces
 * * @par Prototype
 * * @code
 * * int lwip_poll(int sockfd, poll_table *wait);
 * * @endcode
 * *
 * * @par Purpose
 * *  This API is used to poll on multiple file descriptors, waiting until one or more of the file descriptors
 * *  become "ready" for some of I/O operations.
 * *
 * * @par Description
 * *  This API is  used to poll on socket fd
 * *
 * * @param[in]    sockfd    socket file descriptor
 * * @param[in]    wait   poll  table struct include events user provided
 * *
 * * @par Return values
 * *  Socket file descriptor: On success \n
 * *  -1: On failure \n
 * *
 * *
 * * @par Required Header File
 * * sockets.h
 * *
 * * @par Note
 * * \n
 * * N/A
 * *
 * * @par Related Topics
 * * \n
 * * N/A
 * */
int lwip_poll(int sockfd, poll_table *wait);

/*
Func Name:  lwip_select
*/
/**
* @defgroup lwip_select
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
* @endcode
*
* @par Purpose
*  This API is used to keeep tabs on multiple file descriptors, waiting until one or more of the file descriptors
*  become "ready" for some of I/O operations.
*
* @par Description
*  This API is  used to examine  the file descriptor sets whose addresses are passed in the readset, writeset, and exceptset
*  parameters to see whether some of their descriptors are ready for reading, are ready for writing, or have an exceptional
*  condition pending, respectively.
*
* @param[in]    maxfdp1    range of file descriptors [N/A]
* @param[in]    readset	     ptr to struct fd_set, specifies descriptor to be checked for being ready to read [N/A]
* @param[in]    writeset	     ptr to struct fd_set, specifies descriptor to be checked for being ready to write [N/A]
* @param[in]    exceptset   ptr to struct fd_set, specifies descriptor to be checked for pending error conditions [N/A]
* @param[in]    timeout      ptr to struct timeval, for timeout application [N/A]
*
* @par Return values
*  Socket file descriptor: On success \n
*  -1: On failure \n
*
*
* @par Required Header File
* sockets.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);



/*
Func Name:  lwip_ioctl
*/
/**
* @defgroup lwip_ioctl
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_ioctl(int s, long cmd, void *argp);
* @endcode
*
* @par Purpose
*  This API is used as a control device.
*
* @par Description
*  This API is used as a control device.

* @param[in]    s        open socket file descriptor [N/A]
* @param[in]    cmd   command to select control function [FIONREAD|FIONBIO|N/A]
* @param[in]    argp   additional info, if required [N/A]
*
*
* @par Return values
*  0: On success \n
*  -1: On failure \n
*
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. Linux API supports variable argument support. But LwIP API supports only one void * as
* 3rd argument. \n
* 2. Options supported by this api are :
* SIOCADDRT: set IF gateway, soft-route is not support by lwIP yet.\n
* SIOCGIFADDR: get ifnet address.\n
* SIOCGIFFLAGS: get ifnet flags.\n
* SIOCSIFFLAGS: set ifnet flags.\n
*   IFF_UP interface is up.\n
*   IFF_BROADCAST broadcast address valid.\n
*   IFF_LOOPBACK is a loopback net.\n
*   IFF_POINTOPOINT is a point-to-point link.\n
*   IFF_DRV_RUNNING resources allocated.\n
*   IFF_NOARP no address resolution protocol.\n
*   IFF_MULTICAST supports multicast.\n
*   IFF_DYNAMIC dialup device with changing addresses.\n
*   IFF_DYNAMIC_S dialup device with changing addresses.\n
* SIOCGIFADDR: get ifnet address.\n
* SIOCSIFADDR: set ifnet address.\n
* SIOCGIFNETMASK: get net addr mask.\n
* SIOCSIFNETMASK : set net addr mask.\n
* SIOCSIFHWADDR: set IF mac_address.\n
* SIOCGIFHWADDR: get IF mac_address\n
* SIOCGIFNAME: get IF name.\n
* SIOCSIFNAME: set IF name.\n
* SIOCGIFINDEX: get IF index.\n
* SIOCGIFCONF: get netif config. \n
* FIONBIO: set/clear non-blocking i/o.\n
* 3. For FIONREAD option, argp should point to an application variable of type signed int. \n
* @par Related Topics
* \n
* N/A
*/
int lwip_ioctl(int s, long cmd, void *argp);



/*
Func Name:  lwip_fcntl
*/
/**
* @defgroup lwip_fcntl
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_fcntl(int s, int cmd, int val);
* @endcode
*
* @par Purpose
*  This API is used to manipulate file descriptor.
*
* @par Description
*  This API is used to manipulate file descriptor.

* @param[in]    s        socket file descriptor [N/A]
* @param[in]    cmd   command to select an operation [F_GETFL|F_SETFL|N/A]
* @param[in]    val     additional flag, to set non-blocking [N/A]
*
*
* @par Return values
*  Postitive value: On success \n
*  -1: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1.  Function prototype does not support variable arguments like linux fcntl API.\n
* 2. Only F_GETFL & F_SETFL commands are supported. For F_SETFL, only O_NONBLOCK is supported for val.
*
* @par Related Topics
* \n
* N/A
*/
int lwip_fcntl(int s, int cmd, int val);

/* internal function,
   Call this function to intialise global socket resources
*/
int sock_init(void);

/*
Func Name:  lwip_get_conn_info
*/
/**
* @defgroup lwip_get_conn_info
* @ingroup Socket_Interfaces
* @par Prototype
* @code
* int lwip_get_conn_info (int s, struct tcpip_conn * conninfo);
* @endcode
*
* @par Purpose
*  This API is used to get tcp or udp connection information.
*
* @par Description
*   This API is used to get tcp or udp connection information.

* @param[in]    s        socket file descriptor [N/A]
* @param[out]  conninfo  Connection information  details of given socket
*
* @par Return values
*    0: On success \n
*    Negative value: On failure \n
*
* @par Required Header File
* sockets.h
*
* @par Note
* 1. This function called to get tcp or udp  connection information.
*
* @par Related Topics
* \n
* N/A
*/
int lwip_get_conn_info (int s, void *conninfo);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_SOCKET */

#endif /* __LWIP_SOCKETS_H__ */
