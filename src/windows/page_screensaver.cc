/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:30:48
 * @FilePath: /t5_ai_demo/src/windows/page_screensaver.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "page_screensaver.h"

#include <core/inputeventsource.h>

#include "project.h"
#include "wind_mgr.h"
#include "conf_mgr.h"
#include "conn_mgr.h"
#include "tuya_os_mgr.h"

PageScreenSaver::PageScreenSaver(ViewGroup *wind_page_box,int page_type)
    : PageBase(wind_page_box,"@layout/page_screensaver",PAGE_SCREENSAVER) {

    mScreenSaverGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::screensaver_group);

    initPageData();
}

PageScreenSaver::~PageScreenSaver(){
    setScreenLight(g_appData.light);
}

void PageScreenSaver::stopPageAction(){
    InputEventSource::getInstance().closeScreenSaver();
}

void PageScreenSaver::initPageView(){

}

void PageScreenSaver::initPageData(){

    mPageLayout->setOnClickListener([this](View &v){
        setScreenLight(g_appData.light);

        g_appData.eqStatus = ES_PRE_HEATING;
#ifndef TUYA_OS_DISABLE
        g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
        if(!g_appData.displayMode){
            g_appData.coffeePreheatPtc = 0;
        }
        g_objConnMgr->setPreheating(g_appData.hotWaterTemp,g_appData.steamTemp);

        g_windMgr->showPrevPage();
    });

    mScreenSaverGroup->setVisibility(View::VISIBLE);

    setScreenLight(5,false);
    // mScreenSaverGroup->setVisibility(View::GONE);
    g_objConnMgr->setShutDown();
    g_appData.eqStatus = ES_SLEEPING;
#ifndef TUYA_OS_DISABLE
    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
    

}

void PageScreenSaver::updatePageData(){
    if(SystemClock::uptimeMillis() - mInitTime <= 1000) return; //防止初始化时多次调用
}

void PageScreenSaver::btnClickListener(View&view){
    LOGI("btnClickListener view.getId() = %d",view.getId());
    switch(view.getId()){
        // case t5_ai_coffee::R::id::network_btn:{
            
        //     break;
        // }
    }

}