/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:45:09
 * @FilePath: /t5_ai_coffee/src/protocol/tuya_os_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#ifndef TUYA_OS_DISABLE
#include "tuya_os_mgr.h"

#include <conf_mgr.h>
#include <data.h>
#include <wind_mgr.h>
#include <core/app.h>
#include "conn_mgr.h"

#define TICK_TIME 100 // tick触发时间（毫秒）

void tuya_ota_upgrade(const char *path)
{	
    LOGE("tuya ota upgrade!");
	// LOGE("ota upgrade:%s!",path);
	// 执行当前目录下的升级脚本（差异覆盖）
	// std::system("./upgrade.sh");
}


TuyaOsMgr *g_tuyaOsMgr = TuyaOsMgr::ins();
//////////////////////////////////////////////////////////////////

TuyaOsMgr::TuyaOsMgr() {
    mIsConnCloud       = false;
    mIsNetDisConn      = false;
    mDisConnCloudTime  = 0;
    mNextGetWeatherTime = 0;
    mNextUpdateTime    = 0; 
    mNextReportStandbyTime = 0;
    mIsInit            = false;
    mIsRequesting      = false;
    mIsRecvDiyData     = false;
    mRawDpCacheData    = nullptr;
    mCallbackList.clear();
}

TuyaOsMgr::~TuyaOsMgr() {
    App::getInstance().removeEventHandler(this);
}

int TuyaOsMgr::init() {
    if(mIsInit) return -1;
    // 启动延迟一会后开始发包
    mNextEventTime = SystemClock::uptimeMillis() + TICK_TIME * 10;
    App::getInstance().addEventHandler(this);

    std::string authUuid;
    std::string authKey;
    g_objConf->getTuyaAuthCode(authUuid,authKey);
    if(authUuid != "none" && authKey != "none"){
        // 初始化 tuya 
        tuya_gw_init(authUuid.c_str(),authKey.c_str());
        // 添加 tuya ota升级的回调
        tuya_gw_set_upgrade_cb(tuya_ota_upgrade);
        // 添加 tuya 的dp指令下发的回调
        tuya_dp_deal_cb(std::bind(&TuyaOsMgr::onDpCommDeal,this,std::placeholders::_1,std::placeholders::_2));

        tuya_net_statue_cb(std::bind(&TuyaOsMgr::onNetStatus,this,std::placeholders::_1));

        tuya_qrcode_deal_cb(std::bind(&TuyaOsMgr::onGetQrcode,this,std::placeholders::_1));
        
        mIsInit = true;
    }
    return 0;
}

int TuyaOsMgr::checkEvents() {
    int64_t curr_tick = SystemClock::uptimeMillis();
    if(curr_tick >= mConnCompTime){
        if ((curr_tick >= mNextEventTime) || (mCallbackList.size() > 0)) {
            mNextEventTime = curr_tick + TICK_TIME;
            return 1;
        }
    }
    return 0;
}

int TuyaOsMgr::handleEvents() {
    int64_t now_tick = SystemClock::uptimeMillis();
    dealTuyaCallback();

    if( mIsConnCloud && now_tick >= mNextGetWeatherTime){
        ThreadPool::ins()->add(new RequestWeatherThread, &mNextGetWeatherTime,true);
        mNextGetWeatherTime = now_tick + 30 *60 * 1000;
    }
    if( mIsConnCloud && now_tick >= mNextUpdateTime){
        ThreadPool::ins()->add(new RequestTimeThread, nullptr,true);
        mNextUpdateTime = now_tick + 30*60*1000;
    }
    if( mIsConnCloud && now_tick >= mNextReportAllTime){
        reportAllStatus();
        mNextReportAllTime = now_tick + 30*60*1000;
    }
    if( mIsNetDisConn && ( now_tick - mDisConnCloudTime) >= 2*60*1000){
        mIsNetDisConn = false;
    }
    if( (mNextReportStandbyTime!=0) && (now_tick >= mNextReportStandbyTime)){
        g_appData.eqStatus = ES_STANDBY;
        reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 设备状态
        mNextReportStandbyTime = 0;
    }
    return 1;
}
// g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_MODE,PROP_ENUM, &cleanMode); // 清洁工作模式
        // g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); // 清洁 false
