/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:49:18
 * @FilePath: /t5_ai_demo/src/data/conf_mgr.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


#ifndef __conf_mgr_h__
#define __conf_mgr_h__

#include <unistd.h>
#include <core/preferences.h>

#include "project.h"
#include "data.h"

#define g_objConf CConfMgr::ins()

class CConfMgr
{
public:
    static CConfMgr *ins()
    {
        static CConfMgr stIns;
        return &stIns;
    }

    int  init();
    void update();

    // 首次初始化
    bool isInitSet();
    void setInitSetFlag(bool flag);

    // 童锁
    bool getAutoChildLockFlag();
    void setAutoChildLockFlag(bool flag);

    // 静音
    bool getMuteFlag();
    void setMuteFlag(bool flag);

    // 音量
    int  getVolume();
    void setVolume(int value);

    // 亮度
    int  getLight();
    void setLight(int value);

    // 打奶泡 总次数
    int  getFrothTotal();
    void setFrothTotal(int value);

    // 出热水 总次数
    int  getHotWaterTotal();
    void setHotWaterTotal(int value);

    // 研磨 总次数
    int  getGrindTotal();
    void setGrindTotal(int value);

    // 意式咖啡 总次数
    int  getEspTotal();
    void setEspTotal(int value);
    
    // 美式咖啡 总杯数
    int  getAmericanoTotal();
    void setAmericanoTotal(int value);

    // 大师浓缩咖啡 总杯数
    int  getMasEspTotal();
    void setMasEspTotal(int value);

    // 手冲咖啡 总杯数
    int  getHandWashTotal();
    void setHandWashTotal(int value);

    // 萃茶 总次数
    int  getTeaTotal();
    void setTeaTotal(int value);

    // 热水温度
    int  getHotWaterTemp();
    void setHotWaterTemp(int value);

    // 热水 出水量
    int  getHotWater();
    void setHotWater(int value);

    // 蒸汽模式
    int  getSteamMode();
    void setSteamMode(int value);

    // 蒸汽温度
    int  getSteamTemp();
    void setSteamTemp(int value);

    // 磨豆模式
    int  getBeanGrindMode();
    void setBeanGrindMode(int value);


    // 获取MAC地址
    std::string getDeviceMac();
    void setDeviceMac(const std::string &mac);

    // 获取wifi信息
    std::string getWifiPasswd();
    std::string getWifiName();
    bool getWifiInfo(std::string &wifiName, std::string &wifiPasswd);
    void setWifiInfo(const std::string &wifiName, const std::string &wifiPasswd);

    // tuya二维码
    std::string getTuyaQrCode();
    void setTuyaQrCode(const std::string &qrCode);

    // 是否为Beta测试的版本
    bool getBetaVersion();
    void setBetaVersion(bool flag);

    // 获取 涂鸦的授权码
    void getFromServerAuth();
    void getTuyaAuthCode(std::string &authUuid,std::string &authKey);
    std::string getTuyaAuthUUID();
    void setTuyaAuthCode(const std::string &authUuid,const std::string &authKey);

    // 恢复出厂设置
    void reset();
    
public:
    // void changePageJsonData();
    void getHomeTabList(std::vector<HomePageTabDataStr> &tabList);
    EspDataStr getEspData(bool isDefaultData = false);
    void setEspData(EspDataStr data);

    AmericanDataStr getAmericanData(bool isDefaultData = false);
    void setAmericanData(AmericanDataStr data);

    MasEspDataStr getMasEspData(bool isDefaultData = false);
    void setMasEspData(MasEspDataStr data);

    HandWashDataStr getHandWashData(bool isDefaultData = false);
    void setHandWashData(HandWashDataStr data);

    ExtractTeaDataStr getExtractTeaData(bool isDefaultData = false);
    void setExtractTeaData(ExtractTeaDataStr data);
 
    Json::Value getFavModeList();
    void setFavModeList(Json::Value list);
    void addFavModeData(Json::Value data);
    void deleteFavModeData(int Pos);
    void deleteFavModeData(Json::Value data);
    void editFavModeData(Json::Value data,int Pos);
    bool checkFavModeData(Json::Value data);

protected:
    CConfMgr();
    void createConfig();

    void loadModeJsonData();// 加载 模式 数据
    void loadFavJsonData(); // 加载 收藏 数据

    void saveModeJsonData(bool isBak = false);// 保存 模式 数据
    void saveFavJsonData(bool isBak = false); // 保存 收藏 数据
private:
    int         mUpdates;
    bool        mIsSaveModeBak;
    bool        mIsSaveFavBak;
    
    std::string mAuthFileName;  // 文件名(涂鸦授权码)
    std::string mFileName;      // 文件名
    std::string mFavJsonFileName;// 收藏模式数据 文件名
    std::string mModeJsonFileName;// 收藏模式数据 文件名

    Preferences mAuthPrefer;    // 配置文件读写(涂鸦授权码)
    Preferences mPrefer;        // 配置文件读写

    EspDataStr         mEspData;
    AmericanDataStr    mAmericanData;
    MasEspDataStr      mMasEspData;
    HandWashDataStr    mHandWashData;
    ExtractTeaDataStr  mExtTeaData;

    EspDataStr         mEspDefaultData;
    AmericanDataStr    mAmericanDefaultData;
    MasEspDataStr      mMasEspDefaultData;
    HandWashDataStr    mHandWashDefaultData;
    ExtractTeaDataStr  mExtTeaDefaultData;

    Json::Value FavJsonValue;
    Json::Value deleteData; // removeIndex 的垃圾数据

    int64_t     mNextBakTick;
    int64_t     mNextFavBakTick;
    int64_t     mNextModeBakTick;
};

#endif
