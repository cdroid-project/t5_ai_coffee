/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:49:09
 * @FilePath: /t5_ai_coffee/src/data/data.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


#include "data.h"
#include "conf_mgr.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
void setScreenLight(int light, bool chk /* = true*/) {
    // /sys/class/pwm/pwmchip0/pwmN/duty_cycle 数值就为pwm占空比，范围[1-99]
    char path_duty_cycle[] = "/sys/class/pwm/pwmchip0/pwm0/duty_cycle";
    if (access(path_duty_cycle, F_OK)) { return; }

    int pwm = (light) * 99 / MAX_SCREEN_LIGHT;
    if (chk) {
        if (pwm > 99) {
            pwm = (MAX_SCREEN_LIGHT - DEF_SCREEN_LIGHT) * 99 / MAX_SCREEN_LIGHT;   // 暗
        }
        if (pwm < 1) {
            pwm = 1;   // 亮
        }
    }

    char shell[128];
    snprintf(shell, sizeof(shell), "echo %d > %s", 100, path_duty_cycle);
    system(shell);
}


std::vector<cityData> cityProvinceList = {
    {8200, "澳门"}, {8100, "香港"},   {7100, "台湾"}, {6500, "新疆"}, {6400, "宁夏"},
    {6300, "青海"}, {6200, "甘肃"},   {6100, "陕西"}, {5400, "西藏"}, {5300, "云南"},
    {5200, "贵州"}, {5100, "四川"},   {5000, "重庆"}, {4600, "海南"}, {4500, "广西"},
    {4400, "广东"}, {4300, "湖南"},   {4200, "湖北"}, {4100, "河南"}, {3700, "山东"},
    {3600, "江西"}, {3500, "福建"},   {3400, "安徽"}, {3300, "浙江"}, {3200, "江苏"}, 
    {3100, "上海"}, {2300, "黑龙江"}, {2200, "吉林"}, {2100, "辽宁"}, {1500, "内蒙古"}, 
    {1400, "山西"}, {1300, "河北"},   {1200, "天津"}, {1100, "北京"}, {0000, "默认地区"}
};

