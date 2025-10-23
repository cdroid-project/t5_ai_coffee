/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:28:59
 * @FilePath: /t5_ai_coffee/src/windows/pop_set_time.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "pop_set_time.h"
#include "project.h"
#include "wind_mgr.h"
#include "gaussfilterdrawable.h"
PopSetTime::PopSetTime(ViewGroup *wind_pop_box,View *gaussFromView,appCb enterCb,appCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_set_time",POP_SET_TIME,enterCb,cencelCb){

    initPopView();
    initPopData();
    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
}

PopSetTime::~PopSetTime(){
    mWindPopBox->removeCallbacks(mUpdateTime);

}

void PopSetTime::initPopView(){
    mPickerYear = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_time_year_picker);
    mPickerMon = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_time_mon_picker);
    mPickerDay = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_time_day_picker);
    mPickeHour = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_time_hour_picker);
    mPickeMin = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_time_min_picker);

    // mTipsText = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::set_time_tips);
    // msetBtnLayout = (ViewGroup *)mPopLayout->findViewById(t5_ai_coffee::R::id::set_btn_layout);

    mPickerMon->setFormatter([this](int value){ return fillLength(value, 2); });
    mPickerDay->setFormatter([this](int value){ return fillLength(value, 2); });
    mPickeHour->setFormatter([this](int value){ return fillLength(value, 2); });
    mPickeMin->setFormatter([this](int value){ return fillLength(value, 2); });

    mPickerYear->setOnValueChangedListener(std::bind(&PopSetTime::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    mPickerMon->setOnValueChangedListener(std::bind(&PopSetTime::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    auto clickFunc = std::bind(&PopSetTime::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter)->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel)->setOnClickListener(clickFunc);
    mPopLayout->setOnClickListener(clickFunc);
    mPopLayout->setSoundEffectsEnabled(false);
}

void PopSetTime::initPopData(){
    mUpdateTime = [this](){
        std::time_t now_tick = std::time(NULL);
        std::tm tm_now = *localtime(&now_tick);
        timeUpdateFunc();
        mWindPopBox->postDelayed(mUpdateTime,1000);
    };
    if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN) mWindPopBox->postDelayed(mUpdateTime,1000);
    timeUpdateFunc();
}

void PopSetTime::onBtnClickListener(View&v){
    switch (v.getId()){
    case t5_ai_coffee::R::id::pop_cancel:
        g_windMgr->dealClosePopPage();
        break;
    case t5_ai_coffee::R::id::pop_enter:
        timeSet(mPickerYear->getValue(),
                mPickerMon->getValue(),
                mPickerDay->getValue(),
                mPickeHour->getValue(),
                mPickeMin->getValue(),
                getTodayTimeSec());
        g_windMgr->dealClosePopPage();
        break;

    default:
        break;
    }
}

void PopSetTime::onPickerValueChangeListener(NumberPicker& picker,int num_old,int num_new){
    switch(picker.getId()){
        case t5_ai_coffee::R::id::pop_time_year_picker:
        case t5_ai_coffee::R::id::pop_time_mon_picker:
            if(g_appData.tuyaWifiStatus != PRO_STATE_TUYA_GB_CLOUD_CONN){
                setMaxDay(mPickerYear->getValue(),mPickerMon->getValue());  
                break;
            }     
    }
}

//通过年 月，来设置 最大的日期（如 2月份只有29或28天）
void PopSetTime::setMaxDay(int year,int mon){
    std::tm timeinfo = {0};
    timeinfo.tm_year = year - 1900; // 年份从1900开始
    timeinfo.tm_mon = mon; // 月份从0开始，所以减去1

    std::time_t time = std::mktime(&timeinfo);

    int maxDays = std::localtime(&time)->tm_mday;
    mPickerDay->setMaxValue(maxDays);
}

void PopSetTime::timeUpdateFunc(){

    std::time_t now_tick = std::time(NULL);
    std::tm tm_now = *localtime(&now_tick);

    if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
        mPickerYear->setMaxValue(tm_now.tm_year + 1900);
        mPickerMon->setMaxValue(tm_now.tm_mon + 1);
        mPickeHour->setMaxValue(tm_now.tm_hour);
        mPickeMin->setMaxValue(tm_now.tm_min);
        mPickerDay->setMaxValue(tm_now.tm_mday);

        mPickerYear->setMinValue(tm_now.tm_year + 1900);
        mPickerMon->setMinValue(tm_now.tm_mon + 1);
        mPickeHour->setMinValue(tm_now.tm_hour);
        mPickeMin->setMinValue(tm_now.tm_min);
        mPickerDay->setMinValue(tm_now.tm_mday);

        mPickerYear->getSelectedText()->setActivated(false);
        mPickerMon->getSelectedText()->setActivated(false);
        mPickerDay->getSelectedText()->setActivated(false);
        mPickeHour->getSelectedText()->setActivated(false);
        mPickeMin->getSelectedText()->setActivated(false);

        // mTipsText->setVisibility(View::VISIBLE);
        // msetBtnLayout->setVisibility(View::GONE);

    }else{
        mPickerYear->setMaxValue(2037);
        mPickerMon->setMaxValue(12); 
        mPickeHour->setMaxValue(23);
        mPickeMin->setMaxValue(59);

        mPickerYear->setMinValue(2010);
        mPickerMon->setMinValue(1);
        mPickeHour->setMinValue(0);
        mPickeMin->setMinValue(0);
        mPickerDay->setMinValue(1);

        mPickerYear->getSelectedText()->setActivated(true);
        mPickerMon->getSelectedText()->setActivated(true);
        mPickerDay->getSelectedText()->setActivated(true);
        mPickeHour->getSelectedText()->setActivated(true);
        mPickeMin->getSelectedText()->setActivated(true);

        // mTipsText->setVisibility(View::GONE);
        // msetBtnLayout->setVisibility(View::VISIBLE);
    }
    mPickerYear->setValue(tm_now.tm_year + 1900);
    mPickerMon->setValue(tm_now.tm_mon + 1);
    mPickerDay->setValue(tm_now.tm_mday);
    mPickeHour->setValue(tm_now.tm_hour);
    mPickeMin->setValue(tm_now.tm_min);

    if(g_appData.tuyaWifiStatus != PRO_STATE_TUYA_GB_CLOUD_CONN)
        setMaxDay(mPickerYear->getValue(),mPickerMon->getValue()); 
}