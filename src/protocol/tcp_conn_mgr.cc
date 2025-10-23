/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:43:51
 * @FilePath: /t5_ai_demo/src/protocol/tcp_conn_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "tcp_conn_mgr.h"

#include <core/app.h>
#include <fstream>

#include "conf_mgr.h"
#include "data.h"
#include "wind_mgr.h"
#include "btn_mgr.h"
#define TICK_TIME 50 // tick触发时间（毫秒）

//////////////////////////////////////////////////////////////////

TcpConnMgr *g_TcpConnMgr = nullptr;
TcpConnMgr::TcpConnMgr() {
    mPacket            = new SDHWPacketBuffer(BT_MCU);
    mUartMCU           = 0;
    mNextEventTime     = 0;
    mLastSendTime      = 0;
    mLastStatusTime    = 0;
    mMcuType           = 0;
    mLastWarnTime      = 0;
    mLastBeepTime      = 0;
    commErrorCount     = 0;
    mSendOTACount      = 0;
    mSendOTALen        = 0;
    mIsFactory         = false;
    mSendOTAData       = nullptr;

    g_objHandler->addHandler(BT_TCP, this);
}

TcpConnMgr::~TcpConnMgr() {
    __del(mUartMCU);
    App::getInstance().removeEventHandler(this);
}

int TcpConnMgr::init() {
    UartOpenReq ss;

    snprintf(ss.serialPort, sizeof(ss.serialPort), "/dev/ttyS3");
    ss.speed     = 115200;
    ss.flow_ctrl = 0;
    ss.databits  = 8;
    ss.stopbits  = 1;
    ss.parity    = 'N';

    mLastWarnTime = SystemClock::uptimeMillis();
    mUartMCU = new UartClient(mPacket, BT_TCP, ss, "192.168.0.110", 1616, 0);
    mUartMCU->init();

    // 启动延迟一会后开始发包
    mNextEventTime = SystemClock::uptimeMillis() + TICK_TIME * 10;
    App::getInstance().addEventHandler(this);

    return 0;
}

int TcpConnMgr::checkEvents() {
    int64_t curr_tick = SystemClock::uptimeMillis();
    if (curr_tick >= mNextEventTime) {
        mNextEventTime = curr_tick + TICK_TIME;
        return 1;
    }
    return 0;
}

int TcpConnMgr::handleEvents() {
    int64_t now_tick = SystemClock::uptimeMillis();

    if (mUartMCU) mUartMCU->onTick();

    // else if(now_tick - mLastSendTime >= 100){ send2MCU(MODBUS_CMD_READ_ALL); }

    return 1;
}

void TcpConnMgr::send2MCU(IAck *ack) {
    BuffData *bd = mPacket->obtain();
    UI2MCU   snd(bd);

    for (int i = 0; i < ack->mDlen; i++) { snd.setData(i, ack->getData(i));}

    snd.setLen(ack->mDlen);
    snd.checkcode();    // 修改检验位

    LOG(INFO) << "send to mcu. bytes=" << hexstr(bd->buf, bd->len);
    mUartMCU->send(bd);
    
    mLastSendTime = SystemClock::uptimeMillis();
}

void TcpConnMgr::onCommDeal(IAck *ack) {
    LOG(INFO) << "hex str=" << hexstr(ack->mBuf, ack->mDlen);
    if(mIsFactory) return;
    int64_t now_tick = SystemClock::uptimeMillis();
    
}
