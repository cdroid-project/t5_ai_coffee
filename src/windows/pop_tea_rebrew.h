/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:28:46
 * @FilePath: /t5_ai_demo/src/windows/pop_tea_rebrew.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef _POP_TEA_REBREW_H_
#define _POP_TEA_REBREW_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopTeaRebrew:public PopBase{
public:

public:
    PopTeaRebrew(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,popModeCb enterCb,popModeCb cencelCb);
    ~PopTeaRebrew();

private:
    void initPopView()override;
    void initPopData()override;

    void setPopData();
    void onBtnClickListener(View&v);
    void onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue);

protected:

    ViewGroup       *mPopGaussBox;  // 高斯模糊背景 

    NumberPicker    *mPopPicker;

    ImageView       *mPopResetAnim;
    ValueAnimator   *mResetAnimator;

    NumberPicker    *mPopPickerWater;
    NumberPicker    *mPopPickerFlowRate;
    NumberPicker    *mPopPickerTime;
    NumberPicker    *mPopPickerTemp;

    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;
protected:
    int mPickerSelectPos;
    TextView        *mStepSelectView;

    ExtractTeaDataStr mTeaData;

    Runnable         mSetPlotDataRun;
};

#endif
