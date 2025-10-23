/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:02
 * @FilePath: /t5_ai_coffee/src/protocol/btn_mgr.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __BTN_MGR_h__
#define __BTN_MGR_h__

#include "core/looper.h"
#include "data.h"

typedef std::function<void()> factoryCb;
#define g_objBtnMgr BtnMgr::ins()
class BtnMgr : public EventHandler{
protected:
    BtnMgr();
    ~BtnMgr();

public:
    static BtnMgr *ins() {
        static BtnMgr stIns;
        return &stIns;
    }

    // // 产测
    void setFactoryCallback(factoryCb cb);

    // 产测指令
    void sendFactoryData();

    int init();

    void shutDown();

protected:
    virtual int checkEvents();
    virtual int handleEvents();

    void threadFunction();
private:
    bool             mIsRunning;
    bool             mIsFactory;
    bool             mIsShutDown;
    int64_t          mNextEventTime;

    uchar            mMcuType;           // 机型
};



#endif
