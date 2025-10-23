/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:14
 * @FilePath: /t5_ai_coffee/src/protocol/cmd_handler.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __cmd_handler_h__
#define __cmd_handler_h__

#include "ipacket_buffer.h"

///////////////////////////////////////////////////////////////////////////

#define __make_short(a, b) (ushort(a) << 8 | (b))
#define __bind_hander(F) std::bind(F, this, std::placeholders::_1)

typedef std::function<void(IAck *)> cmd_callback;

class IHandler {
public:
    virtual void onRecv(IAck *ack) {
        auto it = mCMDHandlers.find(ack->getType());
        if (it == mCMDHandlers.end()) {
            onCommDeal(ack);
            return;
        }

        it->second(ack);
    }
    virtual void regHandler(int cmd, cmd_callback h) {
        auto it = mCMDHandlers.find(cmd);
        if (it != mCMDHandlers.end()) {
            LOGE("cmd handler already reg. cmd=%x", cmd);
            return;
        }

        LOGD("cmd handler reg. cmd=%x handle=%p", cmd, h);
        mCMDHandlers[cmd] = h;
    }
    virtual void onCommDeal(IAck *ack){
        LOGE("cmd not deal. cmd=%x", ack->getType());
    }

protected:
    std::map<int, cmd_callback> mCMDHandlers; // 协议处理接口
};

///////////////////////////////////////////////////////////////////////////////

class CHandlerManager {
    typedef std::vector<IHandler *> handlers;
public:
    static CHandlerManager *ins() {
        static CHandlerManager stIns;
        return &stIns;
    }

    // 需要立即响应的处理
    bool addHandler(int cmd, IHandler *hd);
    void removeHandler(IHandler *hd);
    // 消息回调
    void onCommand(IAck *ack);

protected:
    CHandlerManager();

private:
    std::map<int, handlers>   mHandlers;
};

extern CHandlerManager *g_objHandler;

#endif