// 读取模式数据
void loadModeDataFromJson(Json::Value fromJsonData,EspDataStr &espStrData,AmericanDataStr &americanStrData,
                            MasEspDataStr &masEspStrData,HandWashDataStr &handWashStrData,ExtractTeaDataStr &extTeaStrData){
    // 意式咖啡 数据
    Json::Value espData = fromJsonData["espresso"];
    espStrData.name = getJsonString(espData,"name");
    espStrData.pic = getJsonString(espData,"pic");
    
    espStrData.powderMin = getJsonInt(espData,"powderMin");
    espStrData.powderMax = getJsonInt(espData,"powderMax");
    espStrData.powderGear = getJsonInt(espData,"powderGear");
    espStrData.extractTempMin = getJsonInt(espData,"extractTempMin");
    espStrData.extractTempMax = getJsonInt(espData,"extractTempMax");
    espStrData.extractTempGear = getJsonInt(espData,"extractTempGear");
    espStrData.extractWaterMin = getJsonInt(espData,"extractWaterMin");
    espStrData.extractWaterMax = getJsonInt(espData,"extractWaterMax");
    espStrData.extractWaterGear = getJsonInt(espData,"extractWaterGear");
    espStrData.soakingTimeMin = getJsonInt(espData,"soakingTimeMin");
    espStrData.soakingTimeMax = getJsonInt(espData,"soakingTimeMax");
    espStrData.soakingTimeGear = getJsonInt(espData,"soakingTimeGear");

    Json::Value espSndData = espData["sndModeData"];
    espStrData.sndData.powderDef = getJsonInt(espSndData,"powderDef");
    espStrData.sndData.extractTempDef = getJsonInt(espSndData,"extractTempDef");
    espStrData.sndData.extractWaterDef = getJsonInt(espSndData,"extractWaterDef");
    espStrData.sndData.soakingTimeDef = getJsonInt(espSndData,"soakingTimeDef");

    // 美式咖啡 数据
    Json::Value americanData = fromJsonData["americano"];
    americanStrData.name = getJsonString(americanData,"name");
    americanStrData.pic = getJsonString(americanData,"pic");
    americanStrData.powderMin = getJsonInt(americanData,"powderMin");
    americanStrData.powderMax = getJsonInt(americanData,"powderMax");
    americanStrData.powderGear = getJsonInt(americanData,"powderGear");
    americanStrData.extractTempMin = getJsonInt(americanData,"extractTempMin");
    americanStrData.extractTempMax = getJsonInt(americanData,"extractTempMax");
    americanStrData.extractTempGear = getJsonInt(americanData,"extractTempGear");
    americanStrData.extractWaterMin = getJsonInt(americanData,"extractWaterMin");
    americanStrData.extractWaterMax = getJsonInt(americanData,"extractWaterMax");
    americanStrData.extractWaterGear = getJsonInt(americanData,"extractWaterGear");
    americanStrData.hotWaterTempMin = getJsonInt(americanData,"hotWaterTempMin");
    americanStrData.hotWaterTempMax = getJsonInt(americanData,"hotWaterTempMax");
    americanStrData.hotWaterTempGear = getJsonInt(americanData,"hotWaterTempGear");
    americanStrData.hotWaterMin = getJsonInt(americanData,"hotWaterMin");
    americanStrData.hotWaterMax = getJsonInt(americanData,"hotWaterMax");
    americanStrData.hotWaterGear = getJsonInt(americanData,"hotWaterGear");
    americanStrData.soakingTimeMin = getJsonInt(americanData,"soakingTimeMin");
    americanStrData.soakingTimeMax = getJsonInt(americanData,"soakingTimeMax");
    americanStrData.soakingTimeGear = getJsonInt(americanData,"soakingTimeGear");

    Json::Value americanSndData = americanData["sndModeData"];
    
    americanStrData.sndData.powderDef = getJsonInt(americanSndData,"powderDef");
    americanStrData.sndData.extractTempDef = getJsonInt(americanSndData,"extractTempDef");
    americanStrData.sndData.extractWaterDef = getJsonInt(americanSndData,"extractWaterDef");
    americanStrData.sndData.soakingTimeDef = getJsonInt(americanSndData,"soakingTimeDef");
    americanStrData.sndData.hotWaterDef = getJsonInt(americanSndData,"hotWaterDef");
    americanStrData.sndData.hotWaterTempDef = getJsonInt(americanSndData,"hotWaterTempDef");
    

    // 大师浓缩
    Json::Value masEspData = fromJsonData["master_espresso"];
    masEspStrData.name = getJsonString(masEspData,"name");
    masEspStrData.powderMin = getJsonInt(masEspData,"powderMin");
    masEspStrData.powderMax = getJsonInt(masEspData,"powderMax");
    masEspStrData.powderGear = getJsonInt(masEspData,"powderGear");
    masEspStrData.extractTempMin = getJsonInt(masEspData,"extractTempMin");
    masEspStrData.extractTempMax = getJsonInt(masEspData,"extractTempMax");
    masEspStrData.extractTempGear = getJsonInt(masEspData,"extractTempGear");
    masEspStrData.extractWaterMin = getJsonInt(masEspData,"extractWaterMin");
    masEspStrData.extractWaterMax = getJsonInt(masEspData,"extractWaterMax");
    masEspStrData.extractWaterGear = getJsonInt(masEspData,"extractWaterGear");
    masEspStrData.soakingTimeMin = getJsonInt(masEspData,"soakingTimeMin");
    masEspStrData.soakingTimeMax = getJsonInt(masEspData,"soakingTimeMax");
    masEspStrData.soakingTimeGear = getJsonInt(masEspData,"soakingTimeGear");

    if(masEspData["sndModeList"].isArray()){
        for(int i = 0; i < masEspData["sndModeList"].size(); i++){
            MasEspSndModeDataStr sndModeData;
            Json::Value jsonSndModeData = masEspData["sndModeList"][i];
            sndModeData.sndModename = getJsonString(jsonSndModeData, "sndModename");
            sndModeData.sndModeType = getJsonInt(jsonSndModeData, "sndModeType");
            sndModeData.powderDef = getJsonInt(jsonSndModeData, "powderDef");
            sndModeData.extractTempDef = getJsonInt(jsonSndModeData, "extractTempDef");
            sndModeData.soakingTimeDef = getJsonInt(jsonSndModeData, "soakingTimeDef");

            Json::Value jsonSndModeStepList(Json::arrayValue);
            if(jsonSndModeData["sndModeStepList"].isArray()){
                for(int j=0; j< jsonSndModeData["sndModeStepList"].size(); j++){
                    Json::Value jsonSndModeStepData = jsonSndModeData["sndModeStepList"][j];
                    sndModeData.stepDataList.push_back({getJsonInt(jsonSndModeStepData,"pressure"),getJsonInt(jsonSndModeStepData,"water")});
                }
            }
            masEspStrData.sndModeList.push_back(sndModeData);
        }
    }

    // 手冲咖啡
    Json::Value handWashData = fromJsonData["hand_wash"];

    handWashStrData.name = getJsonString(handWashData,"name");
    handWashStrData.powderMin = getJsonInt(handWashData,"powderMin");
    handWashStrData.powderMax = getJsonInt(handWashData,"powderMax");
    handWashStrData.powderGear = getJsonInt(handWashData,"powderGear");
    handWashStrData.extractWaterMin = getJsonInt(handWashData,"extractWaterMin");
    handWashStrData.extractWaterMax = getJsonInt(handWashData,"extractWaterMax");
    handWashStrData.extractWaterGear = getJsonInt(handWashData,"extractWaterGear");
    handWashStrData.extractTempMin = getJsonInt(handWashData,"extractTempMin");
    handWashStrData.extractTempMax = getJsonInt(handWashData,"extractTempMax");
    handWashStrData.extractTempGear = getJsonInt(handWashData,"extractTempGear");
    handWashStrData.interTimeMin = getJsonInt(handWashData,"interTimeMin");
    handWashStrData.interTimeMax = getJsonInt(handWashData,"interTimeMax");
    handWashStrData.interTimeGear = getJsonInt(handWashData,"interTimeGear");

    if(handWashData["sndModeList"].isArray()){
        
        for(int i = 0; i < handWashData["sndModeList"].size(); i++){
            HandWashSndModeDataStr sndModeData;
            Json::Value jsonSndModeData = handWashData["sndModeList"][i];
            sndModeData.sndModename = getJsonString(jsonSndModeData, "sndModename");
            sndModeData.sndModeType = getJsonInt(jsonSndModeData, "sndModeType");
            sndModeData.powderDef = getJsonInt(jsonSndModeData, "powderDef");
            sndModeData.extractTempDef = getJsonInt(jsonSndModeData, "extractTempDef");

            Json::Value jsonSndModeStepList(Json::arrayValue);
            if(jsonSndModeData["sndModeStepList"].isArray()){
                for(int j=0; j< jsonSndModeData["sndModeStepList"].size(); j++){
                    Json::Value jsonSndModeStepData = jsonSndModeData["sndModeStepList"][j];
                    sndModeData.stepDataList.push_back({getJsonInt(jsonSndModeStepData,"water"),
                                                        getJsonInt(jsonSndModeStepData,"flowRate"),
                                                        getJsonInt(jsonSndModeStepData,"interTime")});
                }
            }
            handWashStrData.sndModeList.push_back(sndModeData);
        }
    }
    
    // 萃茶
    Json::Value extractingTeaData = fromJsonData["extracting_tea"];
    extTeaStrData.name = getJsonString(extractingTeaData,"name");

    extTeaStrData.extractTempMin = getJsonInt(extractingTeaData, "extractTempMin");
    extTeaStrData.extractTempMax = getJsonInt(extractingTeaData, "extractTempMax");
    extTeaStrData.extractTempGear = getJsonInt(extractingTeaData, "extractTempGear");
    extTeaStrData.waterMin = getJsonInt(extractingTeaData, "waterMin");
    extTeaStrData.waterMax = getJsonInt(extractingTeaData, "waterMax");
    extTeaStrData.waterGear = getJsonInt(extractingTeaData, "waterGear");
    extTeaStrData.interTimeMin = getJsonInt(extractingTeaData, "interTimeMin");
    extTeaStrData.interTimeMax = getJsonInt(extractingTeaData, "interTimeMax");
    extTeaStrData.interTimeGear = getJsonInt(extractingTeaData, "interTimeGear");
    if(extractingTeaData["sndModeList"].isArray()){
        for(int i = 0; i < extractingTeaData["sndModeList"].size(); i++){
            ExtractTeaSndModeDataStr sndModeData;
            Json::Value jsonSndModeData = extractingTeaData["sndModeList"][i];
            sndModeData.sndModename = getJsonString(jsonSndModeData, "sndModename");
            sndModeData.sndModeType = getJsonInt(jsonSndModeData, "sndModeType");
            sndModeData.pic = getJsonString(jsonSndModeData, "pic");
            sndModeData.pressure = getJsonInt(jsonSndModeData, "pressure");
            sndModeData.powderDef = getJsonInt(jsonSndModeData, "powderDef");
            sndModeData.washTeaNumMin = getJsonInt(jsonSndModeData, "washTeaNumMin");
            sndModeData.washTeaNumMax = getJsonInt(jsonSndModeData, "washTeaNumMax");
            sndModeData.extractTempDef = getJsonInt(jsonSndModeData, "extractTempDef");
            sndModeData.washTeaWater = getJsonInt(jsonSndModeData, "washTeaWater");
            sndModeData.washTeaFlowRate = getJsonInt(jsonSndModeData, "washTeaFlowRate");
            sndModeData.washTeaInterTime = getJsonInt(jsonSndModeData, "washTeaInterTime");
            sndModeData.soakTeaWater = getJsonInt(jsonSndModeData, "soakTeaWater");
            sndModeData.soakTeaFlowRate = getJsonInt(jsonSndModeData, "soakTeaFlowRate");
            sndModeData.soakTeaInterTime = getJsonInt(jsonSndModeData, "soakTeaInterTime");
            sndModeData.makeTeaWater = getJsonInt(jsonSndModeData, "makeTeaWater");
            sndModeData.makeTeaFlowRate = getJsonInt(jsonSndModeData, "makeTeaFlowRate");
            sndModeData.makeTeaInterTime = getJsonInt(jsonSndModeData, "makeTeaInterTime");
            extTeaStrData.sndModeList.push_back(sndModeData);
        }
    }
}