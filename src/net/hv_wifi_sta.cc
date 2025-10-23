/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:46:54
 * @FilePath: /t5_ai_coffee/src/net/hv_wifi_sta.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "hv_wifi_sta.h"

#include <cstdio>
#include <iostream>
#include <regex>
#include <sys/prctl.h>
#include <unistd.h>

#include <comm_func.h>
#include <hv_series_conf.h>

#include <core/textutils.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "hv_icmp.h"
#include "hv_net.h"
#ifdef __cplusplus
}
#endif

using namespace std;

// 由于文件系统限制。只能指定工作目录运行
#define WIFI_CMD_DIR "/config/wifi/"

#define WPA_WORK_DIR "/tmp"

#define WPA_SUPPLICANT_SCRIPT WPA_WORK_DIR"/wpa_supplicant.conf"

#define WPA_SUPPLICANT_TIMEOUT_MS 60;

void parseWifiScan(std::list<WifiSta::WIFI_ITEM_S> &wifiCells, const std::string &scanString) {

    std::istringstream iss(scanString);
    std::string        line;
    bool               repNameFlag;
    while (std::getline(iss, line)) {
        if (line.find("Cell") != std::string::npos) {
            WifiSta::WIFI_ITEM_S cell;
   
            cell.mac = line.substr(line.find("Address:") + 9);  // 地址
            std::getline(iss, line);                            // Read ESSID line

            cell.ssid = line.substr(line.find("\"") + 1, line.rfind("\"") - line.find("\"") - 1);   // wifi 名
    
            while (!(iss.bad() || iss.fail())) {
                std::getline(iss, line);
                if(line.find("Encryption key:") != std::string::npos){
                    cell.isKey = line.find(":on") != std:: string::npos;
                }else if(line.find("Quality=") != std::string::npos) {
                    size_t startPos = line.find("Quality=") + std::string("Quality=").length();
                    size_t endPos = line.find("/", startPos);
                    int quality = std::stoi(line.substr(startPos, endPos - startPos));
                    startPos = endPos + 1;
                    endPos = line.find(" ", startPos);
                    int total = std::stoi(line.substr(startPos, endPos - startPos));

                    startPos = line.find("Signal level=") + std::string("Signal level=").length();
                    endPos = line.find(" ", startPos);
                    int signalLevel = std::stoi(line.substr(startPos, endPos - startPos));
                    // LOGI("ssid = %s  signalLevel = %d  quality = %d/%d",cell.ssid.c_str(),signalLevel,quality,total);
                    cell.signal = calculation_signal((float)quality/(float)total,signalLevel);
                    cell.quality = quality;
                    cell.signalLevel = signalLevel;
                    break;
                }
            }

            if (cell.ssid.size() <= 0) { continue; }
            // else std::cout << "ESSID: " << cell.ssid << std::endl;

            repNameFlag = false;
            for (auto &item : wifiCells) {
                if (item.ssid == cell.ssid) { 
                    if(cell.signal > item.signal) item.signal = cell.signal;
                    repNameFlag = true;
                    break; 
                }
            }
            if(!repNameFlag)  wifiCells.push_back(cell);
        }
    }
}

/// @brief
/// @param ucSSID
/// @param ucKey
/// @return
static int __save_conf(const std::string &ssid, const std::string &key) {
    FILE *pFile = NULL;

    pFile = fopen(WPA_SUPPLICANT_SCRIPT, "wt");
    if (pFile == nullptr) {
        perror("open file wpa config error:");
        return -1;
    }

    if (pFile != NULL) {
        fprintf(pFile, "ctrl_interface=%s/wpa_supplicant\n", WPA_WORK_DIR);
        fprintf(pFile, "update_config=1\n");
        fprintf(pFile, "ap_scan=1\n");
        fprintf(pFile, "network={\n");
        fprintf(pFile, "\tssid=\"%s\"\n", ssid.c_str());
        fprintf(pFile, "\tscan_ssid=1\n");
        // 判断密码有无来决定wifi配置文件加密方式
        fprintf(stderr, "key:%d\n", (int)key.length());
        if (key.length() == 0) // NONE
        {
            printf("no passwd!!\n");
            fprintf(pFile, "\tkey_mgmt=NONE\n");
        } else {
            printf("need passwd!! default WPA-PSK\r\n");
            fprintf(pFile, "\tkey_mgmt=WPA-PSK WPA-EAP IEEE8021X NONE\n");
            fprintf(pFile, "\tpairwise=TKIP CCMP\n");
            fprintf(pFile, "\tgroup=CCMP TKIP WEP104 WEP40\n");
            fprintf(pFile, "\tpsk=\"%s\"\n", key.c_str());
        }
        fprintf(pFile, "}");
        fclose(pFile);
        pFile = NULL;
    } else {
        printf("open %s Error!!!\n", WPA_SUPPLICANT_SCRIPT);
        return -1;
    }
    return 0;
}

