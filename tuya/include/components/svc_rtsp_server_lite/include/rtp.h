#ifndef __RTP_H__
#define __RTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RTP_HDR_LEN 12
#define MAX_PAYLOAD_SIZE 1200

typedef enum {
    RTCP_TYPE_SR = 200,
    RTCP_TYPE_RR = 201,
    RTCP_TYPE_SDES = 202,
    RTCP_TYPE_BYE = 203,
    RTCP_TYPE_APP = 204,
} RTCP_TYPE_E;

typedef enum {
    RTP_CODEC_INVALID = 0,
    RTP_CODEC_H264,
    RTP_CODEC_H265,
    RTP_CODEC_JPEG,
    RTP_CODEC_PCM,
    RTP_CODEC_G711A,
    RTP_CODEC_G711U,
} RTP_CODEC_E;

typedef enum {
    RTCP_APP_SUBTYPE_NACK = 1,    //重传
} RTCP_APP_SUBTYPE_E;

typedef struct {
    uint32_t csrc_cnt:4;
    uint32_t extension:1;
    uint32_t padding:1;
    uint32_t version:2;

    uint32_t payload_type:7;
    uint32_t marker:1;

    uint16_t seq:16;  //initial value should be random
    uint32_t timestamp:32; //initial value should be random
    uint32_t ssrc:32; //should be random
    uint32_t csrc_list[15];
} rtp_header_t;

typedef struct {
    uint32_t subtype : 5;
    uint32_t padding:1;
    uint32_t version:2;

    uint32_t payload_type:8;
    uint32_t length:16;

} rtcp_header_t;


int rtp_pack_header(rtp_header_t* phdr, char* buf, int len);
int rtp_unpack_header(rtp_header_t* phdr, char* buf, int len);
int rtcp_pack_header(rtcp_header_t* phdr, char* buf, int len);
int rtcp_unpack_header(rtcp_header_t* phdr, char* buf, int len);


typedef int (*rtp_pkt_handler)(char* buf, int len, void* arg);

/**
* @brief pack a video/audio frame to multiple rtp packets
* @param codec           codec of the input frame
* @param frm_buf         frame data
* @param frm_len         frame data length in bytes
* @param ssrc
* @param rtp_ts
* @param pseq
* @param plist           list to hold the rtp packets(rtp_pkt_list_alloc/rtp_pkt_list_free)
*/
int rtp_pack_nal(RTP_CODEC_E codec, char* frm_buf, int frm_len, uint32_t ssrc, uint32_t rtp_ts,
        uint16_t *pseq, rtp_pkt_handler handler, void* arg);
#ifdef __cplusplus
} // extern "C"
#endif

#endif // __RTP_H__
