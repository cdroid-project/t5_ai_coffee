/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:16:00
 * @FilePath: /t5_ai_coffee/src/windows/pop_warn.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "pop_tea_rebrew.h"
#include "R.h"
#include "wind_mgr.h"
#include "project.h"
#include "proto.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"

PopTeaRebrew::PopTeaRebrew(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,popModeCb enterCb,popModeCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_tea_rebrew",POP_TYPE_TEA_REBREW_SET,enterCb,cencelCb),mPickerSelectPos(selectPos){

    initPopView();

    mPopLayout->setSoundEffectsEnabled(false); // 关闭点击音效
    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
}

PopTeaRebrew::~PopTeaRebrew() {
    mResetAnimator->cancel();
    delete mResetAnimator;
}

void PopTeaRebrew::initPopView(){
    mPopPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker);

    mPopPickerWater = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_water);
    mPopPickerFlowRate = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_flowrate);
    mPopPickerTime = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_time);
    mPopPickerTemp = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_temp);

    mPopResetAnim = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_anim);
    mPopBtnConfirm   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel);
    // mPopGaussBox = (ViewGroup *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_group);
    
    initPopData();

    auto clickFunc = std::bind(&PopTeaRebrew::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->setSoundEffectsEnabled(false);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_group)->setOnClickListener(clickFunc);
    mPopPicker->setOnValueChangedListener(std::bind(&PopTeaRebrew::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    mResetAnimator = ValueAnimator::ofInt({0,10000});
    mResetAnimator->setDuration(1000);
    mResetAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
    mResetAnimator->setRepeatCount(0);       // 设置动画重复次数
    mResetAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        const int t = GET_VARIANT(anim.getAnimatedValue(),int);
        mPopResetAnim->getDrawable()->setLevel(t);
        // LOGV("position = %f",position); 
    }));

    // 旋转时有锯齿
    ((BitmapDrawable *)((RotateDrawable *)mPopResetAnim->getDrawable())->getDrawable())->setFilterBitmap(true);
}

void PopTeaRebrew::initPopData(){
    mTeaData = g_objConf->getExtractTeaData();
    
    mPopPicker->setMaxValue(mTeaData.sndModeList.size()-1);

    mPopPickerWater->setMinValue(mTeaData.waterMin/mTeaData.waterGear);
    mPopPickerWater->setMaxValue(mTeaData.waterMax/mTeaData.waterGear);
    mPopPickerFlowRate->setMinValue(0);
    mPopPickerFlowRate->setMaxValue(2);
    mPopPickerTime->setMinValue(mTeaData.interTimeMin);
    mPopPickerTime->setMaxValue(mTeaData.interTimeMax);
    mPopPickerTemp->setMinValue(mTeaData.extractTempMin);
    mPopPickerTemp->setMaxValue(mTeaData.extractTempMax);

    mPopPicker->setFormatter([this](int value){ return mTeaData.sndModeList.at(value).sndModename; });
    mPopPickerWater->setFormatter([this](int value){ return std::to_string(value*mTeaData.waterGear); });
    mPopPickerFlowRate->setFormatter([this](int value){ 
        switch(value){
            case 0:{return std::string("慢");}
            case 1:{return std::string("中");}
            case 2:{return std::string("快");}
            default:{ LOGE("mPopPickerFlowRate value[%d] is error !",value);return std::string("慢");}
        }
    });
    mPopPicker->setValue(mPickerSelectPos);
    setPopData();
}

void PopTeaRebrew::setPopData(){
    ExtractTeaSndModeDataStr sndModeData = mTeaData.sndModeList.at(mPickerSelectPos);
    mPopPickerWater->setValue(sndModeData.makeTeaWater/mTeaData.waterGear);
    mPopPickerFlowRate->setValue(sndModeData.makeTeaFlowRate);
    mPopPickerTime->setValue(sndModeData.makeTeaInterTime);
    mPopPickerTemp->setValue(sndModeData.extractTempDef);
}

void PopTeaRebrew::onBtnClickListener(View&v){
    LOGE("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_enter:{
            ExtractTeaSndModeDataStr &stepData = mTeaData.sndModeList.at(mPickerSelectPos);
            stepData.makeTeaWater = mPopPickerWater->getValue()*mTeaData.waterGear;
            stepData.makeTeaFlowRate = mPopPickerFlowRate->getValue();
            stepData.makeTeaInterTime = mPopPickerTime->getValue();
            stepData.extractTempDef = mPopPickerTemp->getValue();
            g_objConf->setExtractTeaData(mTeaData);
            if(mModeEnterCallback) mModeEnterCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_cancel:{
            if(mModeCancelCallback) mModeCancelCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_reset_group:{
            setPopData();
            mResetAnimator->start();
            break;
        }
    }
}

void PopTeaRebrew::onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue){
    switch(picker.getId()){
        case t5_ai_coffee::R::id::pop_picker:{
            mPickerSelectPos = newValue;
            setPopData();
            break;
        }
    }
}



