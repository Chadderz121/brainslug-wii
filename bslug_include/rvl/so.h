/* so.h
 *   by Alex Chadwick
 * 
 * Copyright (C) 2014, Alex Chadwick
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* definitions of symbols inferred to exist in the so.h header file for which
 * the brainslug symbol information is available. */

#ifndef _RVL_SO_H_
#define _RVL_SO_H_

#include <stddef.h>
#include <stdint.h>

typedef enum so_ret_t so_ret_t;
typedef struct so_alloc_t so_alloc_t;
typedef int so_fd_t;
typedef enum so_af_t so_af_t;
typedef enum so_pf_t so_pf_t;
typedef enum so_type_t so_type_t;
typedef enum so_prot_t so_prot_t;
typedef struct so_addr_t so_addr_t;
typedef enum so_shut_t so_shut_t;
typedef enum so_event_t so_event_t;
typedef struct so_poll_t so_poll_t;
typedef struct so_host_t so_host_t;
typedef struct so_ainfo_t so_ainfo_t;
typedef enum so_lvl_t so_lvl_t;
typedef enum so_opt_t so_opt_t;

so_ret_t SOInit(so_alloc_t *alloc);
so_ret_t SOFinish(void);
static inline so_ret_t SOStartup(void);
so_ret_t SOStartupEx(int timeout);
so_ret_t SOCleanup(void);

/* These methods set errno! Our constants are wrong though, use the so_ret_t
 * SO_E* ones (they're negative!) */
 
/* returns an so_ret_t error value (negative) on error. */
so_fd_t SOSocket(so_pf_t protocol_family, so_type_t type, so_prot_t protocol);
/* this version is used by other libraries as it suppresses logging. */
so_fd_t __SOSocket(so_pf_t protocol_family, so_type_t type, so_prot_t protocol);
so_ret_t SOClose(so_fd_t socket);
so_ret_t SOBind(so_fd_t socket, so_addr_t *addr);
so_ret_t SOConnect(so_fd_t socket, so_addr_t *addr);
static inline so_ret_t SORecvFrom(
    so_fd_t socket, void *buffer, size_t buffer_size, int flags, 
    so_addr_t *addr);
static inline so_ret_t SORecv(
    so_fd_t socket, void *buffer, size_t buffer_size, int flags);
/* this is not intended to be called directly (feel free to replace) */
so_ret_t SOiRecvFrom(
    int r3, so_fd_t socket, void *buffer, size_t buffer_size, int flags,
    so_addr_t *addr);
static inline so_ret_t SOSendTo(
    so_fd_t socket, const void *buffer, size_t buffer_size, int flags, 
    const so_addr_t *addr);
static inline so_ret_t SOSend(
    so_fd_t socket, const void *buffer, size_t buffer_size, int flags);
/* this is not intended to be called directly (feel free to replace) */
so_ret_t SOiSendTo(
    int r3, so_fd_t socket, const void *buffer, size_t buffer_size, int flags,
    const so_addr_t *addr);

/* command is either F_GETFL or F_SETFL from <fcntl.h>. */
/* realistically, there is only ever one vararg; an int but this is what the
 * method does. Commonly used to set and clear O_NONBLOCK flag. */
so_ret_t SOFcntl(so_fd_t socket, int command, ...);
so_ret_t SOShutdown(so_fd_t socket, so_shut_t how);
so_ret_t SOPoll(so_poll_t *polls, int poll_count, long long timeout);

so_ret_t SOInetAtoN(const char *str, uint32_t *addr);
/* not reentrant or thread safe! */
const char *SOInetNtoA(const uint32_t *addr);

/* host IP or 0 on error. */
uint32_t SOGetHostID(void);
so_ret_t SOGetHostByName(const char *addr);

/* not reentrant or thread safe! */
so_ret_t SOGetAddrInfo(
    const char *node, const char *service,
    const so_ainfo_t *hints, so_addr_t **res);
void SOFreeAddrInfo(so_addr_t *res);

so_ret_t SOSetSockOpt(
    so_fd_t socket, so_lvl_t level, so_opt_t name,
    const void *data, size_t size);
so_ret_t SOGetInterfaceOpt(
    so_fd_t socket, so_lvl_t level, so_opt_t name,
    void *buffer, size_t buffer_size);

