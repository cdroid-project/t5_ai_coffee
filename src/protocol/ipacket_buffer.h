/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:26
 * @FilePath: /t5_ai_demo/src/protocol/ipacket_buffer.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __ipacket_buffer_h__
#define __ipacket_buffer_h__

#include <common.h>

typedef enum {
    BT_NULL = 0,
    BT_MCU,     // MCU
    BT_BTN,     // 按键
    BT_TCP,     // tcp调试工具
    BT_FACTORY, // 产测模式
    BT_T5,      // T5
} BufferType;

#pragma pack(1)
typedef struct {
    short type;   // 数据类型
    short slen;   // 分配缓冲区大小
    short len;    // 有效数据长度
    uchar buf[1]; // 缓冲区
} BuffData;
#pragma pack()

class IAck;
class IPacketBuffer {
public:
    virtual BuffData   *obtain()                    = 0; // 创建
    virtual void        recycle(BuffData *buf)                     = 0; // 回收
    virtual int         add(BuffData *buf, uchar *in_buf, int len) = 0; // 添加数据
    virtual bool        complete(BuffData *buf)                    = 0; // 数据完整
    virtual bool        compare(BuffData *src, BuffData *dst)      = 0; // 对比数据
    virtual bool        check(BuffData *buf)                       = 0; // 校验数据
    virtual std::string str(BuffData *buf)                         = 0; // 格式化字符串
    virtual void        check_code(BuffData *buf)                  = 0; // 生成校验码
    virtual IAck       *ack(BuffData *bf)                          = 0; // 转化成ack
};

// 显示板 -> MCU
class IAsk {
public:
    virtual void parse(BuffData *buf) = 0;
    virtual int getCMD() = 0;
    virtual void checkcode() = 0;
public:
    BuffData *mBf;
};

// MCU -> 显示板
class IAck {
public:
    virtual int  getType() = 0;                     //
    virtual int  getCMD()  = 0;                     //
    virtual int  getData(int pos) = 0;              //
    virtual void parse(BuffData *bf) = 0;           //
    virtual int add(uchar *bf, int len) = 0;        //
    virtual bool complete() = 0;                    //
    virtual bool check() = 0;            //
public:
    short  mDlen;
    short *mPlen;
    uchar *mBuf;
};

#endif
