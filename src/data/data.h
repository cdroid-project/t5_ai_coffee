#ifndef __DATA_H__
#define __DATA_H__

#include <iostream>
#include <stdio.h>
#include <vector>

#include "project.h"
#include "json/json.h"
#include "json_func.h"

#define MAX_SCREEN_LIGHT 100 // 屏幕最大亮度值
#define DEF_SCREEN_LIGHT 50 // 屏幕默认亮度

enum{
    RIGHT_INFO_EXT_TEMP = 0,    // 萃取温度
    RIGHT_INFO_EXT_WATER,       // 萃取水量
    RIGHT_INFO_SOAK,            // 预浸泡时间
    RIGHT_INFO_HOT_WATER,       // 热水水量
    RIGHT_INFO_HOT_WATER_TEMP   // 热水温度
};
typedef struct{
    int type;
    std::string name;
    std::string units;
}RightInfoStr;

// 流速
typedef enum{
    HW_FLOW_RATE_TYPE_SLOW,     // 慢
    HW_FLOW_RATE_TYPE_MEDIUM,   // 中
    HW_FLOW_RATE_TYPE_FAST,     // 快
}FlowRateTypeEm;

///////////////////////////////////////////// 测试咖啡 ////////////////////////////////////////////////
// 测试咖啡 二级 数据结构体
typedef struct tagEspData{
    int powderDef;      // 粉重 默认值
    int extractTempDef; // 萃取温度 默认值
    int extractWaterDef;// 萃取水量 默认值
    int soakingTimeDef;  // 预浸泡时间 默认值

    bool operator == (const tagEspData &other){
        return (powderDef == other.powderDef) && (extractTempDef == other.extractTempDef) 
            && (extractWaterDef == other.extractWaterDef) && (soakingTimeDef == other.soakingTimeDef);
    }
}EspSndDataStr;

// 测试咖啡 萃取模式 数据结构体
typedef struct{
    std::string name;   // 萃取模式名称
    std::string pic;    // 图片
    int powderMin;      // 粉重 最小值
    int powderMax;      // 粉重 最大值
    int powderGear;     // 粉重 步进值

    int extractTempMin; // 萃取温度 最小值
    int extractTempMax; // 萃取温度 最大值
    int extractTempGear;// 萃取温度 步进值

    int extractWaterMin;// 萃取水量 最小值
    int extractWaterMax;// 萃取水量 最大值
    int extractWaterGear;//萃取水量 步进值

    int soakingTimeMin;    // 预浸泡时间 最小值
    int soakingTimeMax;    // 预浸泡时间 最大值
    int soakingTimeGear;   // 预浸泡时间 步进值

    EspSndDataStr  sndData;
}EspDataStr;

///////////////////////////////////////////// 测试1咖啡 ////////////////////////////////////////////////
// 测试1咖啡 二级 数据结构体
typedef struct tagAmericanData{
    int powderDef;      // 粉重 默认值
    int extractTempDef; // 萃取温度 默认值
    int extractWaterDef;// 萃取水量 默认值
    int hotWaterTempDef; // 热水温度 默认值
    int hotWaterDef;    // 热水水量 默认值
    int soakingTimeDef;  // 预浸泡时间 默认值

    bool operator == (const tagAmericanData &other){
        return (powderDef == other.powderDef) && (extractTempDef == other.extractTempDef) && (extractWaterDef == other.extractWaterDef)
            && (hotWaterTempDef == other.hotWaterTempDef) && (hotWaterDef == other.hotWaterDef) && (soakingTimeDef == other.soakingTimeDef);
    }
}AmericanSndDataStr;

// 测试1咖啡 数据结构体
typedef struct{
    std::string name;   // 萃取模式名称
    std::string pic;    // 图片

    int powderMin;      // 粉重 最小值
    int powderMax;      // 粉重 最大值
    int powderGear;     // 粉重 步进值

    int extractTempMin; // 萃取温度 最小值
    int extractTempMax; // 萃取温度 最大值
    int extractTempGear;// 萃取温度 步进值

    int extractWaterMin;// 萃取水量 最小值
    int extractWaterMax;// 萃取水量 最大值
    int extractWaterGear;//萃取水量 步进值

    int hotWaterTempMin; // 热水温度 最小值
    int hotWaterTempMax; // 热水温度 最大值
    int hotWaterTempGear;// 热水温度 步进值

    int hotWaterMin;    // 热水水量 最小值
    int hotWaterMax;    // 热水水量 最大值
    int hotWaterGear;   // 热水水量 步进值

    int soakingTimeMin;  // 预浸泡时间 最小值
    int soakingTimeMax;  // 预浸泡时间 最大值
    int soakingTimeGear; // 预浸泡时间 步进值

    AmericanSndDataStr sndData;
}AmericanDataStr;

