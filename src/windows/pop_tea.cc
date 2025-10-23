/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:28:42
 * @FilePath: /t5_ai_coffee/src/windows/pop_tea.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "pop_tea.h"
#include "R.h"
#include "wind_mgr.h"
#include "project.h"
#include "proto.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"

PopTea::PopTea(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb,popModeCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_tea",POP_TYPE_TEA,enterCb,cencelCb),mPickerSelectPos(selectPos),mStepSelectPos(selectStep){

    LOGE("mStepSelectPos = %d",mStepSelectPos);
    initPopView();

    mPopLayout->setSoundEffectsEnabled(false); // 关闭点击音效
    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
}

PopTea::~PopTea() {
    mResetAnimator->cancel();
    delete mResetAnimator;
}

void PopTea::initPopView(){
    mPopPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker);

    mPopStepPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_picker);
    mTipsPowderTv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_tips_powder_tv);
    mTipsPowderMaxTv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_tips_powder_max_tv);
    mTipsWashTimesTv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_tips_wash_times_tv);
    mTipsPressureTv = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_tips_pressure_tv);

    mPopPickerWater = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_water);
    mPopPickerFlowRate = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_flowrate);
    mPopPickerTime = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_time);

    mPopResetAnim = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_anim);
    mPopBtnConfirm   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel);
    // mPopGaussBox = (ViewGroup *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_group);
    
    initPopData();

    auto clickFunc = std::bind(&PopTea::onBtnClickListener,this,std::placeholders::_1);
    auto PickerFunc = std::bind(&PopTea::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_group)->setOnClickListener(clickFunc);
    mPopPicker->setOnValueChangedListener(PickerFunc);
    mPopStepPicker->setOnValueChangedListener(PickerFunc);

    mPopLayout->setSoundEffectsEnabled(false);
    
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

void PopTea::initPopData(){
    mTeaData = g_objConf->getExtractTeaData();
    mPopPicker->setMaxValue(mTeaData.sndModeList.size()-1);
    mPopStepPicker->setMaxValue(2);

    mPopPickerWater->setMinValue(mTeaData.waterMin/mTeaData.waterGear);
    mPopPickerWater->setMaxValue(mTeaData.waterMax/mTeaData.waterGear);
    mPopPickerFlowRate->setMinValue(0);
    mPopPickerFlowRate->setMaxValue(2);
    mPopPickerTime->setMinValue(mTeaData.interTimeMin);
    mPopPickerTime->setMaxValue(mTeaData.interTimeMax);

    mPopPicker->setFormatter([this](int value){ return mTeaData.sndModeList.at(value).sndModename; });
    mPopStepPicker->setFormatter([this](int value){ 
        switch(value){
            case 0:{ return "洗茶";}
            case 1:{ return "泡茶";}
            case 2:{ return "冲茶";}
        }
    });
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
    mPopStepPicker->setValue(mStepSelectPos);
    setStepData();
    setTipsData();
}

void PopTea::setStepData(){
    ExtractTeaSndModeDataStr stepData = mTeaData.sndModeList.at(mPickerSelectPos);

    switch(mStepSelectPos){
        case 0 :{
            mPopPickerWater->setValue(stepData.washTeaWater/mTeaData.waterGear);
            mPopPickerFlowRate->setValue(stepData.washTeaFlowRate);
            mPopPickerTime->setValue(stepData.washTeaInterTime);
            break;
        }case 1:{
            mPopPickerWater->setValue(stepData.soakTeaWater/mTeaData.waterGear);
            mPopPickerFlowRate->setValue(stepData.soakTeaFlowRate);
            mPopPickerTime->setValue(stepData.soakTeaInterTime);
            break;
        }case 2:{
            mPopPickerWater->setValue(stepData.makeTeaWater/mTeaData.waterGear);
            mPopPickerFlowRate->setValue(stepData.makeTeaFlowRate);
            mPopPickerTime->setValue(stepData.makeTeaInterTime);
            break;
        }
    }
}

void PopTea::saveStepData(){
    ExtractTeaSndModeDataStr &stepData = mTeaData.sndModeList.at(mPickerSelectPos);
    switch(mStepSelectPos){
        case 0 :{
            stepData.washTeaWater       = mPopPickerWater->getValue()*mTeaData.waterGear;
            stepData.washTeaFlowRate    = mPopPickerFlowRate->getValue();
            stepData.washTeaInterTime   = mPopPickerTime->getValue();
            break;
        }case 1:{
            stepData.soakTeaWater       = mPopPickerWater->getValue()*mTeaData.waterGear;
            stepData.soakTeaFlowRate    = mPopPickerFlowRate->getValue();
            stepData.soakTeaInterTime   = mPopPickerTime->getValue();
            break;
        }case 2:{
            stepData.makeTeaWater       = mPopPickerWater->getValue()*mTeaData.waterGear;
            stepData.makeTeaFlowRate    = mPopPickerFlowRate->getValue();
            stepData.makeTeaInterTime   = mPopPickerTime->getValue();
            break;
        }
    }
}

void PopTea::setTipsData(){
    ExtractTeaSndModeDataStr stepData = mTeaData.sndModeList.at(mPickerSelectPos);
    mTipsPowderTv->setText("标准"+std::to_string(stepData.powderDef)+"g");
    mTipsWashTimesTv->setText(std::to_string(stepData.washTeaNumMin)+"-"+std::to_string(stepData.washTeaNumMax)+"次");
    mTipsPressureTv->setText(std::to_string(stepData.pressure)+"bar");
}

void PopTea::onBtnClickListener(View&v){
    LOGE("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_enter:{
            saveStepData();
            g_objConf->setExtractTeaData(mTeaData);
            if(mModeEnterCallback) mModeEnterCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_cancel:{
            if(mModeCancelCallback) mModeCancelCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_reset_group:{
            // mTeaData.sndModeList.at(mPickerSelectPos) = g_objConf->getExtractTeaData().sndModeList.at(mPickerSelectPos);
            mTeaData.sndModeList.at(mPickerSelectPos) = g_objConf->getExtractTeaData(true).sndModeList.at(mPickerSelectPos);
            setStepData();
            mResetAnimator->start();
            break;
        }
    }
}

void PopTea::onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue){
    switch(picker.getId()){
        case t5_ai_coffee::R::id::pop_picker:{
            mPickerSelectPos = newValue;
            mTeaData.sndModeList.at(oldValue) = g_objConf->getExtractTeaData().sndModeList.at(oldValue);
            setStepData();
            setTipsData();
            break;
        }case t5_ai_coffee::R::id::pop_step_picker:{
            saveStepData();
            mStepSelectPos = newValue;
            setStepData();
            break;
        }
    }
}



