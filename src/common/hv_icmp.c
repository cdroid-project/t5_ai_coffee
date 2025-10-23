/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 17:42:15
 * @LastEditTime: 2025-10-23 14:50:33
 * @FilePath: /t5_ai_demo/src/common/hv_icmp.c
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "hv_icmp.h"
#include "stdint.h"
#include "sys/socket.h"
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef struct iphdr_s {
#if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t     ihl : 4; // ip header length
    uint8_t     version : 4;
#elif BYTE_ORDER == BIG_ENDIAN
    uint8_t     version : 4;
    uint8_t     ihl : 4;
#else
#error "BYTE_ORDER undefined!"
#endif
    uint8_t     tos; // type of service
    uint16_t    tot_len; // total length
    uint16_t    id;
    uint16_t    frag_off; // fragment offset
    uint8_t     ttl; // Time To Live
    uint8_t     protocol;
    uint16_t    check; // checksum
    uint32_t    saddr; // srcaddr
    uint32_t    daddr; // dstaddr
    /*The options start here.*/
} iphdr_t;


// sizeof(icmphdr_t) = 8
typedef struct icmphdr_s {
    uint8_t     type;   // message type
    uint8_t     code;   // type sub-code
    uint16_t    checksum;
    union {
        struct {
            uint16_t    id;
            uint16_t    sequence;
        } echo;
        uint32_t    gateway;
        struct {
            uint16_t    reserved;
            uint16_t    mtu;
        } frag;
    } un;
} icmphdr_t;


typedef struct icmp_s {
    uint8_t     icmp_type;
    uint8_t     icmp_code;
    uint16_t    icmp_cksum;
    union {
        uint8_t ih_pptr;
        struct in_addr ih_gwaddr;
        struct ih_idseq {
            uint16_t icd_id;
            uint16_t icd_seq;
        } ih_idseq;
        uint32_t    ih_void;

        struct ih_pmtu {
            uint16_t ipm_void;
            uint16_t ipm_nextmtu;
        } ih_pmtu;

        struct ih_rtradv {
            uint8_t irt_num_addrs;
            uint8_t irt_wpa;
            uint16_t irt_lifetime;
        } ih_rtradv;
    } icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime

    union {
        struct {
            uint32_t its_otime;
            uint32_t its_rtime;
            uint32_t its_ttime;
        } id_ts;
        /*
        struct {
            struct ip idi_ip;
        } id_ip;
        struct icmp_ra_addr id_radv;
        */
        uint32_t id_mask;
        uint8_t  id_data[1];
    } icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
} icmp_t;



static const char* sockaddr_ip(struct sockaddr_in* addr, char* ip, int len) {
    if (addr->sin_family == AF_INET) {
        return inet_ntop(AF_INET, &addr->sin_addr, ip, len);
    }

    return ip;
}


static unsigned int gettick_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


static unsigned long long gethrtime_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000000 + tv.tv_usec;
}

static  int ResolveAddr(const char* host, struct sockaddr_in* addr) {

    if (inet_pton(AF_INET, host, &addr->sin_addr) == 1) {
        addr->sin_family = AF_INET; // host is ipv4, so easy ;)
        return 0;
    }

    struct addrinfo* ais = NULL;
    int ret = getaddrinfo(host, NULL, NULL, &ais);
    if (ret != 0 || ais == NULL || ais->ai_addr == NULL || ais->ai_addrlen == 0) {
        printf("unknown host: %s err:%d:%s\n", host, ret, gai_strerror(ret));
        return ret;
    }
    struct addrinfo* pai = ais;
    while (pai != NULL) {
        if (pai->ai_family == AF_INET) break;
        pai = pai->ai_next;
    }
    if (pai == NULL) pai = ais;
    memcpy(addr, pai->ai_addr, pai->ai_addrlen);
    freeaddrinfo(ais);
    return 0;
}



static inline uint16_t checksum(uint8_t* buf, int len) {
    unsigned int sum = 0;
    uint16_t* ptr = (uint16_t*) buf;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len) {
        sum += *(uint8_t*) ptr;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t) (~sum);
};

