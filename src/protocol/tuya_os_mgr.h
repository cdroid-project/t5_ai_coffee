
#ifndef TUYA_OS_DISABLE
#ifndef __tuya_os_mgr_h__
#define __tuya_os_mgr_h__

#include <common.h>
#include <tuya_gw.h>
#include "comm_class.h"
#include "data.h"

typedef struct weatherInfo{
    std::string weatherImg;
    std::string weatherName;
}WeatherInfo;

static const std::map<int ,WeatherInfo> WeatherImgMap = {
    {101,{"@mipmap/tuyaWeather/dayu","大雨"}},
    {102,{"@mipmap/tuyaWeather/lei","雷暴"}},
    {103,{"@mipmap/tuyaWeather/yangsha","沙尘暴"}},
    {104,{"@mipmap/tuyaWeather/xiaoxue","小雪"}},
    {105,{"@mipmap/tuyaWeather/zhongxue","雪"}},
    {106,{"@mipmap/tuyaWeather/wu","冻雾"}},
    {107,{"@mipmap/tuyaWeather/dayu","暴雨"}},
    {108,{"@mipmap/tuyaWeather/zhongyu","局部阵雨"}},
    {109,{"@mipmap/tuyaWeather/yangsha","浮尘"}},
    {110,{"@mipmap/tuyaWeather/lei","雷电"}},
    {111,{"@mipmap/tuyaWeather/xiaoyu","小阵雨"}},
    {112,{"@mipmap/tuyaWeather/zhongyu","雨"}},
    {113,{"@mipmap/tuyaWeather/xuejiayu","雨夹雪"}},
    {114,{"@mipmap/tuyaWeather/fengbao","尘卷风"}},
    {115,{"@mipmap/tuyaWeather/xiaoxue","冰粒"}},
    {116,{"@mipmap/tuyaWeather/yangsha","强沙尘暴"}},
    {117,{"@mipmap/tuyaWeather/yangsha","扬沙"}},
    {118,{"@mipmap/tuyaWeather/zhongyu","小到中雨"}},
    {119,{"@mipmap/tuyaWeather/qingtian","大部晴朗"}},
    {120,{"@mipmap/tuyaWeather/qingtian","晴"}},
    {121,{"@mipmap/tuyaWeather/wu","雾"}},
    {122,{"@mipmap/tuyaWeather/zhenyu","阵雨"}},
    {123,{"@mipmap/tuyaWeather/zhenyu","强阵雨"}},
    {124,{"@mipmap/tuyaWeather/daxue","大雪"}},
    {125,{"@mipmap/tuyaWeather/dayu","特大暴雨"}},
    {126,{"@mipmap/tuyaWeather/baoxue","暴雪"}},
    {127,{"@mipmap/tuyaWeather/daxue","冰雹"}},
    {128,{"@mipmap/tuyaWeather/zhongxue","小到中雪"}},
    {129,{"@mipmap/tuyaWeather/shaoyun","少云"}},
    {130,{"@mipmap/tuyaWeather/xiaoxue","小阵雪"}},
    {131,{"@mipmap/tuyaWeather/zhongxue","中雪"}},
    {132,{"@mipmap/tuyaWeather/yin","阴"}},
    {133,{"@mipmap/tuyaWeather/xiaoxue","冰针"}},
    {134,{"@mipmap/tuyaWeather/dayu","大暴雨"}},
    {135,{"@mipmap/tuyaWeather/wind",""}},
    {136,{"@mipmap/tuyaWeather/zhenyu","雷阵雨伴有冰雹"}},
    {137,{"@mipmap/tuyaWeather/xuejiayu","冻雨"}},
    {138,{"@mipmap/tuyaWeather/zhongxue","阵雪"}},
    {139,{"@mipmap/tuyaWeather/xiaoyu","小雨"}},
    {140,{"@mipmap/tuyaWeather/mai","霾"}},
    {141,{"@mipmap/tuyaWeather/zhongyu","中雨"}},
    {142,{"@mipmap/tuyaWeather/duoyun","多云"}},
    {143,{"@mipmap/tuyaWeather/leizhenyu","雷阵雨"}},
    {144,{"@mipmap/tuyaWeather/dayu","中到大雨"}},
    {145,{"@mipmap/tuyaWeather/dayu","大到暴雨"}},
    {146,{"@mipmap/tuyaWeather/qingtian","晴朗"}},
};

class TuyaOsMgr: public EventHandler {
protected:
    TuyaOsMgr();
    ~TuyaOsMgr();

    void reportAllStatus();                 // 所有状态上报函数
public:
    static TuyaOsMgr *ins() {
        static TuyaOsMgr stIns;
        return &stIns;
    }

    int init();

    // 设置多线程数据任务状态
    void setRequsetStatus(bool status);
    // 解绑涂鸦
    void unBindTuya(bool isFactory);
    // 上报正常的DP数据
    void reportDpData(unsigned char dpid, unsigned char type,void *data);
    // 上报咖啡的DIY数据
    void reportCoffeeDiyRawData(uint8_t type,     uint8_t mode,       uint8_t temp, uint8_t water, 
                                uint8_t soakTime, uint8_t hotWater,   uint8_t hotWaterTemp,
                                std::vector<MasEspStepDataStr> MasEspStepDataList = {},
                                std::vector<FormStepDataStr>   HandWashStepDataList = {});
    // 上报萃茶的DIY数据
    void reportTeaDiyRawData(uint8_t type, uint8_t mode, uint8_t temp, ExtractTeaSndModeDataStr teaStepData);
protected:
    virtual int checkEvents();
    virtual int handleEvents();
    bool onDpCommDeal(unsigned char dpid,const void *data);
    void onNetStatus(unsigned char status);
    void onGetQrcode(const char *qrcode);
    void dealTuyaCallback();
    // void clearOutWaterData();
    // bool checkDpChange(int dpid, int value);
private:
    int64_t          mNextEventTime;
    int64_t          mNextGetWeatherTime;   // 下一次获取涂鸦天气的时间
    int64_t          mNextUpdateTime;       // 下一次获取云端时间的时间
    int64_t          mNextReportAllTime;    // 下一次上报所有状态的时间
    int64_t          mConnCompTime;         // 连接涂鸦云完成时间
    int64_t          mDisConnCloudTime;     // 与云端断开连接的时间
    int64_t          mNextReportStandbyTime;// 修改状态为Standby

    bool             mIsConnCloud;      // 是否连接上云端
    bool             mIsRequesting;     // 是否正在请求其他数据
    bool             mIsNetDisConn;     // 是否断网，但已经连接上了云端，5分钟后才会重置，全权交由涂鸦状态处理

    bool             mIsInit;
    bool             mIsRecvDiyData;

    std::vector<TY_OBJ_DP_S> mTuyaDpList;
    std::vector<TY_OBJ_DP_S> mTuyaDpCacheList;

    TY_RAW_DP_S             *mRawDpCacheData;
    std::vector<int> mCallbackList;
    // WATER            mOutWaterData;
};

extern TuyaOsMgr *g_tuyaOsMgr;


class RequestWeatherThread :public ThreadTask{
public:
private:
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};

class RequestTimeThread :public ThreadTask{
public:
private:
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};

class ReportRawDpThread :public ThreadTask{
public:
private:
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};

#endif
#endif