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


#include "cdtypes.h"
#include "cdlog.h"
#include <functional>
#include "ghc/filesystem.hpp"
#ifdef __cplusplus
extern "C" {
#endif
    #include "tuya_cloud_types.h"
    #include "tuya_iot_com_api.h"
    #include "tuya_iot_base_api.h"

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include "gw_intf.h"
    #include <fcntl.h>
    #include <signal.h>
    #include "base_event.h"
    #include "tuya_svc_upgrade.h"

    #include "hv_series_conf.h"
    #include "hv_version.h"
    #include "tuya_matop.h"
    #include "tkl_system.h"
    #include "tal_log.h"

#ifdef __cplusplus
}
#endif
#include "tuya_gw.h"
// #include "tuya_gw_atop.h"

#define OTA_DIR  "/appconfigs/customer_bak"

namespace fs = ghc::filesystem;

static char current_path[128];

upgrade_success_listeners upgrade_success_cb=nullptr;
dp_deal_cb dp_callback = nullptr;
net_deal_cb net_callback = nullptr;
qrcode_deal_cb qrcode_callback = nullptr;

#define PID         "jykjalgskjumxgkj"

char authUuid[64];
char authKey[64];

#if 1

STATIC VOID __soc_handle_init_args(INT_T argc, CHAR_T *argv[], 
                                CHAR_T* pid, CHAR_T* uuid, CHAR_T* authkey, CHAR_T* version)
{
    
    CHAR_T* cfg;
    CHAR_T* name;
    INT_T fd = -1;
    INT_T len = 4096;
    CHAR_T buffer[4096] = {0};
    
    // 注意，启动方式不同，可能会导致设备恢复出厂设置
    name = argv[0];

    // cfg，包含pid，uuid，authkey
    if (argc > 2) {
        cfg = argv[2];

        // 一个json配置文件
        fd = open(cfg, O_RDONLY);
        while (len == read(fd, buffer, SIZEOF(buffer)));
        close(fd);

        // 解析JSON
        ty_cJSON *root = ty_cJSON_Parse(buffer);
        if (root) {
            strcpy(pid, ty_cJSON_GetObjectItem(root, "pid")->valuestring);
            strcpy(uuid, ty_cJSON_GetObjectItem(root, "uuid")->valuestring);
            strcpy(authkey, ty_cJSON_GetObjectItem(root, "authkey")->valuestring);
        } else {
            printf("paser %s failed\n", cfg);
        }
    } else {
        strcpy(pid, PID);
        strcpy(uuid, authUuid);
        strcpy(authkey, authKey);
    }

    printf("%s, version %s, pid %s, uuid %s, authkey %s\n", name, version, pid, uuid, authkey);
}




// 升级文件下载完成回调
STATIC OPERATE_RET __ota_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    FILE *p_upgrade_fd = (FILE *)pri_data;
    fclose(p_upgrade_fd);

    if(download_result == 0) {
        LOGD("SOC Upgrade File Download Success");
        // UserTODO
        if(upgrade_success_cb)upgrade_success_cb(current_path);
    }else {
        LOGE("SOC Upgrade File Download Fail.ret = %d", download_result);
        return OPRT_FILE_WRITE_FAILED;
    }

    return OPRT_OK;
}


// 升级文件下载回调
STATIC OPERATE_RET __ota_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                      IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    FILE *p_upgrade_fd = (FILE *)pri_data;
    fwrite(data, 1, len, p_upgrade_fd);
    *remain_len = 0;
    return OPRT_OK;
}



// SOC设备升级入口
STATIC INT_T __soc_dev_rev_upgrade_info_cb(IN CONST FW_UG_S *fw)
{
    LOGD("fw->tp:%d", fw->tp);
    LOGD("fw->fw_url:%s", fw->fw_url);
    LOGD("fw->fw_hmac:%s", fw->fw_hmac);
    LOGD("fw->sw_ver:%s", fw->sw_ver);
    LOGD("fw->file_size:%u", fw->file_size);
    LOGD("fw->diff_ota:%u", fw->diff_ota);

    //存升级包
    FILE *p_upgrade_fd = NULL;
    if (!fw->diff_ota){
        
        snprintf(current_path,sizeof(current_path),"%s/%s.ota",OTA_DIR,fw->sw_ver);
        p_upgrade_fd = fopen(current_path,"w+b");
    }
    
    if(NULL == p_upgrade_fd){
        LOGE("open upgrade file fail. upgrade fail %d", fw->diff_ota);
        return OPRT_COM_ERROR;
    }
    

    OPERATE_RET op_ret = tuya_iot_upgrade_gw(fw, __ota_get_file_data_cb, __ota_upgrade_notify_cb, p_upgrade_fd);
    if(OPRT_OK != op_ret) {
        LOGE("tuya_iot_upgrade_gw err:%d",op_ret);
    }

    return op_ret;
}
// 升级相关代码结束




