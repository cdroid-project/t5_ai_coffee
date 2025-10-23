/**
 * @file tuya_svc_lan_rtsp.h
 * @brief TUYA LAN rtsp service
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_SVC_LAN_RTSP_H__
#define __TUYA_SVC_LAN_RTSP_H__

#include "tuya_cloud_types.h"
#include "ty_cJSON.h"
#include "rtsp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief register stream src to rtsp
 *
 * @param[in] src stream info
 *
 * @note called after iot init, canbe in pre_device_init or device_init
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_svc_lan_reg_rtsp_stream_src(rtsp_stream_src_t src);

/**
 * @brief init rtsp
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_svc_lan_rtsp_init(VOID);

/**
 * @brief uninit rtsp
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_svc_lan_rtsp_uninit(VOID);

#ifdef __cplusplus
}
#endif
#endif //__TUYA_SVC_LAN_RTSP_H__