bool TuyaOsMgr::onDpCommDeal(unsigned char dpid,const void *data){
    LOGI("dpid = %d",dpid);
    switch(dpid){
        case TYCMD_BEAN_START:{
            g_appData.tuyaStartGrind = *static_cast<const bool*>(data);
            mCallbackList.push_back(CS_TUYA_START_GRIND);
            LOGI("磨豆开关：%s",g_appData.tuyaStartGrind?"开":"关");
            break;
        }case TYCMD_POWDER_SET:{
            g_appData.tuyaSetPowder = *static_cast<const int*>(data);
            LOGI("粉量设置  g_appData.tuyaSetPowder = %d",g_appData.tuyaSetPowder);
            break;
        }case TYCMD_MAKEING_START:{
            g_appData.tuyaStartExt = *static_cast<const bool*>(data);
            mCallbackList.push_back(CS_TUYA_START_EXT);
            LOGI("萃取、热水制作 开关：%s",g_appData.tuyaStartExt?"开":"关");
            break;
        }case TYCMD_MAKE_TYPE:{
            g_appData.tuyaMakeType = *static_cast<const unsigned int*>(data);
            LOGI("设置制作大类：%d",g_appData.tuyaMakeType);
            break;
        }case TYCMD_COFFEE_DIY:{
            const unsigned char* diyData = static_cast<const unsigned char*>(data);
            int len;
            if(diyData[0] == COFFEE_TYPE_NORMAL)    len = 7;
            else if(diyData[0] == COFFEE_TYPE_MAS)  len = 17;
            else                                    len = 19;

            memcpy(g_appData.tuyaDiyData,data,len);
            mIsRecvDiyData = true;
            LOG(INFO) << "咖啡DIY透传型数据： hex str=" << hexstr(g_appData.tuyaDiyData, len);
            break;
        }
        case TYCMD_TEA_DIY:{
            const unsigned char* diyData = static_cast<const unsigned char*>(data);
            int len;
            if(diyData[0] == MAKE_TEA)  len = 6;
            else                        len = 12;

            memcpy(g_appData.tuyaDiyData,data,len);
            mIsRecvDiyData = true;
            LOG(INFO) << "萃茶 DIY透传型数据： hex str=" << hexstr(g_appData.tuyaDiyData, len);
            break;
        }case TYCMD_MODE:{
            // g_appData.tuyaExtCoffeeMode = 
            break;
        }case TYCMD_CLEANING_MODE:{
            g_appData.tuyaCleanMode = *static_cast<const unsigned int*>(data);
            LOGI("设置清洁模式 = %d",g_appData.tuyaCleanMode);
            break;
        }case TYCMD_CLEANING_START:{
            g_appData.tuyaStartCleaning = *static_cast<const bool*>(data);
            mCallbackList.push_back(CS_TUYA_START_CLEAN);
            LOGI("清洁开关：%s",g_appData.tuyaStartCleaning?"开":"关");
            break;
        }
    }
    return false;
}

void TuyaOsMgr::dealTuyaCallback(){
    for(auto it=mCallbackList.begin();it!=mCallbackList.end();){
        switch(*it){
            case CS_TUYA_START_GRIND:{
                if((g_appData.tuyaStartGrind && (g_appData.tuyaSetPowder < 0))){
                    it++;
                    continue;
                }
                break;
            }case CS_TUYA_START_EXT:{
                if((!mIsRecvDiyData || (g_appData.tuyaMakeType < 0)) && g_appData.tuyaStartExt){
                    it++;
                    continue;
                }
                mIsRecvDiyData = false;
                break;
            }case CS_TUYA_START_CLEAN:{
                if((g_appData.tuyaCleanMode < 0) && g_appData.tuyaStartCleaning){
                    it++;
                    continue;
                }
                break;
            }case CS_TUYA_NET_CHANGE:{
                break;
            }
            default:{
                return;
            }
        }
        g_appData.statusChangeFlag |= *it;
        it = mCallbackList.erase(it);
    }
    g_windMgr->updateDate();
}

