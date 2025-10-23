/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:12:22
 * @FilePath: /t5_ai_demo/src/windows/page_base.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef _PAGE_BASE_H_
#define _PAGE_BASE_H_

#include <view/viewgroup.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include "common.h"
#include "time_textview.h"

typedef enum {
    PAGE_NONE = -1,         // 无界面
    PAGE_HOME = 0,          // 主界面
    PAGE_SCREENSAVER,       // 屏保界面
    PAGE_KEYBOARD,          // 键盘页面
    PAGE_SETTING,           // 设置页面
    PAGE_CLEAN,             // 清洁页面
    PAGE_CLEAN_CALCIFY,     // 清洁页面（钙化清洁页面）
    PAGE_FACTORY,           // 产测页面
    PAGE_SCALE_CALIBRATION, // 电子秤校准页面
}pageType;

class PageBase{
public:
    typedef enum{
        TITLE_BAR_TIME      = 0x01,
        TITLE_BAR_DATE      = 0x02,
        TITLE_BAR_WEATHER   = 0x04,
        TITLE_BAR_WIFI      = 0x08,
        TITLE_BAR_CLEAN     = 0x10,
    }TitleBarStatusType;
protected:
    ViewGroup *mWindPageBox;
    ViewGroup *mPageLayout;

    ImageView *mTitleBarWeather;
    TextView  *mTitleBarTempText;
    TextView  *mTitleBarWeekText;
    TextView  *mToastView;
    TimeTextView  *mTitleBarTimeText;

    ImageView *mTitleBarWifi;
    ImageView *mTitleWash;
    ImageView *mTitleSetup;

    Animator::AnimatorListener mViewAnimListener;
private:
    Runnable   mTicker;
    int        mTickerTime;
    
protected:
    int mPageType;
    int64_t    mInitTime;
public:
    PageBase(ViewGroup *wind_page_box,const std::string resource,int page_type);
    virtual ~PageBase();

    ViewGroup *getPageLayout();

    void changeTitleBarStatus(uchar status); //改变标题栏状态

    void setPageType(int type);
    int  getPageType();

public:
    virtual void addPage();
    virtual void removePage();
    virtual void stopPageAction();
    virtual void resetPageAction();
    virtual void initPageView()=0;
    virtual void initPageData()=0;
    virtual void updatePageData()=0;
protected:
    void showToast(std::string showText,int translationX = 55);
    void setWifiStatus();
    virtual void onTicker();
private:
    void initPageBase(const std::string resource);
    void btnClickListener(View& view);
    void enterTestPage(View&v);
    void onBaseTicker();        // 定时任务
};

#endif
