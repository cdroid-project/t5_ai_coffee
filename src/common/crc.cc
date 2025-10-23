/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:50:22
 * @FilePath: /t5_ai_demo/src/common/crc.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#include "crc.h"

//===============================================================================
// 表格-CRC
const ushort tCRCData[] = {0x0000,
                       0x1081,
                       0x2102,
                       0x3183,
                       0x4204,
                       0x5285,
                       0x6306,
                       0x7387,
                       0x8408,
                       0x9489,
                       0xa50a,
                       0xb58b,
                       0xc60c,
                       0xd68d,
                       0xe70e,
                       0xf78f};

//===============================================================================
// CRC校验计算函数
// 其中vInit 固定为0xffff   ptr为校验数组首地址    len为校验数组长度
ushort Fuc_GetCRC(ushort vInit, uchar *ptr, uchar len)
{
    ushort vCRC = vInit;
    while (len-- != 0)
    {
        vCRC = ((vCRC >> 4) ^ tCRCData[(vCRC & 0x0f) ^ ((*ptr) & 0x0f)]);
        vCRC = ((vCRC >> 4) ^ tCRCData[(vCRC & 0x0f) ^ ((*ptr) >> 4)]);
        ptr++;
    }
    return ~vCRC;
}

//===============================================================================
/*/ 程序样例
vCRC=((ushort)(gUartData_RxD[8])<<8)+gUartData_RxD[9];
if(     ( Fuc_GetCRC( 0xffff , gUartData_RxD , 8 ) != vCRC )
    ||  ( gUartData_RxD[0] != cUart_Ack )   )
{   fUart_ErrFmData = 1 ;
    gUartErrCnt++ ;
}// */

/*CRC-16 CCITT*/
// CCITT: x^16 + x^12 + x^5 + x^0 (0x1021)
/*高位在左低位在右，使用时请注意高低位顺序！！！*/
ushort CRC16_CCITT(uchar *pdata, int len)
{
    ushort crc = 0x0000;   //初始值
    int            i, j;
    for (j = 0; j < len; j++)
    {
        crc ^= pdata[j];

        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x0001) > 0)
            {
                crc = (crc >> 1) ^ 0x8408;   // 0x1021 翻转 0x8408
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}


ushort CRC16_Modbus(uchar *pdata, int len)
{
    ushort crc = 0xffff;   //初始值
    int            i, j;
    for (j = 0; j < len; j++)
    {
        crc ^= pdata[j];

        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x0001) > 0)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}