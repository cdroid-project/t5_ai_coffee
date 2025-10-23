/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:13:17
 * @FilePath: /t5_ai_coffee/src/windows/wifi_adapter.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#ifndef __TUYA_GW_H__
#define __TUYA_GW_H__ 

#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
    #include "tuya_cloud_com_defs.h"
#ifdef __cplusplus
}
#endif

#define TUYA_DP_CMD_POWER   1

#define TUYA_DP_CMD_START   2 //启动和停止

#define TUYA_DP_CMD_PAUSE   3 //开始或者继续

#define TUYA_DP_CMD_MODE   4 //烹饪模式


typedef std::function< void(const char *) >upgrade_success_listeners;
typedef std::function< bool(unsigned char dpid,const void *data) >dp_deal_cb;
typedef std::function< void(unsigned char netStatus)>net_deal_cb;
typedef std::function< void(const char *)>qrcode_deal_cb;

/// @brief 
/// @return 
int tuya_gw_init(const char *Uuid,const char *Key);
// 获取天气
int tuya_get_weather(int &conditionNum,int &temp,int &expiration);
// 上报程序所有的状态
int tuya_report_allstatus();
// 上报某个dp状态
int tuya_report_dp(TY_OBJ_DP_S &objDpStr);
// 上报某个 raw 透传型dp状态
int tuya_report_raw_dp(TY_RAW_DP_S &raw_dp_s);
// 获取时间
int tuya_get_time(time_t &timeTick);
// 解绑涂鸦设备
int tuya_unbind_device(bool isFactory);
/// @brief 
/// @param l 

// ota升级的回调
void tuya_gw_set_upgrade_cb(upgrade_success_listeners l);
// dp命令处理的回调
void tuya_dp_deal_cb(dp_deal_cb cb);
// 连接网络状态
void tuya_net_statue_cb(net_deal_cb cb);
// 涂鸦获取qrcode的回调
void tuya_qrcode_deal_cb(qrcode_deal_cb cb);

#endif