/// @brief 重新启动wpa进程
static void __kill_wpa() {
    HV_POPEN("killall -9 udhcpc");
    HV_POPEN("ifconfig wlan0 down");
    HV_POPEN("ifconfig wlan0 up");
    HV_POPEN("killall -9 wpa_supplicant");
}

/*****************************************************/
WifiSta::WifiSta(/* args */) {
    mCurThreadInfo = 0;
    mWifiConnStatus= E_DISCONNECT;
#if HV_FUNCTION_WIFI == 1
    const char *command = WIFI_CMD_DIR "ssw01bInit.sh";
    FILE       *fp      = popen(command, "r");
    if (fp == nullptr) { std::cout << "exe cmd error:" << command << std::endl; }
    pclose(fp);
    const char *command1 = "ifconfig wlan0 down";
    fp                   = popen(command1, "r");
    pclose(fp);
    const char *command2 = "ifconfig wlan0 up";
    fp                   = popen(command2, "r");
    pclose(fp);
#endif
}

WifiSta::~WifiSta() {
    if (mCurThreadInfo) { mCurThreadInfo->running = false; }
}

/// @brief
/// @param wifi_lsit
/// @return
int32_t WifiSta::scan(std::list<WIFI_ITEM_S> &wifilist) {
#if HV_FUNCTION_WIFI == 1
#define MAX_BUFFER_SIZE 1024 * 128
    char        buffer[MAX_BUFFER_SIZE] = {0};
    const char *command                 = WIFI_CMD_DIR "iwlist " HV_WLAN_NAME " scan";
    FILE       *fp                      = popen(command, "r");
    if (fp == nullptr) {
        std::cout << "exe cmd error:" << command << std::endl << std::strerror(errno);
        return -1;
    }

    fread(buffer, 1, MAX_BUFFER_SIZE, fp);
    pclose(fp);
    std::list<WIFI_ITEM_S> tmp;
    wifilist.swap(tmp);
    parseWifiScan(wifilist, buffer);
    // for (const auto &cell : wifilist) {
    //     std::cout << "Address: " << cell.mac << std::endl;
    //     std::cout << "ESSID: " << cell.ssid << std::endl;
    //     std::cout << "isKey: " << cell.isKey << std::endl;
    //     std::cout << "signal: " << cell.signal << std::endl;
    // }
#else
    std::list<WIFI_ITEM_S> tmp;
    wifilist.swap(tmp);
    for (uint8_t i = 0; i < 10; i++) {
        WIFI_ITEM_S item;
        item.ssid = "test wifi-" + std::to_string(i) + "-test marquee----";
        item.mac  = "88:88:88:88:" + std::to_string(i);
        item.rssi = -86 + __rand_value(1, 10);
        item.isKey = true;
        item.signal = (i%4)+1;
        wifilist.push_front(item);
    }
#endif
    // 信号排序
    wifilist.sort([](const WIFI_ITEM_S &a, const WIFI_ITEM_S &b) -> bool { 
        if(a.signal == b.signal)  return a.signalLevel > b.signalLevel;
        else                      return a.signal > b.signal;
    });
    // for (const auto &cell : wifilist) {
    //     std::string ssidText = cell.ssid;
    //     LOGI("ssid = %s  signalLevel = %d  quality = %d",fillLength(ssidText,30,' ').c_str(),cell.signalLevel,cell.quality);
    // }
    return 0;
}

