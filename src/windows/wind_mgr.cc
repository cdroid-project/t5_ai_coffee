/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:11:53
 * @FilePath: /t5_ai_coffee/src/windows/wind_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#include "wind_mgr.h"

#include <core/inputeventsource.h>

#include "R.h"

#include "tuya_os_mgr.h"
#include "conn_mgr.h"
#include "project.h"
// #include <conn_mgr.h>
#include "page_screensaver.h"
#include "page_home.h"
#include "page_setting.h"
#include "page_clean.h"
#include "page_factory.h"
#include "page_scale_calibration.h"

#include "pop_master_espresso.h"
#include "pop_handwash.h"
#include "pop_tea_rebrew.h"
#include "pop_tea.h"
#include "pop_normal.h"
#include "pop_set_time.h"
#include "pop_picker.h"
#include "pop_warn.h"
#include "pop_bean.h"
#define LOAD_WINDOW(T, I)                                                                                              \
    T *wnd;                                                                                                            \
    do {                                                                                                               \
        wnd = __dc(T, getWindow(I));                                                                                   \
        if (wnd) {                                                                                                     \
            LOG(WARN) << "window already exists, maybe reflush UI. ID=" << I;                                          \
            break;                                                                                                     \
        }                                                                                                              \
        LOG(INFO) << "new window. ID=" << I;                                                                           \
        wnd = new T();                                                                                                 \
        wnd->setId(I);                                                                                                 \
        regWindow(I, wnd);                                                                                             \
    } while (false)

CWindMgr::~CWindMgr(){ 
    if(mShowPage != mHomePage)      delete mHomePage;
    if(mShowPage != mSettingPage)   delete mSettingPage;
    delete mKeyBoard;
}
void CWindMgr::initWindow(){
    mWindow = new WindowBase;
    mWindLayout     = (ViewGroup *)LayoutInflater::from(mWindow->getContext())->inflate("@layout/main_window", mWindow);
    mWindPageBox    = (ViewGroup *)mWindLayout->findViewById(t5_ai_coffee::R::id::page_box);
    mWindPopBox     = (ViewGroup *)mWindLayout->findViewById(t5_ai_coffee::R::id::pop_box);
    mWindKeyBoardBox= (ViewGroup *)mWindLayout->findViewById(t5_ai_coffee::R::id::keyboard_box);
    
    mShowPage = nullptr;
    mPopPage  = nullptr;
    mSettingPage = nullptr;
    mHomePage = nullptr;
    mPreActionPage = nullptr;
    mClosePageList.clear();
    mPageList.clear();
    mPageCloseMesg.what = MSG_PAGE_DELAY_CLOSE;

    g_windMgr->showPage(PAGE_SETTING);
    g_windMgr->showPage(PAGE_HOME);

    g_objConnMgr->setDeviceData();
    g_objConnMgr->setPreheating(g_appData.hotWaterTemp,g_appData.steamTemp);
    g_appData.eqStatus = ES_PRE_HEATING;
#ifndef TUYA_OS_DISABLE
    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
    InputEventSource::getInstance().setScreenSaver(std::bind(&CWindMgr::screenSaver,this,std::placeholders::_1),5*60 *1000);       //屏保
}

void CWindMgr::screenSaver(bool bEnabled){
    if(bEnabled && (g_appData.machineState == MC_STATE_NONE) && (mShowPage->getPageType() != PAGE_FACTORY) 
        && (g_appData.eqStatus == ES_STANDBY     || g_appData.eqStatus == ES_GRIND_DONE || g_appData.eqStatus == ES_EXTRACT_DONE || 
            g_appData.eqStatus == ES_PRE_HEATING || g_appData.eqStatus == ES_EASH_DONE  || g_appData.eqStatus == ES_CLEAN_CAL_DONE) 
        && !(mPopPage && (mPopPage->getPopType() == POP_WARN))){
        dealClosePopPage(false,true);
        showPage(PAGE_SCREENSAVER);
    }

    const bool screenSaverActived= InputEventSource::getInstance().isScreenSaverActived();
    LOGI("screenSaver returned  actived=%d bEnabled=%d",screenSaverActived,bEnabled);
}

