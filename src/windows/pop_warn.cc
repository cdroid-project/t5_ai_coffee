#include "pop_warn.h"
#include "wind_mgr.h"
#include "conn_mgr.h"
#include "ota.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"
#include "tuya_os_mgr.h"

PopWarn::PopWarn(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb)
    :PopBase(wind_pop_box,"@layout/pop_warn",popType,enterCb,cancelCb){

    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
    mWarnData = (g_appData.warnState & ~g_appData.warnIsDealState);
    LOGE("mWarnData = %d g_appData.warnState = %d g_appData.warnIsDealState = %d",mWarnData,g_appData.warnState,g_appData.warnIsDealState);
    initPopView();
    initWarnText();
    initPopData();
}

PopWarn::~PopWarn(){
    if(g_appData.warnShowType != A02){
        g_appData.warnShowType = WARN_NONE;
#ifndef TUYA_OS_DISABLE
        g_tuyaOsMgr->reportDpData(TYCMD_FAULT,PROP_BITMAP, &g_appData.warnShowType); // 故障告警
        g_tuyaOsMgr->reportDpData(TYCMD_WORK_ALARM,PROP_BITMAP, &g_appData.warnShowType); // 故障告警
#endif
    }
}

void PopWarn::initPopView(){
    mPopGroup       = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_group);

    mWarnInfoGroup  = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_info_group);
    mWarnCleanGroup  = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_clean_group);

    mWarnImg         = (ImageView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_img);
    mPopBtnConfirm  = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_cancel);
    
    mWarnTitleTv     = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_title_tv);
    mWarnInfoTv      = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_info_tv);

    mWarnCleanInfoTv = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_clean_info_tv);
    mWarnCleanSwitchTv = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::warn_clean_switch_tv);

    auto clickFunc = std::bind(&PopWarn::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->setSoundEffectsEnabled(false);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);
    mWarnCleanSwitchTv->setOnClickListener(clickFunc);

}

void PopWarn::initPopData(){
    mWarnInfoGroup->setVisibility(View::VISIBLE);
    mWarnCleanGroup->setVisibility(View::GONE);

    mPopBtnConfirm->setVisibility(View::VISIBLE);
    mPopBtnCancel->setVisibility(View::GONE);
    mPopBtnConfirm->setText("确定");

    if(g_appData.warnShowType == A02){
        mWarnInfoGroup->setVisibility(View::GONE);
        mWarnCleanGroup->setVisibility(View::VISIBLE);
        mPopBtnCancel->setVisibility(View::VISIBLE);

        mWarnCleanInfoTv->setText(mWarnInfoText);
        mWarnCleanSwitchTv->setText("两周内不再弹窗提醒");
        mPopBtnConfirm->setText("立即清洁");
        mPopBtnCancel->setText("稍后清洁");

        mWarnCleanSwitchTv->setSelected(true);
    }else{
        mWarnTitleTv->setText(mWarnTitleText);
        mWarnInfoTv->setText(mWarnInfoText);
    }
}

void PopWarn::updatePageData(){
    if((g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE)){
        LOGE("mWarnData = %d ---- warnState = %d ---- warnIsDealState = %d ---- xxx = %d ---- warnShowType = %d",mWarnData,g_appData.warnState,g_appData.warnIsDealState,g_appData.warnState & g_appData.warnIsDealState,g_appData.warnShowType);
        mWarnData =  (g_appData.warnState & ~g_appData.warnIsDealState);
        if(mWarnData == WARN_NONE){
            g_appData.statusChangeFlag &= ~CS_WARN_STATUS_CHANGE;
            g_appData.warnShowType = WARN_NONE;
            // if(mEnterCallback) mEnterCallback();
            // g_appData.statusChangeFlag |= CS_WARN_CLOSE;
            // g_windMgr->updateDate();
            g_windMgr->dealClosePopPage();
        }else if(!(mWarnData & g_appData.warnShowType)){
            initWarnText();
            initPopData();
        }
    }
}

void PopWarn::onBtnClickListener(View&v){
    LOGI("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::pop_enter:{
            g_appData.warnIsDealState |= g_appData.warnShowType;
            if(g_appData.warnShowType == A02){
                g_windMgr->showPage(PAGE_CLEAN_CALCIFY);
            }else{
                mWarnData &= ~g_appData.warnShowType;
                if(mWarnData != WARN_NONE){
                    initWarnText();
                    initPopData();
                    break;
                }
            }
            g_appData.warnShowType = WARN_NONE;
            if(mEnterCallback) mEnterCallback();
            g_appData.statusChangeFlag |= CS_WARN_CLOSE;
            g_windMgr->updateDate();
            g_windMgr->dealClosePopPage();
            break;
        }case kaidu_ms7_lqy::R::id::pop_cancel:{
            if(g_appData.warnShowType == A02){
                g_appData.warnIsDealState |= g_appData.warnShowType;
                std::time_t nowTime = std::time(NULL);
                g_appData.nextPopWarnClean = *localtime(&nowTime);
                if(mWarnCleanSwitchTv->isSelected()){
                    nowTime += 14*24*60*60;
                    g_appData.nextPopWarnClean = *localtime(&nowTime);
                }
            }
            g_appData.statusChangeFlag |= CS_WARN_CLOSE;
            g_windMgr->updateDate();
            g_windMgr->dealClosePopPage();
            break;
        }case kaidu_ms7_lqy::R::id::warn_clean_switch_tv:{
            mWarnCleanSwitchTv->setSelected(!mWarnCleanSwitchTv->isSelected());
            break;
        }
    }
}

