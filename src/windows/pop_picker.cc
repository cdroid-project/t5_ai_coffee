/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:29:10
 * @FilePath: /t5_ai_coffee/src/windows/pop_picker.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "pop_picker.h"
#include "project.h"
#include "wind_mgr.h"
#include "conf_mgr.h"
#include "gaussfilterdrawable.h"

PopPicker::PopPicker(ViewGroup *wind_pop_box,View *gaussFromView,int type,appCb enterCb,appCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_picker",type,enterCb,cencelCb){

    initPopView();
    initPopData();
    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
}

PopPicker::~PopPicker(){

}

void PopPicker::initPopView(){
    mSelectModePicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_select_mode_picker_1);
    mNumberPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_num_picker_1);
    mNumberPicker2 = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_num_picker_2);

    mTitleTv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_title_tv);
    mPickerName1Tv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_name_1);
    mPickerName2Tv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_name_2_tv);
    mPickerUnits1Tv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_units_1_tv);
    mPickerUnits2Tv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_units_2_tv);

    auto clickFunc = std::bind(&PopPicker::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter)->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel)->setOnClickListener(clickFunc);
    mPopLayout->setOnClickListener(clickFunc);
    mPopLayout->setSoundEffectsEnabled(false);
}

void PopPicker::initPopData(){
    mPickerName1Tv->setVisibility(View::VISIBLE);
    mPickerName2Tv->setVisibility(View::VISIBLE);
    mPickerUnits1Tv->setVisibility(View::GONE);
    mPickerUnits2Tv->setVisibility(View::VISIBLE);
    mNumberPicker->setVisibility(View::VISIBLE);
    mNumberPicker2->setVisibility(View::VISIBLE);
    mSelectModePicker->setVisibility(View::GONE);
    if(mPopType == POP_HOT_WATER){
        mTitleTv->setText("热水设置");
        mPickerName1Tv->setText("热水温度");
        mPickerName2Tv->setText("出水量");
        mPickerUnits1Tv->setVisibility(View::VISIBLE);
        mPickerUnits1Tv->setText("℃");
        mPickerUnits2Tv->setText("ml");
        mNumberPicker->setMinValue(50);
        mNumberPicker->setMaxValue(93);
        mNumberPicker2->setMinValue(10);
        mNumberPicker2->setMaxValue(40);
        mNumberPicker2->setFormatter([this](int value){ return std::to_string(value*5); });

        mNumberPicker->setValue(g_appData.hotWaterTemp);
        mNumberPicker2->setValue(g_appData.hotWater/5);
    }else if(mPopType == POP_STEAM){
        mTitleTv->setText("蒸汽设置");
        mPickerName1Tv->setText("蒸汽模式");
        mPickerName2Tv->setText("蒸汽温度");
        mPickerUnits2Tv->setText("℃");
        mNumberPicker->setVisibility(View::GONE);
        mSelectModePicker->setVisibility(View::VISIBLE);

        mSelectModePicker->setMinValue(0);
        mSelectModePicker->setMaxValue(1);
        mNumberPicker2->setMinValue(50);
        mNumberPicker2->setMaxValue(70);

        mSelectModePicker->setFormatter([this](int value){ 
            if(value == 0){
                return "自动模式";
            }else if(value == 1){
                return "手动模式";
            }else{
                return "ERROR";
            }
        });
        mSelectModePicker->setValue(g_appData.steamMode);
        mNumberPicker2->setValue(g_appData.steamTemp);
    }
}

void PopPicker::onBtnClickListener(View&v){
    switch (v.getId()){
    case t5_ai_coffee::R::id::pop_cancel:
        g_windMgr->dealClosePopPage();
        break;
    case t5_ai_coffee::R::id::pop_enter:
        if(mPopType == POP_HOT_WATER){  
            g_appData.hotWaterTemp = mNumberPicker->getValue();
            g_appData.hotWater = mNumberPicker2->getValue()*5;
            g_objConf->setHotWater(g_appData.hotWater);
            g_objConf->setHotWaterTemp(g_appData.hotWaterTemp);
        }else if(mPopType == POP_STEAM){
            g_appData.steamMode = mSelectModePicker->getValue();
            g_appData.steamTemp = mNumberPicker2->getValue();
            g_objConf->setSteamMode(g_appData.steamMode);
            g_objConf->setSteamTemp(g_appData.steamTemp);
        }
        if(mEnterCallback) mEnterCallback();
        g_windMgr->dealClosePopPage();
        break;

    default:
        break;
    }
}

