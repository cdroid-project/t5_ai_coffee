/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 11:30:36
 * @LastEditTime: 2025-10-23 14:45:02
 * @FilePath: /t5_ai_coffee/src/protocol/t5_conn_mgr.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#ifndef __t5_conn_mgr_h__
#define __t5_conn_mgr_h__

#include "packet_buffer.h"
#include "uart_client.h"
#include "cmd_handler.h"
#include "data.h"

typedef std::function<void()> factoryCb;
#define g_T5Mgr T5Mgr::ins()
class T5Mgr : public EventHandler, public IHandler {
protected:
    T5Mgr();
    ~T5Mgr();

public:
    static T5Mgr *ins() {
        static T5Mgr stIns;
        return &stIns;
    }

    int init();
    // 发送给按键板
    void send2MCU();
    

protected:
    virtual int checkEvents();
    virtual int handleEvents();
    virtual void onCommDeal(IAck *ack);
private:
    IPacketBuffer   *mPacket;
    int64_t          mNextEventTime;
    int64_t          mLastSendTime;
    UartClient      *mUartMCU; // mcu
    int              mSetData; // 设置数据次数
    uchar            mSndData[UI2MCU::BUF_LEN]; // 发送缓冲区

    std::string      mT5RecvText;
    bool             mIsT5RecvEnd;
};



#endif
