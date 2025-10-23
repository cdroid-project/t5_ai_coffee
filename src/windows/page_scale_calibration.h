/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:30:55
 * @FilePath: /t5_ai_coffee/src/windows/page_scale_calibration.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __PAGE_SCALE_CALIBRATION_H__
#define __PAGE_SCALE_CALIBRATION_H__

#include <widget/textview.h>

#include <R.h>
#include <cdlog.h>
#include "page_base.h"

class PageScaleCalibration: public PageBase {
private:
    ViewGroup   *mSelectGroup;
    
    ViewGroup   *mStep1Group;
    ViewGroup   *mStep2Group;
    ViewGroup   *mStep3Group;
    ViewGroup   *mStep4Group;
    TextView    *mStep4Tv;

    TextView    *mWeightTv;

public:
    PageScaleCalibration(ViewGroup *wind_page_box);
    ~PageScaleCalibration();

    void initPageView()override;
    void initPageData()override;
    void updatePageData()override;
    void stopPageAction()override;

protected:

};

#endif

