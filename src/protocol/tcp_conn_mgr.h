/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:43:47
 * @FilePath: /t5_ai_coffee/src/protocol/tcp_conn_mgr.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __tcp_conn_mgr_h__
#define __tcp_conn_mgr_h__

#include "packet_buffer.h"
#include "uart_client.h"
#include "cmd_handler.h"
#include "data.h"

class TcpConnMgr : public EventHandler, public IHandler {
protected:
    TcpConnMgr();
    ~TcpConnMgr();

public:
    static TcpConnMgr *ins() {
        static TcpConnMgr stIns;
        return &stIns;
    }

    int init();
    // 发送给按键板
    void send2MCU(IAck *ack);

protected:
    virtual int checkEvents();
    virtual int handleEvents();
    virtual void onCommDeal(IAck *ack);
private:
    bool             mIsFactory;
    int64_t          mLastStatusTime;
    IPacketBuffer   *mPacket;
    int64_t          mNextEventTime;
    int64_t          mLastSendTime;
    int64_t          mLastWarnTime;
    int64_t          mLastBeepTime;
    UartClient      *mUartMCU; // mcu
    int              mSetData; // 设置数据次数
    int              commErrorCount;
    uchar            mSndData[UI2MCU::BUF_LEN]; // 发送缓冲区    

    uchar            mMcuType;           // 机型

    uchar           *mSendOTAData;
    int              mAllOTALen;
    int              mSendOTALen;
    int              mSendOTACount;
    uchar            mOTADataSum;
};

extern TcpConnMgr *g_TcpConnMgr;

#endif