void TuyaOsMgr::onNetStatus(unsigned char status){
    // 二维码获取成功
    g_appData.tuyaWifiStatus = PRO_STATE_TUYA_GB_LAN_UNCONN + static_cast<int>(status);

    if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
        mNextGetWeatherTime = SystemClock::uptimeMillis() + 7*1000;
        mNextUpdateTime     = SystemClock::uptimeMillis() + 7*1000;
        mNextReportAllTime  = SystemClock::uptimeMillis() + 1*1000;
        mConnCompTime       = SystemClock::uptimeMillis() + 1*1000;
        mIsConnCloud = true;
    }else if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_LAN_CONN){
        if(mIsNetDisConn){
            g_appData.tuyaWifiStatus = PRO_STATE_TUYA_GB_CLOUD_CONN;
            mNextReportAllTime  = SystemClock::uptimeMillis() + 3*1000;
            mConnCompTime       = SystemClock::uptimeMillis() + 3*1000;
            mIsConnCloud = true;
        }else if(mIsConnCloud){
            LOGE("is Conn Network !*************");
        }else{
            mIsConnCloud = false;
        }
    }else{
        if(mIsConnCloud){
            mIsNetDisConn = true;
            mDisConnCloudTime = SystemClock::uptimeMillis();
        }
        mIsConnCloud = false;
    }
    
    mCallbackList.push_back(CS_TUYA_NET_CHANGE);
    
    LOGI("g_appData.tuyaWifiStatus = %d   status = %d mIsNetDisConn = %d mIsConnCloud = %d",g_appData.tuyaWifiStatus,status,mIsNetDisConn,mIsConnCloud);
}

// 设置多线程数据任务状态
void TuyaOsMgr::setRequsetStatus(bool status){
    mIsRequesting = status;
}

// 解绑涂鸦
// isFactory：是否恢复出厂设置
void TuyaOsMgr::unBindTuya(bool isFactory){
    tuya_unbind_device(isFactory);
}

//  上报咖啡的DIY数据
void TuyaOsMgr::reportCoffeeDiyRawData(uint8_t type,     uint8_t mode,       uint8_t temp, uint8_t water, 
                             uint8_t soakTime, uint8_t hotWater,   uint8_t hotWaterTemp,
                             std::vector<MasEspStepDataStr> MasEspStepDataList,
                             std::vector<FormStepDataStr>   HandWashStepDataList){

    uint8_t *rawData;
    int len = 0;
    if(type == COFFEE_TYPE_NORMAL){
        len = 7;
        rawData = (uint8_t *)calloc(len,sizeof(uint8_t));
    }else if(type == COFFEE_TYPE_MAS){
        len = 17;
        rawData = (uint8_t *)calloc(len,sizeof(uint8_t));
        for(int i=0; i<5; i++){
            if(i < MasEspStepDataList.size()){
                rawData[ 7+ i*2 ]       = MasEspStepDataList.at(i).pressure;
                rawData[ 7+ i*2 + 1 ]   = MasEspStepDataList.at(i).water;
            }else{
                rawData[7+ i*2]     = 0x00;
                rawData[7+ i*2 + 1] = 0x00;
            }
        }
    }else if(type == COFFEE_TYPE_HANDWASH){
        len = 19;
        rawData = (uint8_t *)calloc(len,sizeof(uint8_t));
        for(int i=0;i<4;i++){
            if(i < HandWashStepDataList.size()){
                rawData[ 7+ i*3 ]       = HandWashStepDataList.at(i).water;
                rawData[ 7+ i*3 + 1 ]   = HandWashStepDataList.at(i).flowRate;
                rawData[ 7+ i*3 + 2 ]   = HandWashStepDataList.at(i).interTime;
            }else{
                rawData[ 7+ i*3 ]       = 0x00;
                rawData[ 7+ i*3 + 1 ]   = 0x00;
                rawData[ 7+ i*3 + 2 ]   = 0x00;
            }
        }
    }

    rawData[0] = type;
    rawData[1] = mode;
    rawData[2] = temp;
    rawData[3] = water;
    rawData[4] = soakTime;
    rawData[5] = hotWater;
    rawData[6] = hotWaterTemp;

    TY_RAW_DP_S *rawDp = new TY_RAW_DP_S;
    rawDp->dpid = TYCMD_COFFEE_DIY;
    rawDp->data = rawData;
    rawDp->len  = len;

    LOG(INFO) << "hex str=" << hexstr(rawData, len);

    // 是否需要上报，联网成功才需要上报
    if(!mIsInit || !mIsConnCloud || (g_appData.tuyaWifiStatus != PRO_STATE_TUYA_GB_CLOUD_CONN)){
        free(mRawDpCacheData);
        mRawDpCacheData = rawDp;
        LOGE("reportCoffeeDiyRawData  添加到缓存队列");
        return;
    }

    ThreadPool::ins()->add(new ReportRawDpThread, rawDp, true);
}