void PopWarn::initWarnText(){
    LOGE("mWarnData = %d",mWarnData);
    g_appData.warnShowType = WARN_NONE;
    if(mWarnData & E02){
        g_appData.warnShowType = E02;
        mWarnTitleText = "咖啡传感器异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E02：咖啡传感器异常");
    }else if(mWarnData & E03){
        g_appData.warnShowType = E03;
        mWarnTitleText = "蒸汽传感器异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E03：蒸汽传感器异常");
    }else if(mWarnData & E04){
        g_appData.warnShowType = E04;
        mWarnTitleText = "出奶传感器异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E04：出奶传感器异常");
    }else if(mWarnData & E05){
        g_appData.warnShowType = E05;
        mWarnTitleText = "流量计异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E05：流量计异常");
    }else if(mWarnData & E06){
        g_appData.warnShowType = E06;
        mWarnTitleText = "通讯异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E06：通讯异常");
    }else if(mWarnData & E07){
        g_appData.warnShowType = E07;
        mWarnTitleText = "豆盒未装配";
        mWarnInfoText = "检查豆盒是否安装到位";
        LOGI("E07：豆盒未装配");
    }else if(mWarnData & E08){
        g_appData.warnShowType = E08;
        mWarnTitleText = "磨豆电机保护";
        mWarnInfoText = "5分钟内，累计磨豆时长超过180秒，请停机片刻后再使用";
        LOGI("E08：磨豆电机保护中（5分钟内，累计磨豆时长超过180秒）");
    }else if(mWarnData & E09){
        g_appData.warnShowType = E09;
        mWarnTitleText = "容器未正确安装";
        mWarnInfoText = "请放置容器在对应的位置上";
        LOGI("E09：磨豆或萃取过程中取走容器、工作中取走容器");
    }else if(mWarnData & E13){
        g_appData.warnShowType = E13;
        mWarnTitleText = "过零线路异常";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E14：过零线路异常");
    }else if(mWarnData & E15){
        g_appData.warnShowType = E15;
        mWarnTitleText = "蒸汽传感器高温保护";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E15：蒸汽传感器高温保护");
    }else if(mWarnData & E16){
        g_appData.warnShowType = E16;
        mWarnTitleText = "出奶传感器高温保护";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E16：出奶传感器高温保护");
    }else if(mWarnData & E01){
        g_appData.warnShowType = E01;
        mWarnTitleText = "咖啡机传感器高温保护";
        mWarnInfoText = "请联系4008-11-168售后客户咨询处理";
        LOGI("E01：咖啡机传感器高温保护");
    }else if(mWarnData & E10){
        g_appData.warnShowType = E10;
        mWarnTitleText = "水箱未装配";
        mWarnInfoText = "检查水箱是否安装到位";
        LOGI("E13：暂未定义");
    }
#ifndef TUYA_OS_DISABLE
    g_tuyaOsMgr->reportDpData(TYCMD_FAULT,PROP_BITMAP, &g_appData.warnShowType); // 故障告警
#endif
    if(g_appData.warnShowType != WARN_NONE) return;
    g_appData.warnShowType = WARN_NONE;
    int reportAlarm = 0x00;
    if(mWarnData & A01){
        g_appData.warnShowType = A01;
        reportAlarm = 0x01;
        mWarnTitleText = "水箱缺水";
        mWarnInfoText = "请往水箱中注水";
        LOGI("缺水警告"); 
    }else if(mWarnData & A02){
        g_appData.warnShowType = A02;
        reportAlarm = 0x02;
        mWarnInfoText = "您的咖啡机需要进行钙化清洁了，是否立即清洁？";
        LOGI("除垢提醒，萃取使用累计超1500次。");
    }else if(mWarnData & A03){
        g_appData.warnShowType = A03;
        reportAlarm = 0x04;
        mWarnTitleText = "萃取压力过大";
        mWarnInfoText = "请减少咖啡粉量或把研磨度调大";
        LOGI("超压警告");
    }
#ifndef TUYA_OS_DISABLE
    g_tuyaOsMgr->reportDpData(TYCMD_WORK_ALARM,PROP_BITMAP, &reportAlarm); // 故障告警
#endif
}