typedef void *(*so_alloc_fn_t)(int category, size_t size);
typedef void (*so_free_fn_t)(int category, void *memory, size_t size);

struct so_alloc_t {
    so_alloc_fn_t alloc;
    so_free_fn_t free;
};

enum so_ret_t {
    SO_OK = 0,
    SO_EAFNOSUPPORT = -5,
    SO_EAGAIN = -6,
    SO_EALREADY = -7,
    SO_EBUSY = -10,
    SO_EINPROGRESS = -26,
    SO_EINVAL = -28,
    SO_EIO = -29,
    SO_ENETRESET = -39,
    SO_ENOENT = -45,
    SO_ENOLINK = -48,
    SO_ENOMEM = -49,
    SO_ETIMEDOUT = -76,
    SO_E112 = -112,
    SO_E121 = -121,
};

enum so_af_t {
    AF_INET = 2, /* internet */
};

enum so_pf_t {
    PF_INET = 2, /* internet */
};

enum so_type_t {
    SOCK_STREAM = 1, /* tcp */
    SOCK_DGRAM = 2,  /* udp */
};

/* unlike what you may have been led to believe, this parameter is ALWAYS 0. */
enum so_prot_t {
    PROTO_STREAM_UDP = 0,
    PROTO_DGRAM_TCP = 0,
};

struct so_addr_t {
	uint8_t  sa_len;    /* total length */
	uint8_t  sa_family; /* address family */
	uint16_t sa_port;   /* port number */
    uint32_t sa_addr;   /* ip address */
};

enum so_shut_t {
    SHUT_RD = 1,
    SHUT_WR = 2,
    SHUT_RDWR = SHUT_RD | SHUT_WR,
};

enum so_event_t {
    POLLIN = 0x1,    /* data available */
    POLLOUT = 0x8,   /* can send */
    POLLERR = 0x20,  /* error occurred */
    POLLHUP = 0x40,  /* connection closed */
    POLLNVAL = 0x80, /* invalid request */
};

struct so_poll_t {
    so_fd_t socket;
    so_event_t mask;
    so_event_t result;
};

struct so_host_t {
    char *h_name;          /* official name of host */
    char **h_aliases;      /* alias list */
    uint16_t h_addrtype;   /* host address type */
    uint16_t h_length;     /* length of address */
    char **h_addr_list; /* list of addresses from name server */
};

struct so_ainfo_t {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    struct so_addr_t *ai_addr;
    char *ai_canonname;
    struct so_ainfo_t *ai_next;
};

enum so_lvl_t {
    SOL_INTERFACE = 0xfffe,
    SOL_SOCKET = 0xffff,
};

enum so_opt_t {
    SO_REUSEADDR = 0x0004,
    SO_SNDBUF = 0x1001,
    SO_RCVBUF = 0x1002,
    SO_IERROR = 0x1003,
    SO_MAC = 0x1004,
    SO_LINK = 0x1005,
    SO_ERROR = 0x1009,
    SO_IP_NUM = 0x4002,
    SO_IP_TBL = 0x4003,
    SO_DNS_TBL = 0xB003,
};

static inline so_ret_t SOStartup(void) {
    return SOStartupEx(60000);
}
static inline so_ret_t SORecvFrom(
        so_fd_t socket, void *buffer, size_t buffer_size, int flags, 
        so_addr_t *addr) {
    return SOiRecvFrom(0, socket, buffer, buffer_size, flags, addr);
}
static inline so_ret_t SORecv(
        so_fd_t socket, void *buffer, size_t buffer_size, int flags) {
    return SOiRecvFrom(0, socket, buffer, buffer_size, flags, NULL);
}
static inline so_ret_t SOSendTo(
        so_fd_t socket, const void *buffer, size_t buffer_size, int flags, 
        const so_addr_t *addr) {
    return SOiSendTo(0, socket, buffer, buffer_size, flags, addr);
}
static inline so_ret_t SOSend(
        so_fd_t socket, const void *buffer, size_t buffer_size, int flags) {
    return SOiSendTo(0, socket, buffer, buffer_size, flags, NULL);
}

#endif /* _RVL_SO_H_ */