void CWindMgr::showPage(pageType type,bool isNewPage){
    LOGE("type = %d",type);

    // 当前页面与要显示的页面相同，直接返回
    if(mShowPage && mShowPage->getPageType() == type) return;
    // 判断是不是设置页面、清洁页面
    if(isNewPage && (type == PAGE_SETTING) || (type == PAGE_CLEAN) || (type == PAGE_CLEAN_CALCIFY)){
        for(auto it=mPageList.begin(); it != mPageList.end(); it++){
            if((*it == PAGE_SETTING) || (*it) == PAGE_CLEAN){
                mPageList.erase(it);
                break;
            }
        }
    }
    if(isNewPage && ((type == PAGE_CLEAN) || (type == PAGE_CLEAN_CALCIFY))){
        mPageList.push_back(PAGE_CLEAN); 
    }else if(isNewPage){
        mPageList.push_back(type);// 将要显示的页面加入显示列表
    }

    LOGE("mPageList = %d",mPageList.size());

#if defined(USE_DELAY_RECLCYER) && (USE_DELAY_RECLCYER)
    Looper::getMainLooper()->removeMessages(this,MSG_PAGE_DELAY_CLOSE);
    if(mShowPage){
        mShowPage->stopPageAction();
        if(type != PAGE_SCREENSAVER){
            if((mShowPage != mHomePage) && (mShowPage != mSettingPage)) mClosePageList.push_back(mShowPage);
        }else{
            mPreActionPage = mShowPage;
        }
        mShowPage = nullptr;
    }
    Looper::getMainLooper()->sendMessageDelayed(100,this,mPageCloseMesg);

    // 主页不销毁，判断是否为主页
    if((type == PAGE_HOME) && (mHomePage != nullptr)){
        mShowPage = mHomePage;
        mPreActionPage = nullptr;
        mShowPage->resetPageAction();
        return;
    }else if((type == PAGE_SETTING) && (mSettingPage != nullptr)){
        mShowPage = mSettingPage;
        mPreActionPage = nullptr;
        mShowPage->resetPageAction();
        return;
    }else if((type != PAGE_SCREENSAVER) && mPreActionPage){
        mShowPage = mPreActionPage;
        mPreActionPage = nullptr;
        mShowPage->resetPageAction();
        return;
    }
    // 遍历判断是否已经存在该页面，如果存在，则显示该页面，并从关闭列表中移除
    for(auto it = mClosePageList.begin();it != mClosePageList.end();it++){
        if((*it)->getPageType() == type ){
            // (*it)->addPage();
            // (*it)->setPageType(type);
            (*it)->resetPageAction();
            mShowPage = *it;
            mClosePageList.erase(it);
            return;
        }
    }
#else   
    if(mShowPage){
        mShowPage->removePage();
        delete mShowPage;
    }
#endif
    PageBase *page = nullptr;
    switch (type) {
        case PAGE_HOME:{
            page = new PageHome(mWindPageBox);
            mHomePage = page;
            break;
        }case PAGE_SCREENSAVER:{
            page = new PageScreenSaver(mWindPageBox,type);
            break;
        }case PAGE_SETTING:{
            page = new PageSetting(mWindPageBox);
            mSettingPage = page;
            break;
        }case PAGE_CLEAN:
        case PAGE_CLEAN_CALCIFY:{
            page = new PageClean(mWindPageBox,type);
            break;
        }case PAGE_FACTORY:{
            page = new PageFactory(mWindPageBox);
            break;
        }case PAGE_SCALE_CALIBRATION:{
            page = new PageScaleCalibration(mWindPageBox);
            break;
        }default:{
            break;
        } 
    }

    mShowPage = page;
}

void CWindMgr::showPrevPage(){
    if(mPageList.size() <= 1){
        mPageList.clear();
        showPage(PAGE_HOME);
        return;
    }else{
        mPageList.pop_back();
        showPage(mPageList.back(),false);
    }
}

void CWindMgr::showHomePage(){
    if(mPageList.size()>1)mPageList.clear();
    // if(mShowPage && (mShowPage->getPageType() != PAGE_HOME))mPageList.clear();
    showPage(PAGE_HOME);
}

