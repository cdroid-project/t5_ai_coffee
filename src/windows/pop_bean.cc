/*
 * @Author: gezilong
 * @Email: ioremap@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:40:19
 * @FilePath: /t5_ai_coffee/src/windows/pop_bean.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by gezilong, All Rights Reserved. 
 * 
**/

#include "pop_bean.h"
#include "wind_mgr.h"
#include "conn_mgr.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"
#include "tuya_os_mgr.h"

PopBean::PopBean(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb)
    :PopBase(wind_pop_box,"@layout/pop_bean",popType,enterCb,cancelCb){

    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));

    initPopView();
    initPopData();
}

PopBean::~PopBean(){

}

void PopBean::initPopView(){

    mBeanModeWeightTv  = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_bean_weight_tv);
    mBeanModeTimeTv   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_bean_time_tv);
    
    auto clickFunc = std::bind(&PopBean::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->setSoundEffectsEnabled(false);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mBeanModeWeightTv->setOnClickListener(clickFunc);
    mBeanModeTimeTv->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter)->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel)->setOnClickListener(clickFunc);
}

void PopBean::initPopData(){
    mBeanModeWeightTv->setSelected(false);
    mBeanModeTimeTv->setSelected(false);
    if(g_appData.beanGrindMode == BEAN_GRIND_WEI)   mBeanModeWeightTv->setSelected(true);
    else                                            mBeanModeTimeTv->setSelected(true);
}

void PopBean::updatePageData(){

}

void PopBean::onBtnClickListener(View&v){
    LOGI("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_enter:{
            g_appData.beanGrindMode = mBeanModeWeightTv->isSelected()?BEAN_GRIND_WEI:BEAN_GRIND_TIME;
            g_objConf->setBeanGrindMode(g_appData.beanGrindMode);
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_GRAIN_UNIT_MODE,PROP_ENUM, &g_appData.beanGrindMode); // 工作状态
#endif
            if(mEnterCallback) mEnterCallback();
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_cancel:{
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_bean_weight_tv:{
            if(!v.isSelected()){
                v.setSelected(true);
                mBeanModeTimeTv->setSelected(false);
            }
            break;
        }case t5_ai_coffee::R::id::pop_bean_time_tv:{
            if(!v.isSelected()){
                v.setSelected(true);
                mBeanModeWeightTv->setSelected(false);
            }
            break;
        }
    }
}