// 上报萃茶的DIY数据
void TuyaOsMgr::reportTeaDiyRawData(uint8_t type, uint8_t mode, uint8_t temp, ExtractTeaSndModeDataStr teaStepData){
    uint8_t *rawData;
    int len = 0;

    if(type == MAKE_TEA){
        len = 6;
        rawData = (uint8_t *)calloc(len,sizeof(uint8_t));
        rawData[3] = teaStepData.makeTeaWater;
        rawData[4] = teaStepData.makeTeaFlowRate;
        rawData[5] = teaStepData.makeTeaInterTime;
    }else if(type == EXTRACTING_TEA){
        len = 12;
        rawData = (uint8_t *)calloc(len,sizeof(uint8_t));
        for(int i=0; i<5; i++){
            rawData[3] = teaStepData.washTeaWater;
            rawData[4] = teaStepData.washTeaFlowRate;
            rawData[5] = teaStepData.washTeaInterTime;

            rawData[6] = teaStepData.soakTeaWater;
            rawData[7] = teaStepData.soakTeaFlowRate;
            rawData[8] = teaStepData.soakTeaInterTime;

            rawData[9] = teaStepData.makeTeaWater;
            rawData[10]= teaStepData.makeTeaFlowRate;
            rawData[11]= teaStepData.makeTeaInterTime;
        }
    }

    rawData[0] = type;
    rawData[1] = mode;
    rawData[2] = temp;

    TY_RAW_DP_S *rawDp = new TY_RAW_DP_S;
    rawDp->dpid = TYCMD_TEA_DIY;
    rawDp->data = rawData;
    rawDp->len  = len;

    LOG(INFO) << "hex str=" << hexstr(rawData, len);

    // 是否需要上报，联网成功才需要上报
    if(!mIsInit || !mIsConnCloud || (g_appData.tuyaWifiStatus != PRO_STATE_TUYA_GB_CLOUD_CONN)){
        free(mRawDpCacheData);
        mRawDpCacheData = rawDp;
        LOGE("reportTeaDiyRawData  添加到缓存队列");
        return;
    }
    

    ThreadPool::ins()->add(new ReportRawDpThread, rawDp, true);
}

