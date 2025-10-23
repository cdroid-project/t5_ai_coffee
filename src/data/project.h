/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:48:57
 * @FilePath: /t5_ai_coffee/src/data/project.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <comm_class.h>
#include <comm_func.h>
#include <common.h>
#include <proto.h>
#include "json_func.h"
#include "R.h"

#define GB_STAT_LAN_UNCONN 0 // offline in LAN
#define GB_STAT_LAN_CONN 1   // online in LAN, offline in WAN
#define GB_STAT_CLOUD_CONN 2 // online in WAN


typedef enum{
    HOME_MT_ESPRESSO = 0,       // 意式咖啡
    HOME_MT_AMERICANO,          // 美式咖啡
    HOME_MT_MASTER_ESPRESSO,    // 大师浓缩
    HOME_MT_HAND_WASHED,        // 手冲咖啡
    HOME_MT_EXTRACTING_TEA,     // 萃茶
    HOME_MT_FAVORITES,          // 我的收藏
}Home_Mode_Type;

enum {
    BEAN_GRIND_WEI = 0, // 称豆模式
    BEAN_GRIND_TIME= 1, // 计时模式
};

enum {
    STEAM_AUTO      = 0, // 自动模式
    STEAM_MANUAL    = 1, // 手动模式
};

// 程序状态
typedef enum ProgramState {
    PRO_STATE_NONE = 0,                     // 某个功能无状态

    PRO_STATE_TONGSUO,                      // 童锁开启
    PRO_STATE_TONGSUO_CLOSE,                // 童锁关闭
    PRO_STATE_TONGSUO_UBABLE,               // 童锁不可用   

    PRO_STATE_ZHAOMING_CLOSE,               // 照明关闭
    PRO_STATE_ZHAOMING,                     // 照明开启

    PRO_STATE_NET_NONE,                     // 网络 没信号
    PRO_STATE_NET_ONE,                      // 网络 一格信号
    PRO_STATE_NET_TWO,                      // 网络 两格信号
    PRO_STATE_NET_THREE,                    // 网络 三格信号
    PRO_STATE_NET_FULL,                     // 网络 满信号

    // PRO_STATE_TUYA_GW_RESET,                 // 网关重置
    // PRO_STATE_TUYA_GW_ACTIVED,               // 网关激活
    // PRO_STATE_TUYA_GW_FIRST_START,           // 网关第一次启动
    // PRO_STATE_TUYA_GW_NORMAL,                // 网关激活并启动
    // PRO_STATE_TUYA_GW_BLE_ACTIVED,           // 连接上云端

    PRO_STATE_TUYA_UNCONN,                   // 涂鸦离线
    PRO_STATE_TUYA_CONNING,                  // 涂鸦连接中
    PRO_STATE_TUYA_CONN,                     // 涂鸦已连接
    PRO_STATE_TUYA_LOW_POWER,                // 涂鸦低功耗

    PRO_STATE_TUYA_GB_LAN_UNCONN,            // 网关离线
    PRO_STATE_TUYA_GB_LAN_CONN,              // 网关在线
    PRO_STATE_TUYA_GB_CLOUD_CONN,            // 网关连接到云端

    PRO_AUTH_CURL_ERROR,                    // 获取涂鸦授权码 curl 报错
    PRO_AUTH_ERROR,                         // 获取涂鸦授权码报错
    PRO_AUTH_SUCCESS,                       // 获取涂鸦授权码成功
}ProState;

