/*
 * @Author: chenyang chenyang@sanboen.com
 * @Date: 2025-08-06 15:22:39
 * @LastEditors: chenyang chenyang@sanboen.com
 * @LastEditTime: 2025-08-07 23:18:45
 * @FilePath: /t5_ai_coffee/conf/version.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __CONF_VERSION_H__
#define __CONF_VERSION_H__

#include <stdio.h>
#include "hv_series_conf.h"

#define HV_SOFT_VER_MAJ "01"
#define HV_SOFT_VER_MIN "00"

#define HV_SOFT_VER_EXT "1.1.6.3"
#define HV_BETA_BER_EXT "255.1.6.3"
#define HV_LOWEST_VER_EXT "1.0.0.0"

#define HV_TUYA_VER_EXT "1.6.3"

#define HV_SOFT_VER_TIME "2025-10-01 10:33.24 UTC"

#define HV_SOFT_GIT_HARD "820ad3ec3b1bbd3e1099fbc5eefc36676cf48bf3"

#define HV_SOFT_VERSION "V" HV_CPU_NAME "." HV_SOFT_VER_MAJ "." HV_SOFT_VER_MIN "." HV_SOFT_VER_EXT "." HV_SOFT_VER_TIME
#define HV_SOFT_VERSION_NEW HV_SOFT_VER_MAJ "." HV_SOFT_VER_EXT "." HV_SOFT_VER_MIN "." HV_SOFT_VER_TIME


#define HV_OS_VER_S "Linux 4.4"

#define HV_HARD_VER_S "A0.01.00"

#define HV_HARD_VERSION "V" HV_CPU_NAME "." HV_HARD_VER_S


/// @brief 获取格式化后的软件版本，用于展示
/// @param _szBuf 
/// @return 
static inline char* HV_FORMAT_VERSION_STRING(char* _szBuf)
{
	sprintf(_szBuf, "xx版本: %s",HV_SOFT_VER_EXT);
	return _szBuf;
}

/// @brief 获取格式化后的 Beta软件版本，用于展示
/// @param _szBuf 
/// @return 
static inline char* HV_BETA_VERSION_STRING(char* _szBuf)
{
	sprintf(_szBuf, "xx版本: Beta_%s",HV_BETA_BER_EXT);
	return _szBuf;
}

/// @brief 获取当前版本构建时间
/// @return 
static inline const char* HV_FORMAT_VER_TIME_STRING()
{
	return HV_SOFT_VER_TIME;
}

/// @brief 获取当前版本号
/// @return 
static inline const char* HV_FORMAT_VER_EXT_STRING()
{
	return HV_SOFT_VER_EXT;
}

static inline const char* HV_FORMAT_GIT_HARD_STRING()
{
	return HV_SOFT_GIT_HARD;
}

#endif