// 上报正常的DP数据
void TuyaOsMgr::reportDpData(unsigned char dpid, unsigned char type,void *data){
    TY_OBJ_DP_S p_dp_obj;

    // 是否需要上报，联网成功才需要上报
    bool isNeedReport = !(!mIsInit || !mIsConnCloud || (g_appData.tuyaWifiStatus != PRO_STATE_TUYA_GB_CLOUD_CONN));
    auto it = mTuyaDpList.begin();
    for(; it != mTuyaDpList.end();it++){ if(it->dpid == dpid) break; }  // 找到本地上传的dp数据
    bool isChange = false;  // 判断本地上传的dp数据是否有变化

    p_dp_obj.dpid = dpid;
    p_dp_obj.type = type;
    switch(type){
        case PROP_BOOL:{
            p_dp_obj.value.dp_bool = *static_cast<bool*>(data);
            if(isNeedReport && (it != mTuyaDpList.end()) && (it->value.dp_bool != p_dp_obj.value.dp_bool)) isChange = true;
            LOGI("report dp dpid = %d   bool = %s  isNeedReport = %d isChange = %d",dpid,p_dp_obj.value.dp_bool?"true":"false",isNeedReport,isChange);
            break;
        }
        case PROP_VALUE:{
            p_dp_obj.value.dp_value = *static_cast<int*>(data);
            if(isNeedReport && (it != mTuyaDpList.end()) && (it->value.dp_value != p_dp_obj.value.dp_value)) isChange = true;
            LOGI("report dp dpid = %d   value = %d  isNeedReport = %d isChange = %d",dpid,p_dp_obj.value.dp_value,isNeedReport,isChange);
            break;
        }   
        case PROP_STR:{
            p_dp_obj.value.dp_str = static_cast<char*>(data);
            if(isNeedReport && (it != mTuyaDpList.end()) && strcmp(it->value.dp_str,p_dp_obj.value.dp_str)) isChange = true;
            LOGI("report dp dpid = %d   str = %s  isNeedReport = %d isChange = %d",dpid,p_dp_obj.value.dp_str,isNeedReport,isChange);
            break;
        }
        case PROP_ENUM:{
            p_dp_obj.value.dp_enum = *static_cast<int*>(data);
            if(isNeedReport && (it != mTuyaDpList.end()) && (it->value.dp_enum != p_dp_obj.value.dp_enum)) isChange = true;
            LOGI("report dp dpid = %d   enum = %d isNeedReport = %d isChange = %d",dpid,p_dp_obj.value.dp_enum,isNeedReport,isChange);
            break;
        }
        case PROP_BITMAP:{
            p_dp_obj.value.dp_bitmap = *static_cast<int*>(data);
            if(isNeedReport && (it != mTuyaDpList.end()) && (it->value.dp_bitmap != p_dp_obj.value.dp_bitmap)) isChange = true;
            LOGI("report dp dpid = %d   bitmap = %d isNeedReport = %d isChange = %d",dpid,p_dp_obj.value.dp_bitmap,isNeedReport,isChange);
            break;
        }
    }

    if(!isNeedReport){
        for(auto &item : mTuyaDpCacheList){
            if(item.dpid == dpid){
                item.value = p_dp_obj.value;
                LOGI(" mTuyaDpCacheList ---- 修改 dp[%d] 的数据 ---- ",dpid);
                return;
            }
        }
        LOGI(" mTuyaDpCacheList **** 增加 dp[%d] 的缓存 **** ",dpid);
        mTuyaDpCacheList.push_back(p_dp_obj);
        return;
    }

    if(it == mTuyaDpList.end()){
        mTuyaDpList.push_back(p_dp_obj);
        LOGI(" mTuyaDpList #### 增加 dp[%d] 的缓存 #### ",dpid);
    }else{
        // 这些DP点不做差值处理
        // if(dpid == TYCMD_WORK_ALARM || dpid == TYCMD_FAULT) isChange = true;
        if(isChange){
            it->value = p_dp_obj.value;
            LOGV(" mTuyaDpList #### 修改 dp[%d] 的数据 #### ",dpid);
        }else{
            LOGI("dp[%d] 值未变化，无需重复上报.",dpid);
            return;
        }
    }

    if((dpid == TYCMD_WORK_STATE) && ((p_dp_obj.value.dp_enum == ES_GRIND_DONE)||(p_dp_obj.value.dp_enum == ES_EXTRACT_DONE)||(p_dp_obj.value.dp_enum == ES_CLEAN_CAL_DONE))){
        mNextReportStandbyTime = SystemClock::uptimeMillis() + 3*1000;
        LOGE("上报了制作完成状态 [%d] 并在3s后发送 ES_STANDBY 工作状态",p_dp_obj.value.dp_enum);
    }
    tuya_report_dp(p_dp_obj);
}