///////////////////////////////////////////// 测试浓缩 ////////////////////////////////////////////////
// 测试浓缩模式 萃取模式 步骤 数据结构体
typedef struct{
    int pressure;   // 压力
    int water;      // 出水量
}MasEspStepDataStr;

// 测试浓缩模式 二级模式 数据结构体
typedef struct tagMasEspData{
    std::string sndModename;// 萃取模式名称
    int sndModeType;        // 萃取模式Type(目前仅用作上报APP)
    
    int powderDef;      // 粉重 默认值
    int extractTempDef; // 萃取温度 默认值
    int soakingTimeDef;  // 预浸泡时间 默认值

    std::vector<MasEspStepDataStr> stepDataList;

    bool operator == (const tagMasEspData &other){
        if((sndModeType == other.sndModeType)&&(powderDef == other.powderDef) && (extractTempDef == other.extractTempDef)
            && (soakingTimeDef == other.soakingTimeDef) && (stepDataList.size() == other.stepDataList.size())){
            for(int i=0; i<stepDataList.size(); i++){
                if((stepDataList.at(i).water != other.stepDataList.at(i).water) || (stepDataList.at(i).pressure != other.stepDataList.at(i).pressure))
                    return false;
            }
            return true;
        }else
            return false;
    }
}MasEspSndModeDataStr;

// 测试浓缩模式 萃取模式 数据结构体
typedef struct{
    std::string name;   // 萃取模式名称

    int powderMin;      // 粉重 最小值
    int powderMax;      // 粉重 最大值
    int powderGear;     // 粉重 步进值

    int extractTempMin; // 萃取温度 最小值
    int extractTempMax; // 萃取温度 最大值
    int extractTempGear;// 萃取温度 步进值

    int extractWaterMin;// 萃取水量 最小值
    int extractWaterMax;// 萃取水量 最大值
    int extractWaterGear;//萃取水量 步进值

    int soakingTimeMin;  // 预浸泡时间 最小值
    int soakingTimeMax;  // 预浸泡时间 最大值
    int soakingTimeGear; // 预浸泡时间 步进值

    std::vector<MasEspSndModeDataStr> sndModeList;  // 该模式的 步骤数据数据
}MasEspDataStr;

///////////////////////////////////////////// 测试2咖啡 ////////////////////////////////////////////////

// 表格步骤 数据结构体
typedef struct{
    int water;      // 注水量
    int flowRate;   // 流速（快、中、慢）
    int interTime;  // 中断时间
}FormStepDataStr;

// 测试2咖啡 二级模式 数据结构体
typedef struct tagHandWashData{
    std::string sndModename;// 萃取模式名称
    int sndModeType;        // 萃取模式Type(目前仅用作上报APP)
    int powderDef;          // 粉重 默认值
    int extractTempDef;     // 萃取温度 默认值

    std::vector<FormStepDataStr> stepDataList;

    bool operator == (const tagHandWashData &other){
        if((sndModeType == other.sndModeType) && (powderDef == other.powderDef) && (extractTempDef == other.extractTempDef) && (stepDataList.size() == other.stepDataList.size())){
            for(int i=0; i<stepDataList.size(); i++){
                if((stepDataList.at(i).water != other.stepDataList.at(i).water) 
                || (stepDataList.at(i).flowRate != other.stepDataList.at(i).flowRate)
                || (stepDataList.at(i).interTime != other.stepDataList.at(i).interTime))
                    return false;
            }
            return true;
        }else
            return false;
    }
}HandWashSndModeDataStr;

// 测试2咖啡模式 萃取模式 数据结构体
typedef struct{
    std::string name;   // 萃取模式名称

    int powderMin;      // 粉重 最小值
    int powderMax;      // 粉重 最大值
    int powderGear;     // 粉重 步进值

    int extractWaterMin;// 萃取水量 最小值
    int extractWaterMax;// 萃取水量 最大值
    int extractWaterGear;//萃取水量 步进值

    int extractTempMin; // 萃取温度 最小值
    int extractTempMax; // 萃取温度 最大值
    int extractTempGear;// 萃取温度 步进值

    int interTimeMin;   // 中断时间 最小值
    int interTimeMax;   // 中断时间 最大值
    int interTimeGear;  // 中断时间 步进值

    std::vector<HandWashSndModeDataStr> sndModeList;  // 该模式的数据
}HandWashDataStr;