// SOC设备云端状态变更回调
STATIC VOID __soc_dev_status_changed_cb(IN CONST GW_STATUS_E status)
{
    LOGE("SOC TUYA-Cloud Status:%d", status);

}



// SOC设备格式化指令数据下发入口
STATIC VOID __soc_dev_obj_dp_cmd_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    LOGD("SOC Rev DP Obj Cmd t1:%d t2:%d CNT:%u", dp->cmd_tp, dp->dtt_tp, dp->dps_cnt);
    if(dp->cid != NULL) LOGE("soc not have cid.%s", dp->cid);

    std::time_t    t   = std::time(NULL);
    UINT_T index = 0;
    for(index = 0; index < dp->dps_cnt; index++)
    {
        CONST TY_OBJ_DP_S *p_dp_obj = dp->dps + index;
        LOGI("idx:%d dpid:%d type:%d ts:%u nowTime = %u", index, p_dp_obj->dpid, p_dp_obj->type, p_dp_obj->time_stamp,t);
        bool isDeal = false;
        switch (p_dp_obj->type) {
            case PROP_BOOL:     { LOGD("bool value:%d", p_dp_obj->value.dp_bool);   if(dp_callback) isDeal = dp_callback(p_dp_obj->dpid,&p_dp_obj->value.dp_bool); break;}
            case PROP_VALUE:    { LOGD("INT value:%d", p_dp_obj->value.dp_value);   if(dp_callback) isDeal = dp_callback(p_dp_obj->dpid,&p_dp_obj->value.dp_value);break;}
            case PROP_STR:      { LOGD("str value:%s", p_dp_obj->value.dp_str);     if(dp_callback) isDeal = dp_callback(p_dp_obj->dpid,&p_dp_obj->value.dp_str);break;}
            case PROP_ENUM:     { LOGD("enum value:%u", p_dp_obj->value.dp_enum);   if(dp_callback) isDeal = dp_callback(p_dp_obj->dpid,&p_dp_obj->value.dp_enum);break;}
            case PROP_BITMAP:   { LOGD("bits value:0x%X", p_dp_obj->value.dp_bitmap); if(dp_callback) isDeal = dp_callback(p_dp_obj->dpid,&p_dp_obj->value.dp_bitmap);break;}
            default:            { LOGE("dpid:%d type:%d ts:%u is invalid", p_dp_obj->dpid, p_dp_obj->type, p_dp_obj->time_stamp); break;}
        }

        if(isDeal) {
            OPERATE_RET op_ret = dev_report_dp_json_async(dp->cid,dp->dps+index ,1);
            if(OPRT_OK != op_ret) LOGE("dev_report_dp_json_async op_ret:%d",op_ret);
            // LOGI("dev_report_dp_json_async dpid = %d",(dp->dps+index)->dpid);
        }     
    }
    
    // // 用户处理完成之后需要主动上报最新状态，这里简单起见，直接返回收到的数据，认为处理全部成功。
    // OPERATE_RET op_ret = dev_report_dp_json_async(dp->cid,dp->dps,dp->dps_cnt);
    // if(OPRT_OK != op_ret) {
    //     LOGE("dev_report_dp_json_async op_ret:%d",op_ret);
    // }
    
    return;
}

// typedef enum {
//     GW_LOCAL_RESET_FACTORY = 0, // 本地恢复出厂重置
//     GW_REMOTE_UNACTIVE,         // App 普通重置（解除绑定）
//     GW_LOCAL_UNACTIVE,          // 本地普通重置（解除绑定）
//     GW_REMOTE_RESET_FACTORY,    // App 恢复出厂重置
//     GW_RESET_DATA_FACTORY, //need clear local data when active // 激活时数据重置（详见 FAQ）
// } GW_RESET_TYPE_E;

