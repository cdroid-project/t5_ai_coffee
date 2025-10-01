
#ifndef __conn_mgr_h__
#define __conn_mgr_h__

#include "packet_buffer.h"
#include "uart_client.h"
#include "cmd_handler.h"
#include "data.h"

typedef std::function<void()> factoryCb;
#define g_objConnMgr CConnMgr::ins()
class CConnMgr : public EventHandler, public IHandler {
protected:
    CConnMgr();
    ~CConnMgr();

public:
    static CConnMgr *ins() {
        static CConnMgr stIns;
        return &stIns;
    }

    // // 产测
    void setFactoryCallback(UartClient::uartCallback cb);

    // 关机指令
    void setShutDown(bool isShutDown = true);
    // 预热指令
    void setPreheating(uint8_t coffeeTemp, uint8_t steamTemp);
    // 磨豆指令
    void setGrindBean(bool flag, uint8_t powder, uint8_t mode);
    // 萃取指令
    void setExtraction(bool flag, uint8_t type, uint8_t soakTime, uint8_t soakTemp,  uint8_t extWater, uint8_t extTemp,uint8_t amrHotWater ,uint8_t amrHotWaterTemp);
    // 电子秤校准
    void setElectronicScaleCal(bool flag);
    // 电子秤
    void setElectronicScale(uint8_t type,uint8_t weight);
    // 蜂鸣器、SetBit       0x25(37)
    void setBuzzer_SetBit(uint8_t num,uint8_t beepTime,uint8_t beepStopTime);

    // BootLoader 升级信息下发
    void setBootLoaderInfo();
    // 固件下发指令
    void sendBootLoaderData();
    // 固件下载校验结果查询指令
    void sendBootLoaderCheck();

    // 设备自检
    void setDeviceSelfTest();
    // 自清洁（除垢）
    void setCleanSelf(bool flag, uint8_t type);
    // 大师浓缩
    void setMasEsp(bool flag,uint8_t extTemp,uint8_t soakTime,std::vector<MasEspStepDataStr> stepDataList);
    // 手冲咖啡
    void setHandWash(bool flag,uint8_t extTemp,std::vector<FormStepDataStr> stepDataList);
    // 萃茶
    void setExtTea(bool flag,uint8_t extTemp, bool isRebrew,ExtractTeaSndModeDataStr stepData);

    // 异常上报指令的回应帧
    void reportErrorDataFrame();
    // 机器设置指令
    void setDeviceData();

    // 设置机器重置状态（停掉所有的负载，如在ota中、恢复出厂设置中）
    void setResetStatus(bool flag);

    int init();
    // 发送给按键板
    void send2MCU();
    
    // 版本号
    bool getVersion(int &a, int &b, int &c);

    // 检查MCU的OTA固件
    void checkMCUOTA();

protected:
    virtual int checkEvents();
    virtual int handleEvents();
    virtual void onCommDeal(IAck *ack);
private:
    bool             mIsFactory;
    int64_t          mLastStatusTime;
    IPacketBuffer   *mPacket;
    int64_t          mNextEventTime;
    int64_t          mLastSendTime;
    int64_t          mLastWarnTime;
    int64_t          mLastBeepTime;
    UartClient      *mUartMCU; // mcu
    int              mSetData; // 设置数据次数
    int              commErrorCount;
    uchar            mSndData[UI2MCU::BUF_LEN]; // 发送缓冲区    

    uchar            mMcuType;           // 机型

    uchar           *mSendOTAData;
    int              mAllOTALen;
    int              mSendOTALen;
    int              mSendOTACount;
    uchar            mOTADataSum;
};



#endif
