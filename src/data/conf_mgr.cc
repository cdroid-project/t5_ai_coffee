/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:49:18
 * @FilePath: /t5_ai_coffee/src/data/conf_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


#include "conf_mgr.h"
#include "data.h"
#include "wifi_adapter.h"
#include "curldownload.h"
#include "ghc/filesystem.hpp"
#include "tcp_conn_mgr.h"
#include "base_data.h"
#include "wind_mgr.h"

#include <core/app.h>

#define SECTION_INIT     "init"
#define SECTION_AUTH     "tuyaAuth"

CConfMgr::CConfMgr(){
    mNextBakTick    = 0;
    mNextModeBakTick= 0;
    mNextFavBakTick = 0;
    mUpdates        = 0;
    mIsSaveModeBak  = false;
    mIsSaveFavBak   = false;
}

int CConfMgr::init()
{
    std::string bakFileName,tuyaBakFileName;

#ifdef CDROID_X64
    mFileName         = "./apps/" + App::getInstance().getName()+ "/" + App::getInstance().getName() + ".xml";
    mAuthFileName     = "./apps/" + App::getInstance().getName()+ "/" + "tuyaAuth.xml";
    mFavJsonFileName  = "./apps/" + App::getInstance().getName()+ "/" + "favorites.json";
    mModeJsonFileName = "./apps/" + App::getInstance().getName()+ "/" + "mode.json";

#else
    mFileName       = "/appconfigs/" + App::getInstance().getName() + ".xml";
    mAuthFileName   = "/appconfigs/tuyaAuth.xml";
    mFavJsonFileName = "/appconfigs/favorites.json";
    mModeJsonFileName = "/appconfigs/mode.json";
#endif
    bakFileName = mFileName + ".bak";
    tuyaBakFileName = mAuthFileName + ".bak";

    LOG(DEBUG) << "load config. file=" << mFileName;
    
    // 加载涂鸦授权码
    if(access(mAuthFileName.c_str(), F_OK) == 0){
        mAuthPrefer.load(mAuthFileName);
    }else if(access(tuyaBakFileName.c_str(), F_OK) == 0){
        mAuthPrefer.load(tuyaBakFileName);
    }else{
        mAuthPrefer.load(mAuthFileName);
    }
    
    // 加载程序的基本数据
    if (access(mFileName.c_str(), F_OK) == 0) {
        mPrefer.load(mFileName);
    } else if (access(bakFileName.c_str(), F_OK) == 0) {
        mPrefer.load(bakFileName);
    }
    if ((getVolume() < 0 || getLight() < 0) && access(bakFileName.c_str(), F_OK) == 0){
        // 声音和亮度为0，配置文件可能损坏
        mPrefer.load(bakFileName);
    }

    // 配置文件损坏，重新创建
    if (getVolume() < 0 || getLight() < 0) {
        createConfig();
        mPrefer.load(mFileName);
    }

    // 加载 收藏 模式数据
    loadFavJsonData();
    // 加载模式 步骤数据（大师浓缩、收藏咖啡）
    loadModeJsonData();
    
    // 判断是否需要初始化
    if(isInitSet()){
        createConfig();
        setInitSetFlag(false);
        mPrefer.load(mFileName);
    }

    g_appData.isMute = getMuteFlag();
    g_appData.light  = getLight();
    g_appData.volume = getVolume();
    g_appData.isBeta = getBetaVersion();
    g_appData.AutoChildLocked = getAutoChildLockFlag();

    g_appData.beanGrindMode = getBeanGrindMode();
    g_appData.steamMode = getSteamMode();
    g_appData.steamTemp = getSteamTemp();
    g_appData.hotWaterTemp = getHotWaterTemp();
    g_appData.hotWater = getHotWater();

    g_appData.tuyaQrcode = getTuyaQrCode();
    setScreenLight(g_appData.light);

    // getWifiInfo(connWifiData.name,connWifiData.key);
    // if(!connWifiData.name.empty()){
    //     LOGE("connWifiData.name = %s",connWifiData.name.c_str());
    //     g_wifiMgr->delayConnWifi(connWifiData,10*1000);
    // }

#ifdef CDROID_X64
    g_appData.displayMode = 1;
#else
    g_appData.displayMode = getenv("DISPLAY_MODE") && (atoi(getenv("DISPLAY_MODE")) == 1);
#endif
    if(g_appData.displayMode){
        g_appData.coffeePreheatPtc = 100;
    }

    return 0;
}