void TuyaOsMgr::reportAllStatus(){
    /* 故障警告、自动清洗、恢复出厂设置 */

    bool powerSwitch = true;
    int extEspTotal = g_objConf->getEspTotal();
    int extAmericanoTotal = g_objConf->getAmericanoTotal();
    int extHandWashTotal = g_objConf->getHandWashTotal();
    int extMasEspTotal = g_objConf->getMasEspTotal();
    int extTeaTotal = g_objConf->getTeaTotal();
    int grindTotal = g_objConf->getGrindTotal();
    int steamTotal = g_objConf->getFrothTotal();
    int hotwaterTotal = g_objConf->getHotWaterTotal();
    reportDpData(TYCMD_SWITCH,PROP_BOOL,&powerSwitch); // 开机
    reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
    reportDpData(TYCMD_NUM_ESPRESSO,PROP_VALUE, &extEspTotal); // 意式咖啡萃取次数
    reportDpData(TYCMD_NUM_AMERICANO,PROP_VALUE, &extAmericanoTotal); // 美式咖啡萃取次数
    reportDpData(TYCMD_NUM_POUR,PROP_VALUE, &extHandWashTotal); // 手冲咖啡萃取次数
    reportDpData(TYCMD_NUM_MASTER,PROP_VALUE, &extMasEspTotal); // 大师咖啡萃取次数
    reportDpData(TYCMD_NUM_EXTRACT_TEA,PROP_VALUE, &extTeaTotal); // 萃茶萃取次数
    reportDpData(TYCMD_NUM_GRIND,PROP_VALUE, &grindTotal); // 研磨次数
    reportDpData(TYCMD_NUM_MILK,PROP_VALUE, &steamTotal); // 打奶泡次数
    reportDpData(TYCMD_NUM_HOT,PROP_VALUE, &hotwaterTotal); // 出热水次数
    reportDpData(TYCMD_GRAIN_UNIT_MODE,PROP_ENUM, &g_appData.beanGrindMode); // 工作状态
    
    if(g_appData.warnShowType){
        if((g_appData.warnShowType == A01) || (g_appData.warnShowType == A02) || (g_appData.warnShowType == A03)){
            int reportAlarm = 0x00;
            if(g_appData.warnShowType == A01) reportAlarm = 0x01;
            else if(g_appData.warnShowType == A02) reportAlarm = 0x02;
            else if(g_appData.warnShowType == A03) reportAlarm = 0x04;
            reportDpData(TYCMD_WORK_ALARM,PROP_BITMAP, &reportAlarm); // 提醒类故障
        }else{
            reportDpData(TYCMD_FAULT,PROP_BITMAP, &g_appData.warnShowType);    // 故障警告
        }
    }
    bool isNeedReportMake = true;
    bool isNeedReportClean = true;
    bool isNeedReportBean = true;
    for(auto DpData: mTuyaDpCacheList){
        if(DpData.type == PROP_BOOL)        reportDpData(DpData.dpid,DpData.type, &DpData.value.dp_bool);
        else if(DpData.type == PROP_VALUE)  reportDpData(DpData.dpid,DpData.type, &DpData.value.dp_value);
        else if(DpData.type == PROP_STR)    reportDpData(DpData.dpid,DpData.type, &DpData.value.dp_str);
        else if(DpData.type == PROP_ENUM)   reportDpData(DpData.dpid,DpData.type, &DpData.value.dp_enum);
        else if(DpData.type == PROP_BITMAP) reportDpData(DpData.dpid,DpData.type, &DpData.value.dp_bitmap);
        if(DpData.dpid == TYCMD_MAKEING_START) isNeedReportMake = false;
        if(DpData.dpid == TYCMD_CLEANING_START) isNeedReportClean = false;
        if(DpData.dpid == TYCMD_BEAN_START) isNeedReportBean = false;
    }
    bool isStart;
    if(isNeedReportMake){
        isStart = (g_appData.machineState & MC_STATE_EXT) || (g_appData.machineState & MC_STATE_HOT_WATER);
        g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
    }
    if(isNeedReportClean){
        isStart = (g_appData.machineState & MC_STATE_CLEAN_WATER) || (g_appData.machineState & MC_STATE_CLEAN_CAL);
        g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); 
    }

    if(isNeedReportBean){
        isStart = (g_appData.machineState & MC_STATE_POWDER);
        g_tuyaOsMgr->reportDpData(TYCMD_BEAN_START,PROP_BOOL, &isStart);
    }
    if(mRawDpCacheData){
        ThreadPool::ins()->add(new ReportRawDpThread, mRawDpCacheData, true);
        mRawDpCacheData = nullptr;
    }
    mTuyaDpCacheList.clear();
}

