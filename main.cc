/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:52:17
 * @FilePath: /t5_ai_demo/main.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#include <cdlog.h>
#include "R.h"
#include "wind_mgr.h"
#include "conf_mgr.h"
#include "wifi_adapter.h"
#include "curldownload.h"
#include "conn_mgr.h"
#include "t5_conn_mgr.h"
#include "ota.h"
#include <core/app.h>
#include <ghc/filesystem.hpp>
#include "btn_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "hv_net.h"
#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////
class CTickHanlder : public EventHandler , public ThreadTask{
public:
    CTickHanlder() {
        mPingFailCount = 0;
        mNetChecking = false;
        firstCheckNetTime = SystemClock::uptimeMillis()+10*1000;
    }

    //10 ms 执行一 次
    virtual int checkEvents() {
        static int64_t s_last_tick = SystemClock::uptimeMillis();        
        int64_t now_tick = SystemClock::uptimeMillis();

        if (now_tick - s_last_tick >= 1000) {
            s_last_tick = now_tick;
            
            return 1;
        }

        return 0;
    }

    virtual int handleEvents() {
        static int64_t nextScanWifiTime = SystemClock::uptimeMillis()+5*1000; 
        static int64_t nextCheckUpTime = SystemClock::uptimeMillis()+2*1000; 
        int64_t now_tick = SystemClock::uptimeMillis();

        g_objConf->update();
        if( (now_tick >= nextScanWifiTime )  && g_appData.netSwitch) {
            nextScanWifiTime = now_tick + (g_appData.netOk?2*60*1000:30*1000);
#ifdef CDROID_SIGMA
            WIFIMgr::ins()->scanWifi();
#endif
        }
        // 网络探测
        if (!mNetChecking && now_tick >= firstCheckNetTime) {
#ifdef CDROID_SIGMA
            mNetChecking = true;
            ThreadPool::ins()->add(this, 0);
#endif
        }
        // 检查更新
        if( now_tick >= nextCheckUpTime){
            nextCheckUpTime = now_tick + 30*60*1000;
            if(g_appData.eqStatus != ES_RESET) checkUpdata();
        }              
        return 0;
    }

    virtual int onTask(void *data) {
        if (!HV_NET_CheckNetStatus()){
            mPingFailCount++;
        } else {
            mPingFailCount = 0;
        }
        return 0;
    }

    virtual void onMain(void *data) {
        if (mPingFailCount >= 3 && g_appData.netOk && (WIFIMgr::ins()->connStatus() != WIFI_CONNECTING)){
            LOGE("net disconnected!!!");
            g_appData.netOk = false;
            g_appData.netStatus = PRO_STATE_NET_NONE;
            g_appData.statusChangeFlag |= CS_WIFI_CONNECT;
            g_appData.statusChangeFlag |= CS_NETWORK_CHANGE;
            
            g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WIFI);
            g_windMgr->updateDate();
        } else if (!g_appData.netOk && mPingFailCount == 0 && (WIFIMgr::ins()->connStatus() != WIFI_CONNECTING)) {
            LOGI("net connected!!!");
            g_appData.netOk = true;
            g_appData.netStatus = PRO_STATE_NET_THREE;
            g_appData.statusChangeFlag |= CS_WIFI_CONNECT;
            g_appData.statusChangeFlag |= CS_NETWORK_CHANGE;
            
            g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WIFI);
            g_windMgr->updateDate();
            WIFIMgr::ins()->scanWifi();
        }
        mNetChecking = false;
    }
    private:
    int64_t firstCheckNetTime;
    int mPingFailCount;
    bool mNetChecking;
};



int main(int argc, const char* argv[]) {

    // 识别OTA结束脚本
    if (ghc::filesystem::exists("/customer/endOTA.sh")) {
        setScreenLight(0,false);
        std::system("mount -o remount,rw /customer");
        std::system("chmod 777 /customer/endOTA.sh");
        std::system("/customer/endOTA.sh");
        std::system("rm -rf /customer/endOTA.sh");
        sync();
        setScreenLight(0,false);
        std::system("reboot");
    }

    App app(argc, argv);
    cdroid::Context* ctx = &app;
    sleep(5);
    g_wifiMgr->init();

    // 配置
    g_objConf->init();

    // 平台通信
    g_T5Mgr->init();
    // g_objConnMgr->init();
    g_objBtnMgr->init();

    // CURL 初始化
    g_objCurl->init();

    // 线程池
    ThreadPool::ins()->init(2);

    g_windMgr->initWindow();

    //1s定时器
    CTickHanlder th;
    app.addEventHandler(&th);

    return app.exec();
}