void CConfMgr::createConfig(){
    FILE *pf = fopen(mFileName.c_str(), "wb");
    if (!pf) return;

    char buf[] = {
        "<sections>\n"
        "  <section name=\"init\" desc=\"初始信息\">\n"
        "    <item name=\"init_set\" desc=\"是否初始化设置\">false</item>\n"
        "    <item name=\"auto_child_lock\" desc=\"童锁\">true</item>\n"
        "    <item name=\"mute\" desc=\"静音\">false</item>\n"
        "    <item name=\"volume\" desc=\"音量\">50</item>\n"
        "    <item name=\"light\" desc=\"亮度\">50</item>\n"
        "    <item name=\"frothTotal\" desc=\"打奶泡总次数\">0</item>\n" 
        "    <item name=\"hotWaterTotal\" desc=\"出热水总次数\">0</item>\n"
        "    <item name=\"grindTotal\" desc=\"研磨总次数\">0</item>\n"
        "    <item name=\"espressoTotal\" desc=\"意式咖啡总杯数\">0</item>\n"
        "    <item name=\"americanoTotal\" desc=\"美式咖啡总杯数\">0</item>\n"
        "    <item name=\"masEspTotal\" desc=\"大师浓缩咖啡总杯数\">0</item>\n"
        "    <item name=\"handWashTotal\" desc=\"手冲咖啡总杯数\">0</item>\n"
        "    <item name=\"teaTotal\" desc=\"萃茶总杯数\">0</item>\n"
        "    <item name=\"hotWaterTemp\" desc=\"热水温度\">60</item>\n"
        "    <item name=\"hotWater\" desc=\"热水出水量\">150</item>\n"
        "    <item name=\"steamMode\" desc=\"蒸汽模式\">0</item>\n"
        "    <item name=\"steamTemp\" desc=\"蒸汽温度\">65</item>\n"
        "    <item name=\"beanGrindMode\" desc=\"磨豆模式\">0</item>\n"
        "    <item name=\"tuyaQrCode\">none</item>\n"
        "    <item name=\"wifi_name\"></item>\n"
        "    <item name=\"wifi_passwd\"></item>\n"
        "  </section>\n"
        "</sections>\n"
    };

    fwrite(buf, strlen(buf), 1, pf);

    fflush(pf);
    fclose(pf);
}

void CConfMgr::getHomeTabList(std::vector<HomePageTabDataStr> &tabList){
    tabList.clear();
    HomePageTabDataStr tabData;
    tabData.modeType = HOME_MT_AMERICANO;
    tabData.modeName = mAmericanData.name;
    tabList.push_back(tabData);

    tabData.modeType = HOME_MT_ESPRESSO;
    tabData.modeName = mEspData.name;
    tabList.push_back(tabData);

    tabData.modeType = HOME_MT_HAND_WASHED;
    tabData.modeName = mHandWashData.name;
    tabList.push_back(tabData);

    tabData.modeType = HOME_MT_MASTER_ESPRESSO;
    tabData.modeName = mMasEspData.name;
    tabList.push_back(tabData);

    tabData.modeType = HOME_MT_EXTRACTING_TEA;
    tabData.modeName = mExtTeaData.name;
    tabList.push_back(tabData);

    tabData.modeType = HOME_MT_FAVORITES;
    tabData.modeName = "收藏";
    tabList.push_back(tabData);
}

EspDataStr CConfMgr::getEspData(bool isDefaultData){
    return isDefaultData?mEspDefaultData:mEspData;
}

void CConfMgr::setEspData(EspDataStr data){
    mEspData = data;
    saveModeJsonData(false);
}

AmericanDataStr CConfMgr::getAmericanData(bool isDefaultData){
    return isDefaultData?mAmericanDefaultData:mAmericanData;
}

void CConfMgr::setAmericanData(AmericanDataStr data){
    mAmericanData = data;
    saveModeJsonData(false);
}

MasEspDataStr CConfMgr::getMasEspData(bool isDefaultData){
    return isDefaultData?mMasEspDefaultData:mMasEspData;
}

void CConfMgr::setMasEspData(MasEspDataStr data){
    mMasEspData = data;
    saveModeJsonData(false);
}

HandWashDataStr CConfMgr::getHandWashData(bool isDefaultData){
    return isDefaultData?mHandWashDefaultData:mHandWashData;
}

void CConfMgr::setHandWashData(HandWashDataStr data){
    mHandWashData = data;
    saveModeJsonData(false);
}

ExtractTeaDataStr CConfMgr::getExtractTeaData(bool isDefaultData){
    return isDefaultData?mExtTeaDefaultData:mExtTeaData;
}

void CConfMgr::setExtractTeaData(ExtractTeaDataStr data){
    mExtTeaData = data;
    saveModeJsonData(false);
}

