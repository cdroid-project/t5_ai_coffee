/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:30:00
 * @FilePath: /t5_ai_demo/src/windows/page_setting.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __PAGE_SETTING_H__
#define __PAGE_SETTING_H__

#include <widget/textview.h>

#include <R.h>
#include <cdlog.h>
#include "page_base.h"
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widget/switch.h>
#include <widget/seekbar.h>
#include "wifi_adapter.h"

class PageSetting: public PageBase,MessageHandler {
protected:
    enum {
        MSG_DELAY_DISCONN,      // 延迟 断开连接wifi
        MSG_DELAY_SCAN,         // 延迟 搜索wifi
    };

private:
    // card 1 group WIFI
    TextView    *mDisConnText;
    Switch      *mWifiSwitch;
    View        *mWifiSwLayout;
    ImageView   *mShuaxinIcon;

    // scrollView group
    ViewGroup   *mTimeGroup;
    ViewGroup   *mGrindModeGroup;
    ViewGroup   *mHotWaterGroup;
    ViewGroup   *mBindAppGroup;
    ViewGroup   *mVoiceGroup;

    TextView    *mTimeTv;
    TextView    *mGrindModeTv;
    TextView    *mHotWaterTv;
    TextView    *mBindAppTv;
    Switch      *mVoiceSwitch;
    
    // card 3 group 使用情况
    TextView    *mFrothTv;  // 打奶泡
    TextView    *mGrindTv;  // 研磨
    TextView    *mBrewTv;   // 冲煮

    // card 4 group 本机情况
    TextView    *mAppVersionTv;
    TextView    *mMcuVersionTv;
    TextView    *mCdroidVersionTv;

    TextView    *mUpdateTv;
    TextView    *mResetTv;

    ValueAnimator  *mWifishuaxinAnim;
    Animator::AnimatorListener mViewAnimListener;
private:
    
    void btnClickListener(View& view);
public:
    PageSetting(ViewGroup *wind_page_box);
    ~PageSetting();

    void initPageView()override;
    void initPageData()override;
    void updatePageData()override;
    void stopPageAction()override;
    void resetPageAction()override;

protected:
    void connViewClickCallback(int eventAction);
    void onTicker()override;
    void handleMessage(Message& message)override;
private:
    RecyclerView *mWifiRecyclerView;
    LinearLayoutManager *mLinearLyManager; 
    wifiRecycAdapter *mRecyclerAdapter; 
};

#endif