///////////////////////////////////////////// 测茶 ////////////////////////////////////////////////
// 测茶 二级菜单 数据结构体
typedef struct tagTeaData{
    std::string sndModename;   // 萃取模式名称
    int sndModeType;           // 萃取模式Type(目前仅用作上报APP)
    std::string pic;    // 萃取模式图片
    
    int pressure;       // 压力(固定)
    int powderDef;      // 粉重 建议值（最大值固定是5）

    int washTeaNumMin;  // 建议冲泡次数最小值
    int washTeaNumMax;  // 建议冲泡次数最大值

    int extractTempDef; // 萃取温度 默认值
    
    int washTeaWater;    // 洗茶 注水量
    int washTeaFlowRate; // 洗茶 流速 （流速默认只有0、1、2，慢、中、快）
    int washTeaInterTime;// 洗茶 中断时间

    int soakTeaWater;    // 泡茶 注水量
    int soakTeaFlowRate; // 泡茶 流速 （流速默认只有0、1、2，慢、中、快）
    int soakTeaInterTime;// 泡茶 中断时间

    int makeTeaWater;    // 冲茶 注水量
    int makeTeaFlowRate; // 冲茶 流速 （流速默认只有0、1、2，慢、中、快）
    int makeTeaInterTime;// 冲茶 中断时间

    bool operator == (const tagTeaData &other){
        return (sndModeType == other.sndModeType)   && (pressure == other.pressure) && (powderDef == other.powderDef) && (extractTempDef == other.extractTempDef)
            && (washTeaWater == other.washTeaWater) && (washTeaFlowRate == other.washTeaFlowRate) && (washTeaInterTime == other.washTeaInterTime)
            && (soakTeaWater == other.soakTeaWater) && (soakTeaFlowRate == other.soakTeaFlowRate) && (soakTeaInterTime == other.soakTeaInterTime)
            && (makeTeaWater == other.makeTeaWater) && (makeTeaFlowRate == other.makeTeaFlowRate) && (makeTeaInterTime == other.makeTeaInterTime);
    }
}ExtractTeaSndModeDataStr;

// 测茶 数据结构体
typedef struct{
    std::string name; // 萃取模式名称

    int waterMin;       // 注水量 最小值
    int waterMax;       // 注水量 最大值
    int waterGear;      // 注水量 步进值

    int interTimeMin;   // 中断时间 最小值
    int interTimeMax;   // 中断时间 最大值
    int interTimeGear;  // 中断时间 步进值

    int extractTempMin; // 萃取温度 最小值
    int extractTempMax; // 萃取温度 最大值
    int extractTempGear;// 萃取温度 步进值
    std::vector<ExtractTeaSndModeDataStr> sndModeList;
}ExtractTeaDataStr;


///////////////////////////////////////////// 主页Tab数据结构体 ////////////////////////////////////////////////
typedef struct{
    int modeType;
    std::string modeName;
}HomePageTabDataStr;

typedef struct{
    int  ProvinceCity;
    std::string cityName;

}cityData;

enum{
    MC_STATE_NONE       = 0x00, // 空闲状态
    MC_STATE_POWDER     = 0x01, // 磨豆中
    MC_STATE_EXT        = 0x02, // 萃取中
    MC_STATE_STEAM      = 0x04, // 出蒸汽中
    MC_STATE_HOT_WATER  = 0x08, // 出热水中
    MC_STATE_CLEAN_WATER= 0x10, // 水路清洁
    MC_STATE_CLEAN_CAL  = 0x20, // 钙化清洁
};

enum{
    ITEM_ACTIVIT_NONE,
    ITEM_ACTIVIT_FIR = 0x01,   // 活动项为第一个
    ITEM_ACTIVIT_SND = 0x02,   // 活动项为第二个
};

enum{
    HOME_PAGE_NORMAL    = 0x01, // 正常页面
    HOME_PAGE_FAV_EDIT  = 0x02, // 收藏进入的编辑模式
    HOME_PAGE_FAV_EXT   = 0x03, // 收藏进入的萃取模式
};

extern std::vector<cityData> cityProvinceList;            // 城市省份对应关系

// 设置屏幕亮度
void setScreenLight(int light, bool chk = true);

// 读取模式数据
void loadModeDataFromJson(Json::Value fromJsonData,EspDataStr &espStrData,AmericanDataStr &americanStrData,
                            MasEspDataStr &masEspStrData,HandWashDataStr &handWashStrData,ExtractTeaDataStr &extTeaStrData);
#endif