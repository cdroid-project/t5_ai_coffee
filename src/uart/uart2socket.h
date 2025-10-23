/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:43:04
 * @FilePath: /t5_ai_demo/src/uart/uart2socket.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __uart2socket_h__
#define __uart2socket_h__

#include <common.h>

#define UART_PORT_LEN 16

typedef enum {
    main_cmd_uart = 1,
    main_cmd_data,
} main_cmd;

typedef enum {
    sub_cmd_uart_open_req = 1,
    sub_cmd_uart_open_rsp,    
} sub_cmd_uart;

typedef enum {
    sub_cmd_data_trans = 1,
    sub_cmd_heart_ask,
    sub_cmd_heart_ack,
} sub_cmd_data;

#pragma pack(1)
typedef struct tagUartHeader {
    ushort size;
    uchar  mcmd;
    uchar  scmd;
} UartHeader, UARTHEADER, *LPUARTHEADER;

typedef struct tagUartOpenReq {
    char  serialPort[UART_PORT_LEN]; // /dev/ttySxx
    int   speed;          // 串口速度
    uchar flow_ctrl;      // 数据流控制
    uchar databits;       // 数据位   取值为 7 或者8
    uchar stopbits;       // 停止位   取值为 1 或者2
    char  parity;         // 效验类型 取值为N,E,O,,S
} UartOpenReq;

typedef struct {
    int result;
}UartOpenRsp;

typedef struct tagDataTrans {
    uchar data[1];
}DataTrans;
#pragma pack()

#endif
