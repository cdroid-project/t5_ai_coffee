/*
 * @Author: gezilong
 * @Email: ioremap@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:29:56
 * @FilePath: /t5_ai_coffee/src/windows/pop_base.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by gezilong, All Rights Reserved. 
 * 
**/

#include "pop_base.h"
#include "R.h"
#include "comm_func.h"
#include "wind_mgr.h"
#include "project.h"

PopBase::PopBase(ViewGroup *wind_pop_box,const std::string resource,int pop_type,popModeCb enterCb,popModeCb cencelCb)
    :mWindPopBox(wind_pop_box),mPopType(pop_type),mModeEnterCallback(enterCb),mModeCancelCallback(cencelCb) {

    mPopLayout = nullptr;
    mEnterCallback = nullptr;
    mCancelCallback = nullptr;

    initPopBase(resource);
    addPop(); //默认显示

}

PopBase::PopBase(ViewGroup *wind_pop_box,const std::string resource,int pop_type,appCb enterCb,appCb cencelCb)
    :mWindPopBox(wind_pop_box),mPopType(pop_type),mEnterCallback(enterCb),mCancelCallback(cencelCb) {

    mPopLayout = nullptr;
    mModeEnterCallback = nullptr;
    mModeCancelCallback = nullptr;

    initPopBase(resource);
    addPop(); //默认显示

}

PopBase::~PopBase() {
    mWindPopBox->removeView(mPopLayout);
    delete mPopLayout;
}

ViewGroup *PopBase::getPopLayout(){
    return mPopLayout;
}   

void PopBase::initPopBase(const std::string resource){
    mPopLayout = (ViewGroup *)LayoutInflater::from(mWindPopBox->getContext())->inflate(resource, nullptr);

}

void PopBase::removePop(){
    mWindPopBox->removeView(mPopLayout);
}

void PopBase::addPop(){
    mWindPopBox->addView(mPopLayout);
}

void PopBase::updatePageData(){
    LOGV("mPopType = %d  is not override updatePageData",mPopType);
}
void PopBase::setPopType(int type){
    mPopType = type;
}

int  PopBase::getPopType(){
    return mPopType;
}

void PopBase::cancelCallback(){
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel)->performClick();
}


void PopBase::enterCallback(){
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter)->performClick();
}