Json::Value CConfMgr::getFavModeList(){
    return FavJsonValue;
}
void CConfMgr::setFavModeList(Json::Value list){
    FavJsonValue = list;
    saveFavJsonData(false);
}
void CConfMgr::addFavModeData(Json::Value data){
    FavJsonValue.insert(0,data);
    saveFavJsonData(false);
}
void CConfMgr::deleteFavModeData(int Pos){
    Json::Value deleteData;
    FavJsonValue.removeIndex(Pos,&deleteData);
    saveFavJsonData(false);
}
void CConfMgr::deleteFavModeData(Json::Value data){
    for(int i=0; i<FavJsonValue.size(); i++){
        if(areJsonValuesEqual(FavJsonValue[i],data)){
            FavJsonValue.removeIndex(i,&deleteData);
            saveFavJsonData(false);
            return;
        }
    }
}
void CConfMgr::editFavModeData(Json::Value data,int Pos){
    FavJsonValue[Pos] = data;
    saveFavJsonData(false);
}

bool CConfMgr::checkFavModeData(Json::Value data){
    for(int i=0; i<FavJsonValue.size(); i++){
        Json::Value &jsonData = FavJsonValue[i];
        if(areJsonValuesEqual(FavJsonValue[i],data)){
            return true;
        }
    }
    return false;
}

void CConfMgr::loadModeJsonData(){
    Json::Value ModeJsonValue(Json::objectValue);
    Json::Value ModeDefaultJsonValue(Json::objectValue);
    if(!convertStringToJson(__local_mode_data__,ModeDefaultJsonValue)){
        LOGE("load base mode data failed!!!!");
        return;
    }

    if(isInitSet() || !(loadLocalJson(mModeJsonFileName,ModeJsonValue) || loadLocalJson(mModeJsonFileName+".bak",ModeJsonValue))){
        ModeJsonValue = ModeDefaultJsonValue;
        mIsSaveModeBak = true;
        saveLocalJson(mModeJsonFileName, ModeJsonValue);
        mNextModeBakTick = SystemClock::uptimeMillis()+10*1000;
        LOGI("load base mode data --- *-* isInitSet = %d",isInitSet());
    }
    loadModeDataFromJson(ModeJsonValue,mEspData,mAmericanData,mMasEspData,mHandWashData,mExtTeaData);
    loadModeDataFromJson(ModeDefaultJsonValue,mEspDefaultData,mAmericanDefaultData,mMasEspDefaultData,mHandWashDefaultData,mExtTeaDefaultData);
}

void CConfMgr::loadFavJsonData(){
    if(isInitSet() || !(loadLocalJson(mFavJsonFileName,FavJsonValue) || loadLocalJson(mFavJsonFileName+".bak",FavJsonValue))){
        FavJsonValue.clear();
        saveFavJsonData();
        mNextFavBakTick = SystemClock::uptimeMillis()+10*1000;
        mIsSaveFavBak = true;
        LOGI("load favorites list failed --- *-* isInitSet = %d",isInitSet()); 
    }
}