void CWindMgr::showPopPage(int type,View *gaussFromView,appCb enterCb,appCb cencelCb){
    if(mPopPage){  LOGE("pop page is exist"); return; }
    else if(mShowPage && mShowPage->getPageType() == PAGE_SCREENSAVER){ LOGE("show page is screensaver"); return; }
   
    if(!gaussFromView && mShowPage) gaussFromView = mShowPage->getPageLayout();
    switch(type){
        case POP_OTA:
        case POP_BIND_APP:
        case POP_UNBIND_APP:
        case POP_RESTORE_FACTORY:
        case POP_STOP_CLEAN:
        case POP_GET_AUTH:
        case POP_TYPE_TEA_EXT_TIPS:
        case POP_TYPE_TEA_REBREW_TIPS:
        case POP_TYPE_TEA_CANCEL_WORK:{
            mPopPage = new PopNormal(mWindPopBox,gaussFromView,type,enterCb,cencelCb);
            break;
        }case POP_SET_TIME:{
            mPopPage = new PopSetTime(mWindPopBox,gaussFromView,enterCb,cencelCb);
            break;
        }case POP_HOT_WATER:
        case POP_STEAM:{
            mPopPage = new PopPicker(mWindPopBox,gaussFromView,type,enterCb,cencelCb);
            break;
        }case POP_WARN:{
            mPopPage = new PopWarn(mWindPopBox,gaussFromView,type,enterCb,cencelCb);
            break;
        }case POP_BEAN_GRIND:{
            mPopPage = new PopBean(mWindPopBox,gaussFromView,type,enterCb,cencelCb);
            break;
        }
    }
    
    mWindPageBox->setVisibility(View::INVISIBLE);
    mWindPopBox->setVisibility(View::VISIBLE);
}

void CWindMgr::showSelectPosPopPage(int type,View *gaussFromView,int selectPos,popModeCb enterCb,popModeCb cencelCb){
    if(mPopPage){  LOGE("pop page is exist"); return; }
    else if(mShowPage && mShowPage->getPageType() == PAGE_SCREENSAVER){ LOGE("show page is screensaver"); return; }

    if(type == POP_TYPE_MASTER_ESPRESSO){
        mPopPage = new PopMasterEspresso(mWindPopBox,gaussFromView,selectPos,enterCb,cencelCb);
    }else if(POP_TYPE_TEA_REBREW_SET){
        mPopPage = new PopTeaRebrew(mWindPopBox,gaussFromView,selectPos,enterCb,cencelCb);
    }
    mWindPageBox->setVisibility(View::INVISIBLE);
    mWindPopBox->setVisibility(View::VISIBLE);
}

void CWindMgr::showSndStepPopPage(int type,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb ,popModeCb cencelCb ){
    if(mPopPage){  LOGE("pop page is exist"); return; }
    else if(mShowPage && mShowPage->getPageType() == PAGE_SCREENSAVER){ LOGE("show page is screensaver"); return; }

    if(type == POP_TYPE_HAND_WASH){
        mPopPage = new PopHandWash(mWindPopBox,gaussFromView,selectPos,selectStep,enterCb,cencelCb);
    }else if(type == POP_TYPE_TEA){
        mPopPage = new PopTea(mWindPopBox,gaussFromView,selectPos,selectStep,enterCb,cencelCb);
    }
    mWindPageBox->setVisibility(View::INVISIBLE);
    mWindPopBox->setVisibility(View::VISIBLE);
}

void CWindMgr::dealClosePopPage(bool autoClickEnterCb,bool autoClickCancelCb){
    if(mPopPage){
        if(autoClickEnterCb)        mPopPage->enterCallback();
        else if(autoClickCancelCb)  mPopPage->cancelCallback();
        delete mPopPage;
        mPopPage = nullptr;
        mWindPopBox->setVisibility(View::INVISIBLE);
        mWindPageBox->setVisibility(View::VISIBLE);
    }
}

void CWindMgr::showKeyBoardPage(std::string iText,std::string description,int maxCount,input_callback enterCallback,exit_callback exitCallback){
    mKeyBoard = new PageKeyBoard(mWindKeyBoardBox,iText,description,maxCount,enterCallback,exitCallback);
    mWindPageBox->setVisibility(View::INVISIBLE);
    mWindPopBox->setVisibility(View::INVISIBLE);
    mWindKeyBoardBox->setVisibility(View::VISIBLE);
}

void CWindMgr::dealCloseKeyBoardPage(){
    delete mKeyBoard;
    mKeyBoard = nullptr;
    mWindKeyBoardBox->setVisibility(View::GONE);
    if(mPopPage) mWindPopBox->setVisibility(View::VISIBLE);
    else         mWindPageBox->setVisibility(View::VISIBLE);
}

