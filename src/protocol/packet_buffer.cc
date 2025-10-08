
#include <iostream>
#include <iomanip>
#include "packet_buffer.h"

SDHWPacketBuffer::SDHWPacketBuffer(BufferType type):mType(type) {
    switch(mType){
        case BT_MCU:{
            mSND = new UI2MCU;
            mRCV = new MCU2UI;
            break;
        }case BT_BTN:{
            mSND = new UI2IIC;
            mRCV = new IIC2UI;
            break;
        }case BT_T5:{
            mSND = new MCU2AI;
            mRCV = new AI2MCU;
        }
    }
}

SDHWPacketBuffer::~SDHWPacketBuffer(){
    for (BuffData *bf : mBuffs){
        free(bf);
    }
    mBuffs.clear();
}

BuffData *SDHWPacketBuffer::obtain() {
    short len = 8;
    switch (mType) {
        case BT_MCU: { len = UI2MCU::BUF_LEN; break; }
        case BT_BTN: { len = IIC2UI::BUF_LEN; break; }
        case BT_T5: { len = AI2MCU::BUF_LEN; break; }
        default: break;
    }

    for (auto it = mBuffs.begin(); it != mBuffs.end(); it++) {
        BuffData *bf = *it;
        if (bf->type == mType && bf->slen == len) {
            bf->len = 0;
            mBuffs.erase(it);
            return bf;
        }
    }

    BuffData *bf = (BuffData *)calloc(1, sizeof(BuffData) + len);
    bf->type = mType;
    bf->slen = len;
    bf->len  = 0;
    bzero(bf->buf, bf->slen);

    return bf;
}

void SDHWPacketBuffer::recycle(BuffData *buf) {
    bzero(buf->buf, buf->slen);
    buf->len = 0;
    mBuffs.push_back(buf);
}

int SDHWPacketBuffer::add(BuffData *buf, uchar *in_buf, int len) {
    
    mRCV->parse(buf);
    return mRCV->add(in_buf, len);
}

bool SDHWPacketBuffer::complete(BuffData *buf) {
    mRCV->parse(buf);
    return mRCV->complete();
}

bool SDHWPacketBuffer::compare(BuffData *src, BuffData *dst) {    
    // 有按键数据每一帧都需要处理
    return false;
    if(mType == BT_MCU){
        return false;
        if (src->type != dst->type)
            return false;
        if (src->len != dst->len)
            return false;
        for (short i = 0; i < src->len; i++) {
            if (src->buf[i] != dst->buf[i])
                return false;
        }

    }else if(mType == BT_BTN){
        return false;
    }
    return true;
}

bool SDHWPacketBuffer::check(BuffData *buf) {
    mRCV->parse(buf);    
    return mRCV->check();
}

std::string SDHWPacketBuffer::str(BuffData *buf) {
    std::ostringstream oss;
    for (int i = 0; i < buf->len; i++) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)buf->buf[i] << " ";
    }
    return oss.str();
}

void SDHWPacketBuffer::check_code(BuffData *buf) {
    mSND->parse(buf);
    mSND->checkcode();
}

IAck *SDHWPacketBuffer::ack(BuffData *bf) {
    mRCV->parse(bf);
    return mRCV;
}
