/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:50:28
 * @FilePath: /t5_ai_demo/src/common/crc.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#ifndef __crc_h__
#define __crc_h__

#include "common.h"

//===============================================================================
// CRC校验计算函数
// 其中vInit 固定为0xffff   ptr为校验数组首地址    len为校验数组长度
ushort Fuc_GetCRC(ushort vInit, uchar *ptr, uchar len);


/*CRC-16 CCITT*/
// CCITT: x^16 + x^12 + x^5 + x^0 (0x1021)
// 起始值0x0000
/*高位在左低位在右，使用时请注意高低位顺序！！！*/
ushort CRC16_CCITT(uchar *pdata, int len);

/*CRC-16 MODBUS*/
// x^16 + x^15 + x^2 + 1 (0xA001)  
// 起始值0xFFFF
/*高位在左低位在右，使用时请注意高低位顺序！！！*/
ushort CRC16_Modbus(uchar *pdata, int len);
#endif