// SOC设备进程重启请求入口
STATIC VOID __soc_dev_restart_req_cb(GW_RESET_TYPE_E type)
{
    LOGE("SOC Rev Restart Req %d", type);
    if (type == GW_LOCAL_UNACTIVE || type == GW_LOCAL_RESET_FACTORY || type == GW_REMOTE_UNACTIVE || type == GW_REMOTE_RESET_FACTORY) {
        // UserTODO 设备进程重启
        system("reboot");
    }
}




// SOC设备透传指令数据下发入口
STATIC VOID __soc_dev_raw_dp_cmd_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    LOGI("SOC Rev DP Raw Cmd t1:%d t2:%d dpid:%d len:%u", dp->cmd_tp, dp->dtt_tp, dp->dpid, dp->len);
    if(dp->cid != NULL) LOGE("soc not have cid.%s", dp->cid);
    // UserTODO
    bool isDeal = false;
    if(dp_callback) isDeal = dp_callback(dp->dpid,dp->data);
    if(isDeal) {
        OPERATE_RET op_ret = dev_report_dp_raw_sync(dp->cid,dp->dpid,dp->data,dp->len,0);
        if(OPRT_OK != op_ret) {
            LOGE("dev_report_dp_json_async op_ret:%d",op_ret);
        }
    }
    
    return;

    // // 用户处理完成之后需要主动上报最新状态，这里简单起见，直接返回收到的数据，认为处理全部成功。
    // OPERATE_RET op_ret = dev_report_dp_raw_sync(dp->cid,dp->dpid,dp->data,dp->len,0);
    // if(OPRT_OK != op_ret) {
    //     LOGE("dev_report_dp_json_async op_ret:%d",op_ret);
    // }
}




// SOC设备特定数据查询入口
STATIC VOID __soc_dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry)
{
    LOGD("SOC Rev DP Query Cmd");
    if(dp_qry->cid != NULL) LOGE("soc not have cid.%s", dp_qry->cid);

    if(dp_qry->cnt == 0) {
        LOGD("soc rev all dp query");
        // UserTODO
    }else {
        LOGD("soc rev dp query cnt:%d", dp_qry->cnt);
        UINT_T index = 0;
        for(index = 0; index < dp_qry->cnt; index++) {
            LOGD("rev dp query:%d", dp_qry->dpid[index]);
            // UserTODO
        }
    }
}




STATIC INT_T __qrcode_printf(CHAR_T *msg)
{
    if(qrcode_callback) qrcode_callback(msg);
    return 0;
}

// TuyaOS获取到短链接之后调用此接口输出qrcode打印
STATIC VOID __qrcode_active_shourturl_cb(CONST CHAR_T *shorturl)
{
    if (NULL == shorturl) {
        return;
    }
    LOGE("shorturl : %s", shorturl);
    
    ty_cJSON *item = ty_cJSON_Parse(shorturl);
    __qrcode_printf(ty_cJSON_GetObjectItem(item, "shortUrl")->valuestring);
    ty_cJSON_Delete(item);

    return;
}





// SOC外网状态变动回调
STATIC VOID __soc_dev_net_status_cb(IN CONST GW_BASE_NW_STAT_T stat)
{
    LOGE("network status:%d", stat);
    if(net_callback) net_callback(stat);        
}



VOID upgrade_detect_result(IN BOOL_T is_err, IN BOOL_T is_exe, IN UINT_T detect_interval)
{


}


// SIGIPE 处理
STATIC VOID __soc_handle_signal(INT_T sig)
{
    //不做任何处理即可
    LOGD("recv signal %d", sig);
}




// 重置ota handler，由demo自行处理ota
STATIC OPERATE_RET __soc_handle_run_event(VOID *data)
{
    LOGD("__soc_handle_run_event");
    tuya_svc_upgrade_register_pre_cb(NULL);
    TUYA_UPGRADE_DETECT_T detect={0};

    return OPRT_OK;
}


