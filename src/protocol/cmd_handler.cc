/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:09
 * @FilePath: /t5_ai_demo/src/protocol/cmd_handler.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "cmd_handler.h"
#include "proto.h"

#include <wind_mgr.h>

#define RECV_TIMEOUT_TIME 3000 // 接收数据超时警告

CHandlerManager *g_objHandler = CHandlerManager::ins();

CHandlerManager::CHandlerManager() {   
}

void CHandlerManager::onCommand(IAck *ack) {
    LOG(VERBOSE) << "type=" << std::hex << ack->getType();

    auto factoryIt = mHandlers.find(BT_FACTORY);
    if(factoryIt != mHandlers.end() && !factoryIt->second.empty()){
        
    }

    auto it = mHandlers.find(ack->getType());
    if (it == mHandlers.end() || it->second.empty()) {
        LOGW("command not deal. cmd(0x%x)", ack->getType());
        return;
    }
    for (IHandler *hd : it->second) {
        hd->onRecv(ack);
    }
}

bool CHandlerManager::addHandler(int cmd, IHandler *hd) {
    auto it = mHandlers.find(cmd);
    if (it == mHandlers.end()) {
        mHandlers[cmd].push_back(hd);
        return true;
    }
    auto it_find = std::find(it->second.begin(), it->second.end(), hd);
    if (it_find != it->second.end()){    
        LOGE("handler already in cmd. cmd=%d handler=%p", cmd, hd);
        return false;
    }
    mHandlers[cmd].push_back(hd);
    return true;
}

void CHandlerManager::removeHandler(IHandler *hd) {
    for (auto itm = mHandlers.begin(); itm != mHandlers.end();){
        auto it_find = std::find(itm->second.begin(), itm->second.end(), hd);
        if (it_find != itm->second.end()) {
            LOGD("cmd handler rem. cmd=%d handle=%p", itm->first, *it_find);
            itm->second.erase(it_find);
            if (itm->second.empty()) {
                itm = mHandlers.erase(itm);
                continue;
            }
        }
        itm++;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