void CWindMgr::updateDate(){
    LOGI_IF(g_appData.statusChangeFlag,"updateDate  statusChangeFlag = %d",g_appData.statusChangeFlag);
    do{
        if((g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && (g_appData.warnState & ~g_appData.warnIsDealState)
            && ((mPopPage && (mPopPage->getPopType() != POP_WARN)) || !mPopPage)){
                dealClosePopPage(false,true);
                showPopPage(POP_WARN,nullptr);
                break;
        }
        if(mPopPage){ mPopPage->updatePageData(); }
    }while(false);
    
    // 涂鸦下发 萃取、磨豆指令
    if(g_appData.statusChangeFlag & (CS_TUYA_START_GRIND | CS_TUYA_START_EXT)){
        if((g_appData.statusChangeFlag & CS_TUYA_START_EXT) && mPopPage && mPopPage->getPopType() == POP_TYPE_TEA_EXT_TIPS){
            g_appData.statusChangeFlag &= ~CS_TUYA_START_EXT;
            if(g_appData.tuyaStartExt)  { dealClosePopPage(true,false); }
            else                        { dealClosePopPage(false,true); }
        }else if((g_appData.statusChangeFlag & CS_TUYA_START_EXT) && mPopPage && mPopPage->getPopType() == POP_TYPE_TEA_REBREW_TIPS){
            if(g_appData.tuyaStartExt)  { 
                // 当前显示是否再萃弹窗，且APP下发是确定再萃，则直接关闭弹窗，再交由 tea 的mgr 去处理。因为需要对下发的数据做处理。
                // dealClosePopPage(); 
                // 若APP也是先需要取消掉是否再萃的弹窗，再操作，则无需这么麻烦，以弹窗的处理为准即可。
                g_appData.statusChangeFlag &= ~CS_TUYA_START_EXT;
                dealClosePopPage(true,false);
            }else { 
                g_appData.statusChangeFlag &= ~CS_TUYA_START_EXT;
                dealClosePopPage(false,true); 
            }
        }else{
            dealClosePopPage(false,true);
        }
        showHomePage();
        ((PageHome *)mShowPage)->stopSteamWork();
    }

    if(g_appData.statusChangeFlag & CS_PRE_HEATING){
        g_appData.eqStatus = (g_appData.coffeePreheatPtc < 100)?ES_PRE_HEATING:ES_STANDBY;
#ifndef TUYA_OS_DISABLE
        g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
        LOGE("g_appData.coffeePreheatPtc = %d  g_appData.eqStatus  = %d",g_appData.coffeePreheatPtc,g_appData.eqStatus );
#endif
    }
    if(g_appData.statusChangeFlag & CS_TUYA_START_CLEAN){
        if(g_appData.tuyaStartCleaning && (mShowPage->getPageType() != PAGE_CLEAN)){
            if(mShowPage->getPageType() == PAGE_HOME) ((PageHome *)mShowPage)->stopSteamWork();
            showPage(PAGE_CLEAN);
            return;
        }
    }

    if(g_appData.statusChangeFlag & CS_AI_DATA_CHANGE){
        showHomePage();
        std::string deviceText = g_appData.aiJsonText["device"].isString()?g_appData.aiJsonText["device"].asString():"null";

        if(deviceText.find("coffee") != std::string::npos){
            g_appData.statusChangeFlag |= CS_AI_EXT;
            g_appData.statusChangeFlag &= ~CS_AI_DATA_CHANGE;
        }
    }

    if(mShowPage) mShowPage->updatePageData();
    g_appData.statusChangeFlag = 0x00;
}

void CWindMgr::changeTitleBar(uchar status){
    if(mShowPage) mShowPage->changeTitleBarStatus(status);
}

void CWindMgr::handleMessage(Message& message){
    LOGE("message.what = %d",message.what);
    switch(message.what){
        case MSG_PAGE_DELAY_CLOSE:{
            LOGE("mClosePageList.size() = %d",mClosePageList.size());
            for(auto it = mClosePageList.rbegin();it != mClosePageList.rend();it++){
                (*it)->removePage();
                delete *it;
            }
            mClosePageList.clear();
            break;
        }
    }
}

WindowBase::WindowBase() : Window(0, 0, -1, -1) {
    mAttachInfo->mPlaySoundEffect = playSound;

}   

WindowBase::~WindowBase() {}