/// @brief 链接
/// @param name
/// @param key
/// @return
int32_t WifiSta::connect(const std::string &name, const std::string &key) {
    mMutex.lock();
    if (mCurThreadInfo) { mCurThreadInfo->running = false; }
    mMutex.unlock();

#if HV_FUNCTION_WIFI == 1
    // 保存配置文件。连接wifi
    __save_conf(name, key);
    mWifiConnStatus    = E_STA_CONENCTING;
#else
    usleep(2*1000*1000);
    mWifiConnStatus = E_CONNECT_SUCCESS;
#endif
    mCurThreadInfo     = new ThreadInfo(name, key);
    conenct_run(mCurThreadInfo);
    return 0;
}

/**
 * 处理 WPA 的控制状态。
 *
 */
WifiSta::CONENCT_STATUS_E WifiSta::handle_result(CONENCT_STATUS_E old, struct wifi_status_result_t &result) {
    CONENCT_STATUS_E state = old;
    printf("wifi sta state:%s\r\n", result.wpa_state);
    if (strcmp(result.wpa_state, "COMPLETED") == 0) {
        state = E_STA_CONENCTINGED;
    } else if (strcmp(result.wpa_state, "SCANNING") == 0) {
        // 校验超时。
        state = E_STA_CONENCTING;
    } else {
    }
    return state;
}

