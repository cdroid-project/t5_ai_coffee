/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 17:42:15
 * @LastEditTime: 2025-10-23 14:50:42
 * @FilePath: /t5_ai_coffee/src/common/hv_icmp.h
 * @Description:
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#ifndef HV_ICMP_H_
#define HV_ICMP_H_

// @param cnt: ping count
// @return: ok count
// @note: printd $CC -DPRINT_DEBUG
int ping(const char* host, int cnt);

#endif // HV_ICMP_H_
