/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 11:30:36
 * @LastEditTime: 2025-10-23 14:44:54
 * @FilePath: /t5_ai_demo/src/protocol/t5_conn_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#include "t5_conn_mgr.h"

#include <core/app.h>
#include <fstream>

#include "conf_mgr.h"
#include "data.h"
#include "wind_mgr.h"
#include "btn_mgr.h"
#include "tcp_conn_mgr.h"
#define TICK_TIME 50 // tick触发时间（毫秒）

//////////////////////////////////////////////////////////////////

T5Mgr::T5Mgr() {
    mPacket            = new SDHWPacketBuffer(BT_T5);
    mUartMCU           = 0;
    mNextEventTime     = 0;
    mLastSendTime      = 0;
    mSetData           = 0;
    mIsT5RecvEnd       = true;

    g_objHandler->addHandler(BT_T5, this);
}

T5Mgr::~T5Mgr() {
    __del(mUartMCU);
    App::getInstance().removeEventHandler(this);
}

int T5Mgr::init() {
    UartOpenReq ss;

    snprintf(ss.serialPort, sizeof(ss.serialPort), "/dev/ttyS5");
    ss.speed     = 115200;
    ss.flow_ctrl = 0;
    ss.databits  = 8;
    ss.stopbits  = 1;
    ss.parity    = 'N';

    mUartMCU = new UartClient(mPacket, BT_T5, ss, "10.0.0.55", 1616, 0);
    mUartMCU->init();

    // 启动延迟一会后开始发包
    mNextEventTime = SystemClock::uptimeMillis() + TICK_TIME * 10;
    App::getInstance().addEventHandler(this);

    return 0;
}

int T5Mgr::checkEvents() {
    int64_t curr_tick = SystemClock::uptimeMillis();
    if (curr_tick >= mNextEventTime) {
        mNextEventTime = curr_tick + TICK_TIME;
        return 1;
    }
    return 0;
}

int T5Mgr::handleEvents() {
    int64_t now_tick = SystemClock::uptimeMillis();

    if (mUartMCU) mUartMCU->onTick();

    return 1;
}

void T5Mgr::send2MCU() {
    return;
    BuffData *bd = mPacket->obtain();
    UI2MCU   snd(bd);

    for (int i = 0; i < mSetData; i++) { snd.setData(i+BUF_SND_D1, mSndData[i]);}

    if(snd.getCMD() == CMD_ERROR){
        snd.setData(0,0x5B);
    }
    snd.setLen(mSetData+4);
    snd.checkcode();    // 修改检验位
    // LOGE("send Beep = %d",statusByte & 0x01);
    LOG(INFO) << "send to mcu. bytes=" << hexstr(bd->buf, bd->len);
    mUartMCU->send(bd);
    
    mSetData      = 0;
    mLastSendTime = SystemClock::uptimeMillis();
}

void T5Mgr::onCommDeal(IAck *ack) {
    LOG(INFO) << "hex str=" << hexstr(ack->mBuf, ack->mDlen);
    int64_t now_tick = SystemClock::uptimeMillis();

    // 将 uchar * 转换为 std::string
    std::string utf8_data(reinterpret_cast<char*>(ack->mBuf + 4), ack->mBuf[2] + ack->mBuf[3]*256);

    mT5RecvText += utf8_data;

    if(mT5RecvText.find("nlgEnd_cdroid") != std::string::npos) {
        // 输出结果
        std::cout << "UTF-8 字符串: " << mT5RecvText << std::endl;

        std::string result;
        for (char c : mT5RecvText) {
            if (c != '\0') {
                result += c; // 只添加非 null 字符
            }
        }
        mT5RecvText = result;
        // 输出处理后的结果
        std::cout << "处理后的字符串: " << mT5RecvText << std::endl;
        // 提取 JSON 数据
        g_appData.aiText = mT5RecvText;
        std::string json_data = extractJson(mT5RecvText);

        // 检查提取的 JSON 数据是否有效
        if (!json_data.empty()) {
            if(!convertStringToJson(json_data,g_appData.aiJsonText)){
                g_appData.aiJsonText.clear();
            }
            std::cout << "提取的 JSON 数据: " << json_data << std::endl;
        } else {
            g_appData.aiJsonText.clear();
            std::cout << "没有有效的 JSON 数据." << std::endl;
        }
        
        g_appData.statusChangeFlag |= CS_AI_DATA_CHANGE;
        g_windMgr->updateDate();

        mT5RecvText.clear();
    }

    
    return;
}
