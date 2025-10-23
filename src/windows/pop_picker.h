/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:29:06
 * @FilePath: /t5_ai_demo/src/windows/pop_picker.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __POP_PICKER_H_
#define __POP_PICKER_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopPicker:public PopBase{

public:
    PopPicker(ViewGroup *wind_pop_box,View *gaussFromView,int type,appCb enterCb,appCb cencelCb);
    ~PopPicker();

private:
    void initPopView()override;
    void initPopData()override;

    void onBtnClickListener(View&v);

private:
    ViewGroup    *mViewGroup; 

    NumberPicker *mSelectModePicker;
    NumberPicker *mNumberPicker;
    NumberPicker *mNumberPicker2;

    TextView     *mTitleTv;
    TextView     *mPickerName1Tv;
    TextView     *mPickerName2Tv;
    TextView     *mPickerUnits1Tv;
    TextView     *mPickerUnits2Tv;

    ViewGroup    *msetBtnLayout;

};

#endif