void CConfMgr::saveModeJsonData(bool isBak){
    Json::Value ModeJsonValue(Json::objectValue);

    // 意式咖啡
    Json::Value espData(Json::objectValue);
    espData["pic"] = mEspData.pic;
    espData["name"] = mEspData.name;
    espData["powderMin"] = mEspData.powderMin;
    espData["powderMax"] = mEspData.powderMax;
    espData["powderGear"] = mEspData.powderGear;
    espData["extractTempMin"] = mEspData.extractTempMin;
    espData["extractTempMax"] = mEspData.extractTempMax;
    espData["extractTempGear"] = mEspData.extractTempGear;
    espData["extractWaterMin"] = mEspData.extractWaterMin;
    espData["extractWaterMax"] = mEspData.extractWaterMax;
    espData["extractWaterGear"] = mEspData.extractWaterGear;
    espData["soakingTimeMin"] = mEspData.soakingTimeMin;
    espData["soakingTimeMax"] = mEspData.soakingTimeMax;
    espData["soakingTimeGear"] = mEspData.soakingTimeGear;
    Json::Value espSndData(Json::objectValue);
    espSndData["powderDef"] = mEspData.sndData.powderDef;
    espSndData["extractTempDef"] = mEspData.sndData.extractTempDef;
    espSndData["extractWaterDef"] = mEspData.sndData.extractWaterDef;
    espSndData["soakingTimeDef"] = mEspData.sndData.soakingTimeDef;
    espData["sndModeData"] = espSndData;

    // 美式咖啡
    Json::Value americanData(Json::objectValue);
    americanData["pic"] = mAmericanData.pic;
    americanData["name"] = mAmericanData.name;
    americanData["powderMin"] = mAmericanData.powderMin;
    americanData["powderMax"] = mAmericanData.powderMax;
    americanData["powderGear"] = mAmericanData.powderGear;
    americanData["extractTempMin"] = mAmericanData.extractTempMin;
    americanData["extractTempMax"] = mAmericanData.extractTempMax;
    americanData["extractTempGear"] = mAmericanData.extractTempGear;
    americanData["extractWaterMin"] = mAmericanData.extractWaterMin;
    americanData["extractWaterMax"] = mAmericanData.extractWaterMax;
    americanData["extractWaterGear"] = mAmericanData.extractWaterGear;
    americanData["soakingTimeMin"] = mAmericanData.soakingTimeMin;
    americanData["soakingTimeMax"] = mAmericanData.soakingTimeMax;
    americanData["soakingTimeGear"] = mAmericanData.soakingTimeGear;
    americanData["hotWaterTempMin"] = mAmericanData.hotWaterTempMin;
    americanData["hotWaterTempMax"] = mAmericanData.hotWaterTempMax;
    americanData["hotWaterTempGear"] = mAmericanData.hotWaterTempGear;
    americanData["hotWaterMin"] = mAmericanData.hotWaterMin;
    americanData["hotWaterMax"] = mAmericanData.hotWaterMax;
    americanData["hotWaterGear"] = mAmericanData.hotWaterGear;

    Json::Value americanSndData(Json::objectValue);
    americanSndData["powderDef"] = mAmericanData.sndData.powderDef;
    americanSndData["extractTempDef"] = mAmericanData.sndData.extractTempDef;
    americanSndData["extractWaterDef"] = mAmericanData.sndData.extractWaterDef;
    americanSndData["soakingTimeDef"] = mAmericanData.sndData.soakingTimeDef;
    americanSndData["hotWaterDef"] = mAmericanData.sndData.hotWaterDef;
    americanSndData["hotWaterTempDef"] = mAmericanData.sndData.hotWaterTempDef;
    americanData["sndModeData"] = americanSndData;

    // 大师浓缩
    Json::Value masEspData(Json::objectValue);
    masEspData["name"] = mMasEspData.name;
    masEspData["powderMin"] = mMasEspData.powderMin;
    masEspData["powderMax"] = mMasEspData.powderMax;
    masEspData["powderGear"] = mMasEspData.powderGear;
    masEspData["extractTempMin"] = mMasEspData.extractTempMin;
    masEspData["extractTempMax"] = mMasEspData.extractTempMax;
    masEspData["extractTempGear"] = mMasEspData.extractTempGear;
    masEspData["extractWaterMin"] = mMasEspData.extractWaterMin;
    masEspData["extractWaterMax"] = mMasEspData.extractWaterMax;
    masEspData["extractWaterGear"] = mMasEspData.extractWaterGear;
    masEspData["soakingTimeMin"] = mMasEspData.soakingTimeMin;
    masEspData["soakingTimeMax"] = mMasEspData.soakingTimeMax;
    masEspData["soakingTimeGear"] = mMasEspData.soakingTimeGear;

    Json::Value masEspSndList(Json::arrayValue);
    for(int i=0; i<mMasEspData.sndModeList.size(); i++){
        MasEspSndModeDataStr sndModeData = mMasEspData.sndModeList[i];
        Json::Value jsonSndModeData;
        jsonSndModeData["sndModename"] = sndModeData.sndModename;
        jsonSndModeData["sndModeType"] = sndModeData.sndModeType;
        jsonSndModeData["powderDef"] = sndModeData.powderDef;
        jsonSndModeData["extractTempDef"] = sndModeData.extractTempDef;
        jsonSndModeData["soakingTimeDef"] = sndModeData.soakingTimeDef;
        Json::Value jsonSndModeStepList(Json::arrayValue);
        for(int k=0; k<sndModeData.stepDataList.size(); k++){
            Json::Value jsonSndModeStepData;
            jsonSndModeStepData["pressure"] = sndModeData.stepDataList.at(k).pressure;
            jsonSndModeStepData["water"] = sndModeData.stepDataList.at(k).water;
            jsonSndModeStepList.append(jsonSndModeStepData);
        }
        jsonSndModeData["sndModeStepList"] = jsonSndModeStepList;
        masEspSndList.append(jsonSndModeData);
    }
    masEspData["sndModeList"] = masEspSndList;

    //手冲咖啡
    Json::Value handWashData(Json::objectValue);
    handWashData["name"] = mHandWashData.name;
    handWashData["powderMin"] = mHandWashData.powderMin;
    handWashData["powderMax"] = mHandWashData.powderMax;
    handWashData["powderGear"] = mHandWashData.powderGear;
    handWashData["extractTempMin"] = mHandWashData.extractTempMin;
    handWashData["extractTempMax"] = mHandWashData.extractTempMax;
    handWashData["extractTempGear"] = mHandWashData.extractTempGear;
    handWashData["extractWaterMin"] = mHandWashData.extractWaterMin;
    handWashData["extractWaterMax"] = mHandWashData.extractWaterMax;
    handWashData["extractWaterGear"] = mHandWashData.extractWaterGear;
    handWashData["interTimeMin"] = mHandWashData.interTimeMin;
    handWashData["interTimeMax"] = mHandWashData.interTimeMax;
    handWashData["interTimeGear"] = mHandWashData.interTimeGear;

    Json::Value handWashSndList(Json::arrayValue);
    for(int i=0; i<mHandWashData.sndModeList.size(); i++){
        HandWashSndModeDataStr sndModeData = mHandWashData.sndModeList[i];
        Json::Value jsonSndModeData;
        jsonSndModeData["sndModename"] = sndModeData.sndModename;
        jsonSndModeData["sndModeType"] = sndModeData.sndModeType;
        jsonSndModeData["powderDef"] = sndModeData.powderDef;
        jsonSndModeData["extractTempDef"] = sndModeData.extractTempDef;
        Json::Value jsonSndModeStepList(Json::arrayValue);
        for(int k=0; k<sndModeData.stepDataList.size(); k++){
            Json::Value jsonSndModeStepData;
            jsonSndModeStepData["water"] = sndModeData.stepDataList.at(k).water;
            jsonSndModeStepData["flowRate"] = sndModeData.stepDataList.at(k).flowRate;
            jsonSndModeStepData["interTime"] = sndModeData.stepDataList.at(k).interTime;
            jsonSndModeStepList.append(jsonSndModeStepData);
        }
        jsonSndModeData["sndModeStepList"] = jsonSndModeStepList;
        handWashSndList.append(jsonSndModeData);
    }
    handWashData["sndModeList"] = handWashSndList;

    // 萃茶
    Json::Value extTeaData(Json::objectValue);
    extTeaData["name"] = mExtTeaData.name;
    extTeaData["extractTempMin"] = mExtTeaData.extractTempMin;
    extTeaData["extractTempMax"] = mExtTeaData.extractTempMax;
    extTeaData["extractTempGear"] = mExtTeaData.extractTempGear;
    extTeaData["waterMin"] = mExtTeaData.waterMin;
    extTeaData["waterMax"] = mExtTeaData.waterMax;
    extTeaData["waterGear"] = mExtTeaData.waterGear;
    extTeaData["interTimeMin"] = mExtTeaData.interTimeMin;
    extTeaData["interTimeMax"] = mExtTeaData.interTimeMax;
    extTeaData["interTimeGear"] = mExtTeaData.interTimeGear;

    Json::Value extTeaSndList(Json::arrayValue);
    for(int i=0; i<mExtTeaData.sndModeList.size(); i++){
        ExtractTeaSndModeDataStr sndModeData = mExtTeaData.sndModeList[i];
        Json::Value jsonSndModeData;
        jsonSndModeData["pic"] = sndModeData.pic;
        jsonSndModeData["sndModename"] = sndModeData.sndModename;
        jsonSndModeData["sndModeType"] = sndModeData.sndModeType;
        jsonSndModeData["pressure"] = sndModeData.pressure;
        jsonSndModeData["powderDef"] = sndModeData.powderDef;
        jsonSndModeData["washTeaNumMin"] = sndModeData.washTeaNumMin;
        jsonSndModeData["washTeaNumMax"] = sndModeData.washTeaNumMax;
        jsonSndModeData["extractTempDef"] = sndModeData.extractTempDef;
        jsonSndModeData["washTeaWater"] = sndModeData.washTeaWater;
        jsonSndModeData["washTeaFlowRate"] = sndModeData.washTeaFlowRate;
        jsonSndModeData["washTeaInterTime"] = sndModeData.washTeaInterTime;
        jsonSndModeData["soakTeaWater"] = sndModeData.soakTeaWater;
        jsonSndModeData["soakTeaFlowRate"] = sndModeData.soakTeaFlowRate;
        jsonSndModeData["soakTeaInterTime"] = sndModeData.soakTeaInterTime;
        jsonSndModeData["makeTeaWater"] = sndModeData.makeTeaWater;
        jsonSndModeData["makeTeaFlowRate"] = sndModeData.makeTeaFlowRate;
        jsonSndModeData["makeTeaInterTime"] = sndModeData.makeTeaInterTime;
        extTeaSndList.append(jsonSndModeData);
    }
    extTeaData["sndModeList"] = extTeaSndList;

    ModeJsonValue["espresso"] = espData;
    ModeJsonValue["americano"] = americanData;
    ModeJsonValue["master_espresso"] = masEspData;
    ModeJsonValue["hand_wash"] = handWashData;
    ModeJsonValue["extracting_tea"] = extTeaData;
    if(isBak){
        saveLocalJson(mModeJsonFileName+".bak", ModeJsonValue);
#ifdef CDROID_SIGMA
        sync();
#endif
    }else{
        saveLocalJson(mModeJsonFileName, ModeJsonValue);
        mNextModeBakTick = SystemClock::uptimeMillis()+10*1000;
        mIsSaveModeBak = true;
    }
    return;
}

