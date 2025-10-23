/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 16:55:43
 * @FilePath: /t5_ai_coffee/src/protocol/mcu_ui.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __mcu_ui_ack_h__
#define __mcu_ui_ack_h__

#include "ipacket_buffer.h"
#include "proto.h"

#include <crc.h>

typedef enum {
    BUF_SND_D0 = 0, // 帧头
    BUF_SND_D1,     
    BUF_SND_D2,     // 功能码
    BUF_SND_D3,     
    BUF_SND_D4,
    BUF_SND_D5,
    BUF_SND_D6,
    BUF_SND_D7,
    BUF_SND_D8,
    BUF_SND_D9,
    BUF_SND_D10,
    BUF_SND_D11,
    BUF_SND_D12,
    BUF_SND_D13,
    BUF_SND_D14,
    BUF_SND_D15, // (CRC16)
} EMUI2MCU;

typedef enum {
    BUF_RCV_D0 = 0, // 帧头
    BUF_RCV_D1,     // 功能码
    BUF_RCV_D2,     // 数据长度
    BUF_RCV_D3,     // 数据 0 
    BUF_RCV_D4,
    BUF_RCV_D5,
    BUF_RCV_D6,
    BUF_RCV_D7,
    BUF_RCV_D8,
    BUF_RCV_D9,
    BUF_RCV_D10,
    BUF_RCV_D11,
    BUF_RCV_D12,
    BUF_RCV_D13,
    BUF_RCV_D14,
    BUF_RCV_D15,

} EMMCU2UI;

class DataCheck {
public:
    DataCheck() {}
    unsigned char Sum(unsigned char *ptr, unsigned char len){
        unsigned char sum = 0;
        for(int i = 0; i < len; i++){
            sum += ptr[i];
        }
        return sum;
    }
};




////////////////////////////////////////////////////////////////////////////////////////////////
// 串口通讯（与电控板）

// UI -> MCU
class UI2MCU : public IAsk, public DataCheck {
public:
    constexpr static uchar BUF_LEN      = 250;
    constexpr static uchar HEAD_BYTE    = 0x5A; // 帧头
    constexpr static uchar END_BYTE     = 0x2A; // 帧尾
public:
    UI2MCU() {}
    UI2MCU(BuffData *buf) { parse(buf); }
    void parse(BuffData *buf) {           // override
        mBf                  = buf;
        if (mBf->len == 0) {
            mBf->len             = BUF_LEN;
            mBf->buf[BUF_SND_D0] = HEAD_BYTE;
        }
    }

protected:

public:
    int getCMD() { return mBf->buf[BUF_SND_D2]; } // override
    void setCMD(uchar cmd) { mBf->buf[BUF_SND_D2] = cmd; }
    void setData(uchar pos, uchar data) { mBf->buf[pos] = data; }
    void setLen(uchar len) { mBf->len = len;}
    void checkcode() { 
        int len = mBf->buf[1]+4;
        mBf->buf[len - 2] = Sum(mBf->buf, len - 2); 
        mBf->buf[len - 1] = 0x2A; 
        mBf->len = len;
    }
};

