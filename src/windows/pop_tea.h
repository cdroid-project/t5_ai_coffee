/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:28:38
 * @FilePath: /t5_ai_demo/src/windows/pop_tea.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef _POP_TEA_H_
#define _POP_TEA_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopTea:public PopBase{
public:

public:
    PopTea(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb,popModeCb cencelCb);
    ~PopTea();

private:
    void initPopView()override;
    void initPopData()override;

    void setStepData();// 初始化 手冲咖啡 步骤数据
    void saveStepData();
    void setTipsData();

    void onBtnClickListener(View&v);
    void onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue);

protected:

    ViewGroup       *mPopGaussBox;  // 高斯模糊背景 

    NumberPicker    *mPopPicker;
    NumberPicker     *mPopStepPicker;

    TextView        *mTipsPowderTv;
    TextView        *mTipsPowderMaxTv;
    TextView        *mTipsWashTimesTv;
    TextView        *mTipsPressureTv;

    ImageView       *mPopResetAnim;
    ValueAnimator   *mResetAnimator;

    NumberPicker    *mPopPickerWater;
    NumberPicker    *mPopPickerFlowRate;
    NumberPicker    *mPopPickerTime;

    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;
protected:
    int mPickerSelectPos;
    int mStepSelectPos;
    TextView        *mStepSelectView;

    ExtractTeaDataStr  mTeaData;
};

#endif
