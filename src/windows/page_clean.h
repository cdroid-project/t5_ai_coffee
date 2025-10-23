/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:12:13
 * @FilePath: /t5_ai_demo/src/windows/page_clean.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef __PAGE_CLEAN_H__
#define __PAGE_CLEAN_H__

#include <widget/textview.h>

#include <R.h>
#include <cdlog.h>
#include "page_base.h"
#include <widgetEx/recyclerview/recyclerview.h>
#include <widget/progressbar.h>
#include "wifi_adapter.h"

class PageClean: public PageBase {
private:
    ViewGroup   *mSelectGroup;
    ViewGroup   *mStepGroup;
    ViewGroup   *mRunningGroup;

    // select group
    TextView    *mSelectWaterTv;
    TextView    *mSelectCalcifyTv;
    View        *mCalCleanWarnImg;
    TextView    *mCalInfoTv;
    TextView    *mCalInfoTv2;

    // step group
    ViewGroup   *mStepGroup_1;
    ViewGroup   *mStepGroup_2;

    TextView    *mSteptitleTv_1;
    TextView    *mSteptitleTv_2;
    TextView    *mStepInfoTv_1;
    TextView    *mStepInfoTv_2;

    TextView    *mStepEnterTv;
    TextView    *mStepcancelTv;

    // running group
    TextView    *mRunningTitleTv;
    TextView    *mRunningInfoTv;
    // ProgressBar *mRunningProgress;
    View        *mRunningView;
private:
    int mPageStep;
    bool mIsSelectWater;

    ValueAnimator *mCleanAnim;
private:
    
    void btnClickListener(View& view);
public:
    PageClean(ViewGroup *wind_page_box,int pageType);
    ~PageClean();

    void initPageView()override;
    void initPageData()override;
    void updatePageData()override;
    void stopPageAction()override;

protected:
    void changePageStep();
};

#endif