/// @brief 
/// @param argc 
/// @param argv 
/// @return 
int tuya_gw_init(const char *Uuid,const char *Key)
{
    strcpy(authUuid,Uuid);
    strcpy(authKey,Key);

    fs::path pdir(OTA_DIR);

    if(!fs::exists(pdir) || !fs::is_directory(pdir))
    {
        fs::remove_all(pdir);
        fs::create_directory(pdir);
    }

    OPERATE_RET rt = OPRT_OK;
     // 支持用户入参定制PID，UUID，AUTHKEY以模拟多种类型产品
    CHAR_T pid[PRODUCT_KEY_LEN+1];
    CHAR_T uuid[GW_UUID_LEN+1];
    CHAR_T authkey[AUTH_KEY_LEN+1];
    // CHAR_T version[SW_VER_LEN+1];
    strcpy(pid,PID);

    strcpy(uuid,authUuid);
    strcpy(authkey,authKey);

    // __soc_handle_init_args(argc, argv, pid, uuid, authkey, version);

    // IGNORE SIGPIPE, 避免挂起     
    signal(SIGPIPE, __soc_handle_signal);

    // 主联网固件ota使用demo自己实现的ota机制，需要覆盖基线提供的ota机制
    ty_subscribe_event(EVENT_RUN,HV_SERIES_NAME,__soc_handle_run_event, SUBSCRIBE_TYPE_ONETIME);

    // 初始化TuyaOS开发框架，需要手动创建一下数据存储目录
    rt= system("mkdir -p /appconfigs/tuya_db_files/");
    TUYA_CALL_ERR_LOG(tuya_iot_init_params("/appconfigs/tuya_db_files/", NULL));

    TAL_PR_NOTICE("sdk_info:%s", tuya_iot_get_sdk_info());                        /* print SDK information */
    TAL_PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);              /* print firmware compilation time */
    TAL_PR_NOTICE("system reset reason:[%d]", tal_system_get_reset_reason(NULL)); /* print system reboot causes */


    // 设置授权信息
    GW_PROD_INFO_S prod_info = {uuid, authkey};
    TUYA_CALL_ERR_RETURN(tuya_iot_set_gw_prod_info(&prod_info));

    rt = tal_log_set_manage_attr(TAL_LOG_LEVEL_WARN);
    if(OPRT_OK != rt) {
        LOGE("tal_log_set_manage_attr err:%d", rt);
        return -3;
    }
    
    // 初始化TuyaOS产品信息
    TY_IOT_CBS_S iot_cbs = {0};
    iot_cbs.gw_status_cb = __soc_dev_status_changed_cb;
    iot_cbs.gw_ug_cb = __soc_dev_rev_upgrade_info_cb;
    iot_cbs.gw_reset_cb = __soc_dev_restart_req_cb;
    iot_cbs.dev_obj_dp_cb = __soc_dev_obj_dp_cmd_cb;
    iot_cbs.dev_raw_dp_cb = __soc_dev_raw_dp_cmd_cb;
    iot_cbs.dev_dp_query_cb = __soc_dev_dp_query_cb;
#if (defined(ENABLE_QRCODE_ACTIVE) && (ENABLE_QRCODE_ACTIVE == 1)) && (!(defined(ENABLE_WIFI_QRCODE) && (ENABLE_WIFI_QRCODE == 1)))
    iot_cbs.active_shorturl = __qrcode_active_shourturl_cb;
#endif
    LOGE("this version:%s",HV_TUYA_VER_EXT);
    TUYA_CALL_ERR_RETURN(tuya_iot_soc_init(&iot_cbs, PID, HV_TUYA_VER_EXT));

    // 设置网络状态回调
    rt = tuya_iot_reg_get_nw_stat_cb(__soc_dev_net_status_cb);
    if(OPRT_OK != rt) {
        LOGE("tuya_iot_reg_get_nw_stat_cb err:%d", rt);
        return -4;
    }

//     ty_subscribe_event(EVENT_LINK_UP, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
//     ty_subscribe_event(EVENT_LINK_DOWN, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
//     ty_subscribe_event(EVENT_MQTT_CONNECTED, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
    
// #ifdef ENABLE_BT_SERVICE
//     tuya_ble_enable_debug(false);
// #endif

    return 0;
}

