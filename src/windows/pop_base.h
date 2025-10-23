/*
 * @Author: gezilong
 * @Email: ioremap@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:40:15
 * @FilePath: /t5_ai_demo/src/windows/pop_base.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by gezilong, All Rights Reserved. 
 * 
**/

#ifndef _POP_BASE_H_
#define _POP_BASE_H_

#include <view/viewgroup.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include "common.h"
// #include "R.h"
// #include "time_textview.h"

typedef enum{
    POP_TYPE_MASTER_ESPRESSO,   // 大师浓缩
    POP_TYPE_HAND_WASH,         // 手冲咖啡
    POP_TYPE_TEA,               // 萃茶
    POP_TYPE_TEA_REBREW_SET,    // 萃茶 再沏的参数设置
    POP_TYPE_TEA_EXT_TIPS,      // 萃茶 冲泡
    POP_TYPE_TEA_REBREW_TIPS,   // 萃茶 再沏
    POP_TYPE_TEA_CANCEL_WORK,   // 萃茶 是否取消工作
    POP_OTA,                    // OTA弹窗
    POP_SET_TIME,               // 设置时间弹窗
    POP_HOT_WATER,              // 热水设置
    POP_STEAM,                  // 蒸汽设置
    POP_BEAN_GRIND,             // 磨豆设置
    POP_BIND_APP,               // 绑定设备
    POP_UNBIND_APP,             // 解绑设备
    POP_RESTORE_FACTORY,        // 恢复出厂设置
    POP_STOP_CLEAN,             // 停止清洁
    POP_GET_AUTH,               // 获取授权码
    POP_WARN,                   // 警告
}popType;

typedef std::function<void(int seletePos)> popModeCb;
typedef std::function<void()> appCb;

class PopBase{
protected:
    ViewGroup *mWindPopBox;
    ViewGroup *mPopLayout;
    
    popModeCb mModeEnterCallback;
    popModeCb mModeCancelCallback;

    appCb mEnterCallback;
    appCb mCancelCallback;
protected:
    int mPopType;
  
public:
    PopBase(ViewGroup *wind_pop_box,const std::string resource,int pop_type,popModeCb enterCb = nullptr,popModeCb cencelCb = nullptr);
    PopBase(ViewGroup *wind_pop_box,const std::string resource,int pop_type,appCb     enterCb = nullptr,appCb     cencelCb = nullptr);
    virtual ~PopBase();

    ViewGroup *getPopLayout();

    void setPopType(int type);
    int  getPopType();

    void cancelCallback();
    void enterCallback();
public:
    virtual void addPop();
    virtual void removePop();
    virtual void updatePageData();
    virtual void initPopView()=0;
    virtual void initPopData()=0;
private:
    void initPopBase(const std::string resource);
};

#endif
