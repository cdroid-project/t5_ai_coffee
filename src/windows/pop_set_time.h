/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:28:55
 * @FilePath: /t5_ai_demo/src/windows/pop_set_time.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __POP_SET_TIME_H_
#define __POP_SET_TIME_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopSetTime:public PopBase{

public:
    PopSetTime(ViewGroup *wind_pop_box,View *gaussFromView,appCb enterCb,appCb cencelCb);
    ~PopSetTime();

private:
    void initPopView()override;
    void initPopData()override;

    void onBtnClickListener(View&v);
    void onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue);

    void setMaxDay(int year,int mon);

    void timeUpdateFunc();
private:
    ViewGroup    *mViewGroup; 

    NumberPicker *mPickerYear;
    NumberPicker *mPickerMon;
    NumberPicker *mPickerDay;
    NumberPicker *mPickeHour;
    NumberPicker *mPickeMin;

    TextView     *mTipsText;
    ViewGroup    *msetBtnLayout;

    Runnable mUpdateTime;
};

#endif