void CConfMgr::saveFavJsonData(bool isBak){
    if(isBak){
        saveLocalJson(mFavJsonFileName+".bak", FavJsonValue);
#ifdef CDROID_SIGMA
        sync();
#endif
    }else{
        saveLocalJson(mFavJsonFileName, FavJsonValue);
        mNextFavBakTick = SystemClock::uptimeMillis()+10*1000;
        mIsSaveFavBak = true;
    }
    return;
}

void CConfMgr::update()
{
    int64_t     nowTick = SystemClock::uptimeMillis();
    if (mPrefer.getUpdates() > 0){
        mUpdates = mPrefer.getUpdates();
        mPrefer.save(mFileName);
        mNextBakTick = nowTick + 1000 * 10;
        LOG(DEBUG) << "save config. file=" << mFileName;
    }else if(mAuthPrefer.getUpdates() > 0){
        mUpdates = mAuthPrefer.getUpdates();
        mAuthPrefer.save(mAuthFileName);
        mNextBakTick = nowTick + 1000 * 10;
        LOG(DEBUG) << "save config. file=" << mAuthFileName;
    }else if( (mUpdates > 0) && ((nowTick - mNextBakTick) > 0)){
        mPrefer.save(mFileName + ".bak");
        mAuthPrefer.save(mAuthFileName + ".bak");
#ifdef CDROID_SIGMA
        sync();
#endif
        mUpdates = 0;
        mNextBakTick += 1000 * 60;
        LOG(DEBUG) << "save config. bak file=" << mAuthFileName+".bak" << "   " << mFileName+".bak";
    }else if( mIsSaveModeBak && (nowTick - mNextModeBakTick) > 0){
        saveModeJsonData(true);
        mIsSaveModeBak = false;
    }else if( mIsSaveFavBak && (nowTick - mNextFavBakTick) > 0){
        saveFavJsonData(true);
        mIsSaveFavBak = false;
    }
}