void TuyaOsMgr::onGetQrcode(const char *qrcode){
    g_appData.tuyaQrcode = std::string(qrcode);
    g_objConf->setTuyaQrCode(g_appData.tuyaQrcode);
    LOGI("g_appData.tuyaQrcode = %s",g_appData.tuyaQrcode.c_str());
}


int RequestWeatherThread::onTask(void* data){
    int64_t &dat = *static_cast<int64_t*>(data);
    g_tuyaOsMgr->setRequsetStatus(true);
    int64_t curr_tick = SystemClock::uptimeMillis();
    int temp,conditionNum,expiration;
    LOGE("tuya_get_weather");
    int ret = tuya_get_weather(conditionNum,temp,expiration);
    LOGE("end tuya_get_weather");
    if(ret == 0 && (conditionNum >= 101 && conditionNum <= 146)){
        g_appData.weatherConImage = WeatherImgMap.at(conditionNum).weatherImg;
        g_appData.weatherTemp = temp;
        dat = curr_tick + expiration *60 * 1000;
        LOGI("conditionNum = %d g_appData.weatherConImage = %s g_appData.weatherTemp = %d",conditionNum,g_appData.weatherConImage.c_str(),g_appData.weatherTemp);
    }
    return 0;
}

void RequestWeatherThread::onMain(void *data) {
    g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WEATHER);
    g_tuyaOsMgr->setRequsetStatus(false);
}

int RequestTimeThread::onTask(void* data){
    g_tuyaOsMgr->setRequsetStatus(true);
    time_t timeTick;
    int ret = tuya_get_time(timeTick);
    struct tm tm = *localtime(&timeTick);
    if(tm.tm_year + 1900 >= 2023){
        timeSet(tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
    }

    LOGE("time:%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return 0;
}

void RequestTimeThread::onMain(void *data) {
    g_tuyaOsMgr->setRequsetStatus(false);
    g_appData.statusChangeFlag |= CS_SETPAGE_SYNC_TIME;
    g_windMgr->updateDate();
    g_windMgr->changeTitleBar(PageBase::TITLE_BAR_TIME);
}

int ReportRawDpThread::onTask(void* data){
    tuya_report_raw_dp(*(TY_RAW_DP_S *)data);
    return 0;
}

void ReportRawDpThread::onMain(void *data) {
    LOGE("ReportRawDpThread end");

    free(((TY_RAW_DP_S *)data)->data);
    free((TY_RAW_DP_S *)data);
    LOGE("ReportRawDpThread end");
}
#endif