#define PING_TIMEOUT    1000 // ms
int ping(const char* host, int cnt) {
    static uint16_t seq = 0;
    uint16_t pid16 = (uint16_t) getpid();
    char ip[64] = { 0 };
    uint32_t start_tick, end_tick;
    uint64_t start_hrtime, end_hrtime;
    int timeout = 0;
    int sendbytes = 64;
    char sendbuf[64];
    char recvbuf[128]; // iphdr + icmp = 84 at least
    icmp_t* icmp_req = (icmp_t*) sendbuf;
    iphdr_t* ipheader = (iphdr_t*) recvbuf;
    icmp_t* icmp_res;
    // ping stat
    int send_cnt = 0;
    int recv_cnt = 0;
    int ok_cnt = 0;
    float rtt, min_rtt, max_rtt, total_rtt;
    rtt = max_rtt = total_rtt = 0.0f;
    min_rtt = 1000000.0f;
    //min_rtt = MIN(rtt, min_rtt);
    //max_rtt = MAX(rtt, max_rtt);
    // gethostbyname -> socket -> setsockopt -> sendto -> recvfrom -> closesocket
    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    memset(&peeraddr, 0, addrlen);
    int ret = ResolveAddr(host, &peeraddr);
    if (ret != 0) return ret;
    sockaddr_ip(&peeraddr, ip, sizeof(ip));
    int sockfd = socket(peeraddr.sin_family, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        // perror("socket");
        if (errno == EPERM) {
            // fprintf(stderr, "please use root or sudo to create a raw socket.\n");
        }
        return -errno;
    }

    timeout = PING_TIMEOUT;
    {
        struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
        ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        if (ret < 0) {
            // perror("setsockopt");
            goto error;
        }
    }
    timeout = PING_TIMEOUT;
    {
        struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
        ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        if (ret < 0) {
            // perror("setsockopt");
            goto error;
        }
    }
#define ICMP_ECHOREPLY		0	/* Echo Reply			*/
#define ICMP_ECHO		8	/* Echo Request			*/
    icmp_req->icmp_type = ICMP_ECHO;
    icmp_req->icmp_code = 0;
    icmp_req->icmp_id = pid16;
    for (int i = 0; i < sendbytes - sizeof(icmphdr_t); ++i) {
        icmp_req->icmp_data[i] = i;
    }
    start_tick = gettick_ms();
    while (cnt-- > 0) {
        // NOTE: checksum
        icmp_req->icmp_seq = ++seq;
        icmp_req->icmp_cksum = 0;
        icmp_req->icmp_cksum = checksum((uint8_t*) icmp_req, sendbytes);
        start_hrtime = gethrtime_us();
        addrlen = sizeof(struct sockaddr_in);
        int nsend = sendto(sockfd, sendbuf, sendbytes, 0, (struct sockaddr*) &peeraddr, addrlen);
        if (nsend < 0) {
            // perror("sendto");
            continue;
        }
        ++send_cnt;
        addrlen = sizeof(peeraddr);
        int nrecv = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) &peeraddr, &addrlen);
        if (nrecv < 0) {
            // perror("recvfrom");
            continue;
        }
        ++recv_cnt;
        end_hrtime = gethrtime_us();
        // check valid
        uint8_t valid = 0;
        int iphdr_len = ipheader->ihl * 4;
        int icmp_len = nrecv - iphdr_len;
        if (icmp_len == sendbytes) {
            icmp_res = (icmp_t*) (recvbuf + ipheader->ihl * 4);
            if (icmp_res->icmp_type == ICMP_ECHOREPLY &&
                icmp_res->icmp_id == pid16 &&
                icmp_res->icmp_seq == seq) {
                valid = 1;
            }
        }

        if (valid == 0) {
            printf("recv invalid icmp packet!\r\n");
            continue;
        }
        rtt = (end_hrtime - start_hrtime) / 1000.0f;
        min_rtt = MIN(rtt, min_rtt);
        max_rtt = MAX(rtt, max_rtt);
        total_rtt += rtt;
        // printf("%d bytes from %s: icmp_seq=%u ttl=%u time=%.1f ms\r\n", icmp_len, ip, seq, ipheader->ttl, rtt);
        // fflush(stdout);
        ++ok_cnt;
        if (cnt > 0) sleep(1); // sleep a while, then agian
    }
    end_tick = gettick_ms();
    // printf("--- %s ping statistics ---\r\n", host);
    // printf("%d packets transmitted, %d received, %d%% packet loss, time %d ms\r\n",
    //     send_cnt, recv_cnt, (send_cnt-recv_cnt)*100/(send_cnt==0?1:send_cnt), end_tick-start_tick);
    // printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\f",
    //     min_rtt, total_rtt/(ok_cnt==0?1:ok_cnt), max_rtt);

    close(sockfd);
    return ok_cnt;
error:
    close(sockfd);
    return errno > 0 ? -errno : -1;
}