// 是否初始话
bool CConfMgr::isInitSet()
{
    return mPrefer.getBool(SECTION_INIT, "init_set", false);
}

void CConfMgr::setInitSetFlag(bool flag)
{
    mPrefer.setValue(SECTION_INIT, "init_set", flag);
}

// 童锁
bool CConfMgr::getAutoChildLockFlag()
{
    return mPrefer.getBool(SECTION_INIT, "auto_child_lock");
}

void CConfMgr::setAutoChildLockFlag(bool flag)
{
    mPrefer.setValue(SECTION_INIT, "auto_child_lock", flag);
}

// 静音
bool CConfMgr::getMuteFlag()
{
    return mPrefer.getBool(SECTION_INIT, "mute");
}

void CConfMgr::setMuteFlag(bool flag)
{
    mPrefer.setValue(SECTION_INIT, "mute", flag);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 音量
int CConfMgr::getVolume()
{
    return mPrefer.getInt(SECTION_INIT, "volume", -1);
}

void CConfMgr::setVolume(int value)
{
    mPrefer.setValue(SECTION_INIT, "volume", value);
}

// 亮度
int CConfMgr::getLight()
{
    return mPrefer.getInt(SECTION_INIT, "light", 50);
}

void CConfMgr::setLight(int value)
{
    if (value < 0) value = 0;
    mPrefer.setValue(SECTION_INIT, "light", value);
}

// 打奶泡 总次数
int  CConfMgr::getFrothTotal(){
    return mPrefer.getInt(SECTION_INIT, "frothTotal", 0);
}
void CConfMgr::setFrothTotal(int value){
    mPrefer.setValue(SECTION_INIT, "frothTotal", value);
}

// 出热水 总次数
int  CConfMgr::getHotWaterTotal(){
    return mPrefer.getInt(SECTION_INIT, "hotWaterTotal", 0);
}
void CConfMgr::setHotWaterTotal(int value){
    mPrefer.setValue(SECTION_INIT, "hotWaterTotal", value);
}

// 研磨 总次数
int  CConfMgr::getGrindTotal(){
    return mPrefer.getInt(SECTION_INIT, "grindTotal", 0);
}
void CConfMgr::setGrindTotal(int value){
    mPrefer.setValue(SECTION_INIT, "grindTotal", value);
}

// 意式咖啡 总次数
int  CConfMgr::getEspTotal(){
    return mPrefer.getInt(SECTION_INIT, "espressoTotal", 0);
}
void CConfMgr::setEspTotal(int value){
    mPrefer.setValue(SECTION_INIT, "espressoTotal", value);
}

// 美式咖啡 总杯数
int  CConfMgr::getAmericanoTotal(){
    return mPrefer.getInt(SECTION_INIT, "americanoTotal", 0);
}
void CConfMgr::setAmericanoTotal(int value){
    mPrefer.setValue(SECTION_INIT, "americanoTotal", value);
}

// 大师浓缩咖啡 总杯数
int  CConfMgr::getMasEspTotal(){
    return mPrefer.getInt(SECTION_INIT, "masEspTotal", 0);
}
void CConfMgr::setMasEspTotal(int value){
    mPrefer.setValue(SECTION_INIT, "masEspTotal", value);
}

// 手冲咖啡 总杯数
int  CConfMgr::getHandWashTotal(){
    return mPrefer.getInt(SECTION_INIT, "handWashTotal", 0);
}
void CConfMgr::setHandWashTotal(int value){
    mPrefer.setValue(SECTION_INIT, "handWashTotal", value);
}

// 萃茶 总次数
int  CConfMgr::getTeaTotal(){
    return mPrefer.getInt(SECTION_INIT, "teaTotal", 0);
}
void CConfMgr::setTeaTotal(int value){
    mPrefer.setValue(SECTION_INIT, "teaTotal", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 热水温度
int  CConfMgr::getHotWaterTemp(){
    return mPrefer.getInt(SECTION_INIT, "hotWaterTemp", 60);
}
void CConfMgr::setHotWaterTemp(int value){
    mPrefer.setValue(SECTION_INIT, "hotWaterTemp", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 热水 出水量
int  CConfMgr::getHotWater(){
    return mPrefer.getInt(SECTION_INIT, "hotWater", 150);
}   
void CConfMgr::setHotWater(int value){
    mPrefer.setValue(SECTION_INIT, "hotWater", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 蒸汽模式
int  CConfMgr::getSteamMode(){
    return mPrefer.getInt(SECTION_INIT, "steamMode", 0);
}
void CConfMgr::setSteamMode(int value){
    mPrefer.setValue(SECTION_INIT, "steamMode", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 蒸汽温度
int  CConfMgr::getSteamTemp(){
    return mPrefer.getInt(SECTION_INIT, "steamTemp", 65);
}
void CConfMgr::setSteamTemp(int value){
    mPrefer.setValue(SECTION_INIT, "steamTemp", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}

// 磨豆模式
int  CConfMgr::getBeanGrindMode(){
    return mPrefer.getInt(SECTION_INIT, "beanGrindMode", 0);
}

void CConfMgr::setBeanGrindMode(int value){
    mPrefer.setValue(SECTION_INIT, "beanGrindMode", value);
#ifdef CDROID_SIGMA
    update();
    sync();
#endif
}


std::string CConfMgr::getDeviceMac(){
#ifndef TUYA_OS_DISABLE
    std::string mac = mPrefer.getString(SECTION_INIT, "mac");
    if (mac.empty()) {
        std::string comm = "ifconfig wlan0 | awk '/HWaddr/{print $5}'";
        mac = sysCommand(comm);
        if (!mac.empty() && mac.back() == '\n') {
            mac.erase(mac.length() - 1);
        }
        if (!mac.empty())setDeviceMac(mac);
    }
#else
    std::string mac = "30:7B:C9:4E:B2:8A";
#endif
    return mac;
}

void CConfMgr::setDeviceMac(const std::string &mac){
    mPrefer.setValue(SECTION_INIT, "mac", mac);
}

std::string CConfMgr::getWifiPasswd() {
    return mPrefer.getString(SECTION_INIT, "wifi_passwd");
}

std::string CConfMgr::getWifiName() {
    return mPrefer.getString(SECTION_INIT, "wifi_name");
}

bool CConfMgr::getWifiInfo(std::string &wifiName, std::string &wifiPasswd) {

    wifiName   = mPrefer.getString(SECTION_INIT, "wifi_name");
    wifiPasswd = mPrefer.getString(SECTION_INIT, "wifi_passwd");

    return !wifiName.empty();
}

void CConfMgr::setWifiInfo(const std::string &wifiName, const std::string &wifiPasswd) {
    mPrefer.setValue(SECTION_INIT, "wifi_name", wifiName);
    mPrefer.setValue(SECTION_INIT, "wifi_passwd", wifiPasswd);
    LOGE("setWifiInfo      wifiName = %s wifiPasswd = %s",wifiName.c_str(),wifiPasswd.c_str());
}

std::string CConfMgr::getTuyaQrCode(){
    return mAuthPrefer.getString(SECTION_AUTH, "tuyaQrCode","");
}
void CConfMgr::setTuyaQrCode(const std::string &qrCode){
    mAuthPrefer.setValue(SECTION_AUTH, "tuyaQrCode", qrCode);
}

bool CConfMgr::getBetaVersion(){
    return mPrefer.getBool(SECTION_INIT, "isBeta");
}
void CConfMgr::setBetaVersion(bool flag){
    mPrefer.setValue(SECTION_INIT, "isBeta", flag);
}

void CConfMgr::reset(){
    // 设置亮度 50
    setLight(50);
    setScreenLight(50);

    // 设置声音
    setVolume(50);
}


void CConfMgr::getFromServerAuth(){
    CurlDownloader::ConnectionData* cnn = new CurlDownloader::ConnectionData("http://tuya.sanbontft.com:10201/getAuthCode?mac=" + getDeviceMac()+"&project=62525", "",
        [this,cnn](int code, char* body, std::string&) {
            if (code) {
                LOGE("fail to get new version ! code = %d", code);
                g_appData.getAuthError = "001";
                g_appData.getAuthStatus = PRO_AUTH_CURL_ERROR;
                g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                g_windMgr->updateDate();
                return;
            }
            if(body == nullptr){
                LOGE("fail to get new version ! code = %d  body is nullptr!!", code);
                g_appData.getAuthError = "001-";
                g_appData.getAuthStatus = PRO_AUTH_CURL_ERROR;
                g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                g_windMgr->updateDate();
                return;
            }
            LOGD("success to get new version");
            Json::Value  root;
            Json::Reader reader(Json::Features::strictMode());
            bool bRet = reader.parse(body, root);
            if (!bRet) {
                LOGE("fail to get new version ! code = %d",bRet);
                g_appData.getAuthError = "002";
                g_appData.getAuthStatus = PRO_AUTH_CURL_ERROR;
                g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                g_windMgr->updateDate();
            } else {
                if (!root["code"].isInt()) { 
                    LOGE("get new version data error !");
                    g_appData.getAuthError = "003";
                    g_appData.getAuthStatus = PRO_AUTH_CURL_ERROR;
                    g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                    g_windMgr->updateDate();
                    return; 
                }
                int code = std::move(root["code"].asInt());
                if (code != 200) { 
                    g_appData.getAuthStatus = PRO_AUTH_ERROR;
                    g_appData.getAuthError = std::move(root["message"].asString());
                    g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                    g_windMgr->updateDate();
                    LOGE("get new version data error !");
                    return; 
                }
                g_appData.getAuthStatus = PRO_AUTH_SUCCESS;
                std::string uuid = std::move(root["data"]["uuid"].asString());
                std::string key = std::move(root["data"]["key"].asString());

                mAuthPrefer.setValue(SECTION_AUTH,"uuid",uuid);
                mAuthPrefer.setValue(SECTION_AUTH,"key",key);
                
                g_appData.statusChangeFlag |= CS_GET_AUTH_STATUS;
                g_windMgr->updateDate();
                update();
#ifdef CDROID_SIGMA
                sync();
#endif
                LOGI("uuid = %s  key = %s",uuid.c_str(),key.c_str());
            }
        }
    );
    g_objCurl->addConnection(cnn);
}

void CConfMgr::getTuyaAuthCode(std::string &authUuid,std::string &authKey){
    authUuid = mAuthPrefer.getString(SECTION_AUTH, "uuid");
    authKey = mAuthPrefer.getString(SECTION_AUTH, "key");
    LOGI("authUuid = %s  authKey = %s",authUuid.c_str(),authKey.c_str());
}

std::string CConfMgr::getTuyaAuthUUID(){
    return mAuthPrefer.getString(SECTION_AUTH, "uuid");
}

void CConfMgr::setTuyaAuthCode(const std::string &authUuid,const std::string &authKey){
    // 暂时不能设置
    mAuthPrefer.setValue(SECTION_AUTH, "uuid", authUuid);
    mAuthPrefer.setValue(SECTION_AUTH, "key", authKey);
    update();
#ifdef CDROID_SIGMA
    sync();
#endif
}