//
void WifiSta::conenct_run(ThreadInfo *lpInfo) {
#if HV_FUNCTION_WIFI == 1
    int32_t connectTimeOut = WPA_SUPPLICANT_TIMEOUT_MS;
    char    szGateway[16]  = {0};
    prctl(PR_SET_NAME, __func__);
    uint8_t pingCnt = 0;

GOTO_CONENNCTION_REFUSED:
    if (lpInfo->softResetCnt && lpInfo->softResetCnt <= 5) {
        HV_POPEN(WIFI_CMD_DIR "ssw01bInit.sh");
    } else if (lpInfo->softResetCnt > 5) {
        while (lpInfo->running) {
            printf("%s abnormal. May require a reboot\r\n", HV_WLAN_NAME);
        }
        mMutex.lock();
        if (lpInfo == mCurThreadInfo) { mCurThreadInfo = 0; }
        delete lpInfo;
        mMutex.unlock();
        return;
    }

    lpInfo->softResetCnt++;

    //
    __kill_wpa();

    // 启动脚本链接wifi
    system(WIFI_CMD_DIR "wpa_supplicant -B -Dnl80211 -i" HV_WLAN_NAME " -c" WPA_SUPPLICANT_SCRIPT);
    lpInfo->conn_status = E_STA_CONENCTING;
    struct timespec stTime;
    clock_gettime(CLOCK_MONOTONIC, &stTime);
    uint32_t old_timecount = stTime.tv_sec;
    printf("wpa_ctrl:conenct_run start\r\n");
    pingCnt = 0;
    while (lpInfo->running) {
        clock_gettime(CLOCK_MONOTONIC, &stTime);

        switch (lpInfo->conn_status) {
        case E_STA_CONENCTING:
            // 查询状态
            struct wifi_status_result_t result;
            if (wpa_ctrl_sta_status(&result) == 0) {
                lpInfo->conn_status = handle_result(lpInfo->conn_status, result);
                old_timecount       = stTime.tv_sec;
            }

            if ((stTime.tv_sec > old_timecount && stTime.tv_sec - old_timecount > 60)) {
                printf("connection timed out\r\n");
                int rssi;
                if (wpa_ctrl_sta_get_near_ap_list(lpInfo->ssid.c_str(), &rssi) == 0) {
                    lpInfo->conn_status = E_STA_CONENCTING;
                } else {
                    printf("ap not find\r\n");
                }
                goto GOTO_CONENNCTION_REFUSED;
            }
            /* code */
            break;
        // 连接完成 分配或设置IP
        case E_STA_CONENCTINGED:
            system("udhcpc -i " HV_WLAN_NAME " -s /etc/init.d/udhcpc.script");
            lpInfo->conn_status = E_CONENCT_STA_DHCP;
            break;
        // 检查IP 地址 和 MAC 不为空
        case E_CONENCT_STA_DHCP:

            char szIP[16];
            char szMac[32];
            HV_NET_GetIpAddr(HV_WLAN_NAME, szIP);
            HV_NET_GetMacAddr(HV_WLAN_NAME, NULL, szMac);

            if (szIP[0] == '\0') {
                // 软复位
                printf("no correct ip!!!!,softreset\r\n");
                goto GOTO_CONENNCTION_REFUSED;
            }

            if (szMac[0] == '\0') {
                // 软复位
                printf("no correct mac!!!!,softreset\r\n");
                goto GOTO_CONENNCTION_REFUSED;
            }

            if (szGateway[0] == 0) { HV_NET_GetGateway(HV_WLAN_NAME, (char *)szGateway); }
            lpInfo->conn_status = E_CONENCT_STA_GUARD;
            break;
        case E_CONENCT_STA_GUARD:

            // 还没获取到网关
            if (szGateway[0] == 0) {
                HV_NET_GetGateway(HV_WLAN_NAME, (char *)szGateway);
                if (szGateway[0] == 0) { goto GOTO_CONENNCTION_REFUSED; }
            }

            if (stTime.tv_sec - old_timecount > 5) {
                old_timecount = stTime.tv_sec;

                if (ping((char *)szGateway, 1) > 0) {
                    pingCnt = 0;
                } else {
                    pingCnt++;
                    printf("NetWork ping failed cnt:%d %s\r\n", pingCnt, szGateway);
                }
            }

            if (pingCnt > 10) {
                // 网卡异常
                if (HV_NET_CheckNicStatusUp(HV_WLAN_NAME) != 0) {
                    printf("Network card abnormality\r\n");
                    goto GOTO_CONENNCTION_REFUSED;
                }

                // 网络连接异常
                else if (HV_NET_CheckNicStatus(HV_WLAN_NAME) != 0) {
                    // 重新连接wifi
                    printf("Network link abnormality\r\n");
                    goto GOTO_CONENNCTION_REFUSED;
                }
            }
            mWifiConnStatus = E_CONNECT_SUCCESS;
            break;
        default: break;
        }
        usleep(500*1000);
        LOGV("mWifiConnStatus = %d lpInfo->conn_status = %d",mWifiConnStatus,lpInfo->conn_status);
        if(mWifiConnStatus == E_CONNECT_SUCCESS) break;
    }
#endif

    mMutex.lock();    
    if (lpInfo == mCurThreadInfo) { mCurThreadInfo = 0; }
    delete lpInfo;
    mMutex.unlock();
}

WifiSta::CONENCT_STATUS_E WifiSta::get_status() {
    WifiSta::CONENCT_STATUS_E status = mWifiConnStatus;
    mMutex.lock();    
    if (mCurThreadInfo) { status = mCurThreadInfo->conn_status; }    
    mMutex.unlock();
    return status;
}

void WifiSta::disconnect() {
    mMutex.lock();
    if (mCurThreadInfo) { mCurThreadInfo->running = false; }
    mMutex.unlock();
    mWifiConnStatus = E_DISCONNECT;
}

// void WifiSta::resetWifi(){
    
//     // 启动脚本链接wifi
//     // disconnect();
//     __kill_wpa();
//     system(WIFI_CMD_DIR "wpa_supplicant -B -Dnl80211 -i" HV_WLAN_NAME " -c" WPA_SUPPLICANT_SCRIPT);
// }

void WifiSta::disEnableWifi(){
    HV_POPEN("ifconfig wlan0 down");
}
void WifiSta::enableWifi(){
    HV_POPEN(WIFI_CMD_DIR "ssw01bInit.sh"); 
    HV_POPEN("ifconfig wlan0 up");
}