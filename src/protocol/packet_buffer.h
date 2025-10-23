/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:35
 * @FilePath: /t5_ai_demo/src/protocol/packet_buffer.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __packet_buffer_h__
#define __packet_buffer_h__

#include <list>
#include "ipacket_buffer.h"
#include "mcu_ui.h"

class SDHWPacketBuffer : public IPacketBuffer {
public:
    SDHWPacketBuffer(BufferType type);
    ~SDHWPacketBuffer();

protected:

protected:
    virtual BuffData   *obtain();              // 创建
    virtual void        recycle(BuffData *buf);                     // 回收
    virtual int         add(BuffData *buf, uchar *in_buf, int len); // 添加数据
    virtual bool        complete(BuffData *buf);                    // 数据完整
    virtual bool        compare(BuffData *src, BuffData *dst);      // 对比数据
    virtual bool        check(BuffData *buf);                       // 校验数据
    virtual std::string str(BuffData *buf);                         // 格式化字符串
    virtual void        check_code(BuffData *buf);                  // 生成校验码
    virtual IAck       *ack(BuffData *bf);                  // 转化成应答包
private:
    BufferType          mType;

    std::list<BuffData *> mBuffs;

    IAsk                *mSND;
    IAck                *mRCV;
};

#endif