// 获取天气
int tuya_get_weather(int &conditionNum,int &temp,int &expiration){
    OPERATE_RET ret = OPRT_OK;
    ty_cJSON *result; 
    char body[] = "{\"codes\":\"[\\\"w.temp\\\",\\\"w.conditionNum\\\"]\"}";   // 获取温度、天气
    ret = iot_httpc_common_post_simple("tuya.device.public.weathers.get", "1.0", body, NULL, &result);
    if(ret != OPRT_OK) {
        LOGE("iot_httpc_common_post_simple err:%d", ret);
        return -1;
    }
    if(!result){
        LOGE("iot_httpc_common_post_simple return is null err:-2");
        return -2;
    }

    LOGI("%s",ty_cJSON_PrintUnformatted(result));

    ty_cJSON *expirationJson = ty_cJSON_GetObjectItem(result,"expiration");
    if(expirationJson){
        expiration = expirationJson->valueint;
    }else{
        LOGE("iot_httpc_common_post_simple get data: expiration is empty! err:-3");
        return -3;
    }

    ty_cJSON *data = ty_cJSON_GetObjectItem(result, "data");
    if(data){
        ty_cJSON *conditionNumJson = ty_cJSON_GetObjectItem(data, "w.conditionNum");
        ty_cJSON *tempJson = ty_cJSON_GetObjectItem(data, "w.temp");
        if(conditionNumJson){
            conditionNum = std::stoi(conditionNumJson->valuestring);
        }else{
            LOGE("iot_httpc_common_post_simple get data: conditionNum is empty! err:-4");
            return -4;
        }
        if(tempJson){
            temp = tempJson->valueint;
        }else{
            LOGE("iot_httpc_common_post_simple get data: temp is empty! err:-5");
            return -5;
        }
    }else{
        LOGE("iot_httpc_common_post_simple get data: data is empty! err:-6");
        return -6;
    }

    ty_cJSON_Delete(result);

    return OPRT_OK;
}

int tuya_get_time(time_t &timeTick){
    // 获取当前时间
    POSIX_TM_S tm;
    memset(&tm, 0, SIZEOF(tm));

    // // 当前本地时间
    // tal_time_get_local_time_custom(0, &tm);

    // 当前本地时间毫秒
    SYS_TICK_T time_ms = tal_time_get_posix_ms();
    TIME_T sec = (TIME_T)(time_ms / 1000);
    UINT_T ms  = (UINT_T)(time_ms % 1000);
    tal_time_get_local_time_custom(sec, &tm);

    timeTick = tal_time_mktime(&tm);

    return OPRT_OK;
}

// 解绑涂鸦设备
int tuya_unbind_device(bool isFactory){
    if(isFactory)   tuya_iot_gw_reset();
    else            tuya_iot_gw_unactive();

    return OPRT_OK;
}

/// @brief 
/// @param l 
void tuya_gw_set_upgrade_cb(upgrade_success_listeners l){
    upgrade_success_cb=l;
}

void tuya_dp_deal_cb(dp_deal_cb cb){
    dp_callback = cb;
}
// 上报程序所有的状态
int tuya_report_allstatus(){
    return 0;
}

// 上报某个dp状态
int tuya_report_dp(TY_OBJ_DP_S &objDpStr){

    TY_OBJ_DP_S p_dp_obj = {.dpid = objDpStr.dpid,
        .type = objDpStr.type,
        .value = objDpStr.value,
        .time_stamp = static_cast<UINT_T>(std::time(NULL))};
    OPERATE_RET op_ret = dev_report_dp_json_async(nullptr,&p_dp_obj ,1);
    if(OPRT_OK != op_ret) LOGE("dev_report_dp_json_async op_ret:%d",op_ret);
    return op_ret;
}

// 上报某个 raw 透传型dp状态
int tuya_report_raw_dp(TY_RAW_DP_S &raw_dp_s){
    UINT_T t = static_cast<UINT_T>(std::time(NULL));

    LOGI("report raw dp dpid = %d raw_dp_s.len = %d",raw_dp_s.dpid,raw_dp_s.len);
    OPERATE_RET op_ret = dev_report_dp_raw_sync(nullptr,raw_dp_s.dpid,raw_dp_s.data,raw_dp_s.len,t);
    if(OPRT_OK != op_ret) {
        LOGE("dev_report_dp_raw_sync op_ret:%d",op_ret);
    }

    return op_ret;
}

void tuya_net_statue_cb(net_deal_cb cb){
    net_callback = cb;
}
void tuya_qrcode_deal_cb(qrcode_deal_cb cb){
    qrcode_callback = cb;
}
#endif