// 应用全局数据
typedef struct tagAppGlobalData {

    bool  isBeta;                   // 是否是测试版本
    int   tuyaWifiStatus;           // 涂鸦wifi状态
    bool  isNeedUpdate;             // 是否需要更新软件
    std::string otaVersion;         // 软件更新版本号
    std::string otaVersionDes;      // 软件更新的说明
    std::string otaMd5;             // 软件更新的md5
    std::string updateUrl;          // 软件更新的url
    std::string tuyaQrcode;         // 涂鸦的二维码连接

    std::string mcuUpgradeVer;      // MCU更新版本号
    std::string mcuUpgradeUrl;      // MCU更新Url
    std::string mcuUpgradeMd5;      // MCU更新Md5
    std::string mcuUpgradeText;     // MCU更新说明

    std::string McuVersion;         // MCU版本号
    
    bool  netSwitch;                // 网络开关
    bool  netOk;                    // 是否连接wifi
    int   netStatus;                // 网络状态

    bool  isMute;                   // 静音
    int   volume;                   // 音量
    int   light;                    // 亮度
    EquipmentStatus eqStatus;       // 设备状态
    int   warnState;                // 设备警告
    int   warnShowType;             // 正在显示的警告
    int   warnIsDealState;          // 已经处理了的警告
    int   weatherTemp;              // 天气温度
    std::string   weatherConImage;  // 天气 图片

    int   homeTabSelectMode;        // 主页选择模式

    int   resetCount;
    bool  AutoChildLocked;          // 自动童锁
    int   childLocked;              // 童锁
    bool  isShutDown;                // 休眠

    int   beanGrindMode;            // 磨豆 模式
    int   steamMode;                // 蒸汽 模式
    int   steamTemp;                // 蒸汽 温度
    int   hotWaterTemp;             // 热水 温度
    int   hotWater;                 // 热水 出水量

    int   extractWater;             // 实时萃取水量
    int   extractPressure;          // 实时萃取压力
    int   grindBean;                // 实时磨豆（克重、时间）
    int   machineState;             // 机器运行状态（磨豆中、萃取中、闲置）
    int   coffeePreheatPtc;         // 咖啡预热进度
    int   getAuthStatus;            // 获取授权
    int   statusChangeFlag;         // 状态变换标记
    int   scaleCalFlag;             // 电子秤标定状态
    int   scaleCalWeight;           // 电子秤重量

    int   tuyaSetPowder;            // 涂鸦 粉量设置
    bool  tuyaStartGrind;           // 涂鸦 磨豆启动/停止

    bool  tuyaStartExt;             // 涂鸦 萃取、测茶、热水的开关
    int   tuyaExtCoffeeMode;        // 涂鸦 萃取咖啡模式
    int   tuyaMakeType;             // 涂鸦 制作大类
    uchar tuyaDiyData[20];          // 涂鸦 DIY数据
    bool  tuyaStartBrewAgain;       // 涂鸦 再沏（再次沏茶）
    bool  tuyaStartCleaning;        // 涂鸦 清洁启动、暂停
    int   tuyaCleanMode;            // 涂鸦 清洁模式
    bool  displayMode;
    int   mcuOTAProgress;           // MCU更新的进度
    bool  mIsFactory;               // 是否在产测中

    std::tm nextPopWarnClean;       // 下次弹窗 钙化清洁 警告时间
                
    std::string   getAuthError;     // 获取授权码的error code
    std::string   mcuOTAError;      // mcu更新的错误信息
    
    std::string   aiText;           // ai回复的文本
    Json::Value   aiJsonText;       // ai的json

    tagAppGlobalData()
        : isBeta(false),tuyaWifiStatus(PRO_STATE_TUYA_GB_LAN_UNCONN),isNeedUpdate(false),tuyaQrcode("none"),netSwitch(true),netOk(false),childLocked(PRO_STATE_TONGSUO_CLOSE),isShutDown(false),resetCount(0),AutoChildLocked(true),
            weatherTemp(20),weatherConImage("@mipmap/tuyaWeather/yin"),homeTabSelectMode(HOME_MT_ESPRESSO),
            McuVersion("2024_02_06_V101"),coffeePreheatPtc(0),scaleCalFlag(0),
            tuyaSetPowder(-1),tuyaCleanMode(-1),tuyaStartGrind(false),machineState(0),warnState(WARN_NONE),warnIsDealState(WARN_NONE),mcuOTAProgress(0),mIsFactory(false),
            light(70),displayMode(false){}
} AppGlobalData;

extern AppGlobalData g_appData;

bool isTimeRange(std::tm star_time, std::tm end_time);

#endif