// MCU -> UI
class MCU2UI : public IAck, public DataCheck {
public:
    constexpr static uchar BUF_LEN      = 64;
    constexpr static uchar HEAD_BYTE_ACTIVE    = 0x5A; // 帧头 主机
    constexpr static uchar HEAD_BYTE    = 0x5B; // 帧头
    constexpr static uchar END_BYTE     = 0x2A; // 帧尾
public:
    MCU2UI() {}
    MCU2UI(BuffData *bf) { parse(bf); }
    void parse(BuffData *bf) {
        mDlen = bf->len;
        mPlen = &bf->len;
        mBuf  = bf->buf;
    }
    int add(uchar *bf, int len) {
        int  i, j, rlen = 0;
        bool find_head = false;
        while (mDlen < BUF_LEN && rlen < len) {
            // add data
            if (BUF_LEN - mDlen >= len - rlen) {
                memcpy(mBuf + mDlen, bf + rlen, len - rlen);
                mDlen += len - rlen;
                rlen = len;
            } else {
                memcpy(mBuf + mDlen, bf + rlen, BUF_LEN - mDlen);
                rlen += BUF_LEN - mDlen;
                mDlen = BUF_LEN;
            }
            // find head
            if (!find_head && mDlen > sizeof(HEAD_BYTE)) {
                for (i = 0, j = -1; i < mDlen; i++) {
                    if (mBuf[i] == HEAD_BYTE || mBuf[i] == HEAD_BYTE_ACTIVE) {
                        // if (i < mDlen - 1 && mBuf[i + 1] == HEAD_BYTE[1]) {
                            // if (i < mDlen - 2 && mBuf[i + 2] == HEAD_BYTE[2]) {
                                j         = i;
                                find_head = true;
                                break;
                            // } else {
                            //     if (i == mDlen - 1) { j = i; }
                            // }
                        // } else {
                        //     if (i == mDlen - 1) { j = i; }
                        // }
                    }
                }
                if (j == -1) {
                    mDlen = 0;
                } else if (j > 0) {
                    memmove(mBuf, mBuf + j, mDlen - j);
                    mDlen -= j;
                }                
            }
            *mPlen = mDlen;
        }

        if (find_head && mDlen > 4) {
            uint16_t RealLen = mBuf[1] + 4;
            if (mDlen > RealLen) {
                rlen -= (mDlen - RealLen);
                rlen = rlen < 0 ? 0 : rlen;
            }
        }
        return rlen;
    }
    bool complete() {       
        if ((mBuf[0] == HEAD_BYTE || mBuf[0] == HEAD_BYTE_ACTIVE) && mDlen > 4 && mDlen >= mBuf[1]+4 ) {
            return true;
        }
        return false;
    }
    bool check() { 
        return Sum(mBuf,mBuf[1]+2) && mBuf[mBuf[1]+3] == END_BYTE; 
    }

protected:
    virtual int getType() { return BT_MCU; }

public:
    int getCMD() { return mBuf[BUF_SND_D2]; }
    int getData(int pos) {
        if (pos >= 0 && pos < BUF_LEN - 1) { return mBuf[pos]; }
        return 0;
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////
// IIC 通讯 按键

// UI -> IIC
class UI2IIC : public IAsk, public DataCheck {
public:
    constexpr static uchar BUF_LEN      = 8;
    constexpr static uchar HEAD_BYTE    = 0xAA; // 帧头
    constexpr static uchar END_BYTE     = 0xBB; // 帧尾
public:
    UI2IIC() {}
    UI2IIC(BuffData *buf) { parse(buf); }
    void parse(BuffData *buf) {           // override
        mBf                  = buf;
        if (mBf->len == 0) {
            mBf->len             = BUF_LEN;
            mBf->buf[BUF_SND_D0] = HEAD_BYTE;
        }
    }

protected:
    virtual int getCMD() { return 0; } // override

public:
    void setData(uchar pos, uchar data) { mBf->buf[pos] = data; }
    void setLen(uchar len) { mBf->len = len;}
    void checkcode() { mBf->buf[BUF_SND_D4] = END_BYTE; }
};

// IIC -> UI
class IIC2UI : public IAck, public DataCheck {
public:
    constexpr static uchar BUF_LEN      = 8;
    constexpr static uchar HEAD_BYTE    = 0xAA; // 帧头
    constexpr static uchar END_BYTE     = 0xBB; // 帧尾
public:
    IIC2UI() {}
    IIC2UI(BuffData *bf) { parse(bf); }
    void parse(BuffData *bf) {
        mDlen = bf->len;
        mPlen = &bf->len;
        mBuf  = bf->buf;
    }
    int add(uchar *bf, int len) {
        int  i, j, rlen = 0;
        bool find_head = false;
        while (mDlen < BUF_LEN && rlen < len) {
            // add data
            if (BUF_LEN - mDlen >= len - rlen) {
                memcpy(mBuf + mDlen, bf + rlen, len - rlen);
                mDlen += len - rlen;
                rlen = len;
            } else {
                memcpy(mBuf + mDlen, bf + rlen, BUF_LEN - mDlen);
                rlen += BUF_LEN - mDlen;
                mDlen = BUF_LEN;
            }
            // find head
            if (!find_head && mDlen > sizeof(HEAD_BYTE)) {
                for (i = 0, j = -1; i < mDlen; i++) {
                    if (mBuf[i] == HEAD_BYTE) {
                        j         = i;
                        find_head = true;
                        break;
                    }
                }
                if (j == -1) {
                    mDlen = 0;
                } else if (j > 0) {
                    memmove(mBuf, mBuf + j, mDlen - j);
                    mDlen -= j;
                }                
            }
            *mPlen = mDlen;
        }

        // if (find_head && mDlen > 4) {
        //     uint16_t RealLen = mBuf[1] + 4;
        //     if (mDlen > RealLen) {
        //         rlen -= (mDlen - RealLen);
        //         rlen = rlen < 0 ? 0 : rlen;
        //     }
        // }
        return rlen;
    }
    bool complete() {       
        if ((mBuf[0] == HEAD_BYTE) && mDlen >= 5 ) {
            return true;
        }
        return false;
    }
    bool check() { 
        return mBuf[BUF_RCV_D4] == END_BYTE; 
    }

protected:
    virtual int getType() { return BT_BTN; }
    virtual int getCMD() { return mBuf[1]; }

public:
    int getData(int pos) {
        if (pos >= 0 && pos < BUF_LEN - 1) { return mBuf[pos]; }
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
// AI -> MCU
class MCU2AI : public IAsk, public DataCheck {
public:
    constexpr static int BUF_LEN      = 3096;
public:
    MCU2AI() {}
    MCU2AI(BuffData *buf) { parse(buf); }
    void parse(BuffData *buf) {           // override
        mBf                  = buf;
        if (mBf->len == 0) {
            mBf->len             = BUF_LEN;
            // mBf->buf[BUF_SND_D0] = HEAD_BYTE;
        }
    }

protected:
    virtual int getCMD() { return 0; } // override

public:
    void setCMD(uchar cmd) { mBf->buf[BUF_SND_D2] = cmd; }
    void setData(uchar pos, uchar data) { mBf->buf[pos] = data; }
    void setLen(uchar len) { mBf->len = len;}
    void checkcode() { 
        int len = mBf->buf[1]+4;
        mBf->buf[len - 2] = Sum(mBf->buf, len - 2); 
        mBf->buf[len - 1] = 0x2A; 
        mBf->len = len;
    }
};

// MCU -> UI
class AI2MCU : public IAck, public DataCheck {
public:
    constexpr static int BUF_LEN  = 3096;
    uchar HEAD[2] = {0xcc, 0xee};
    int  recvLen = 0;
public:
    AI2MCU() {}
    AI2MCU(BuffData *bf) { parse(bf); }
    void parse(BuffData *bf) {
        mDlen = bf->len;
        mPlen = &bf->len;
        mBuf  = bf->buf;
    }
    int add(uchar *bf, int len) {
        int  i, j, rlen = 0;
        bool find_head = false;
        while (mDlen < BUF_LEN && rlen < len) {
            // add data
            if (BUF_LEN - mDlen >= len - rlen) {
                memcpy(mBuf + mDlen, bf + rlen, len - rlen);
                mDlen += len - rlen;
                rlen = len;
            } else {
                memcpy(mBuf + mDlen, bf + rlen, BUF_LEN - mDlen);
                rlen += BUF_LEN - mDlen;
                mDlen = BUF_LEN;
            }
            // find head
            if (!find_head && mDlen > sizeof(HEAD)-1) {
                for (i = 0,j=-1; i + len <= mDlen; i++) {
                    if (memcmp(mBuf + i, HEAD, sizeof(HEAD)) == 0) {
                        find_head = true;
                        j = i;
                        break;
                    }
                    if (i + len == mDlen)j = i;
                }
                if (j == -1) {
                    mDlen = 0;
                } else if (j > 0) {
                    memmove(mBuf, mBuf + j, mDlen - j);
                    mDlen -= j;
                }                
            }
            *mPlen = mDlen;
        }

        if (find_head && mDlen > 4) {
            uint16_t RealLen = mBuf[2] + mBuf[3]*256 + 6;
            if (mDlen > RealLen) {
                rlen -= (mDlen - RealLen);
                rlen = rlen < 0 ? 0 : rlen;
            }
        }
        return rlen;
    }
    bool complete() {
        if( (mDlen >= mBuf[2] + mBuf[3]*256 + 6)){
            return true;
        }
        return false;
    }
    bool check() { 
        return true;
    }

protected:
    virtual int getType() { return BT_T5; }
    virtual int getCMD() { return mBuf[1]; }

public:
    int getData(int pos) {
        if (pos >= 0 && pos < BUF_LEN - 1) { return mBuf[pos]; }
        return 0;
    }
};

#endif
