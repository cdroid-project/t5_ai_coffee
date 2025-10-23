/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:43:39
 * @FilePath: /t5_ai_demo/src/protocol/tcp_client.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "tcp_client.h"
#include "cmd_handler.h"
#include <uart.h>
#include <core/app.h>

#define CONN_TCP 1

#define RECV_PACKET_TIME_SPACE 50  // epoll_wait 收包间隔ms
#define SEND_PACKET_TIME_SPACE 80 // 发包间隔ms

TcpClient::TcpClient(IPacketBuffer *ipacket, BufferType type, UartOpenReq &uartInfo, const std::string &ip,
                       short port, int recv_space)
    : mPacketBuff(ipacket), mBufType(type), mUartInfo(uartInfo), mIp(ip), mPort(port), mRecvSpace(recv_space) {

    mLastSendTime = 0; // 最后一次发包时间
    mSendCount    = 0;
    mRecvCount    = 0;
    mChkErrCount  = 0;
    mLastRecvTime = SystemClock::uptimeMillis() + 5000;
    mLastRecv     = mPacketBuff->obtain();
    mCurrRecv     = mPacketBuff->obtain();
    mLastSndHeart = SystemClock::uptimeMillis();
    mSerialOk     = 0;
    mLastDealDataTime = 0;
}

TcpClient::~TcpClient() {
    mPacketBuff->recycle(mLastRecv);
    mPacketBuff->recycle(mCurrRecv);

    for (BuffData *ask : mSendQueue) { mPacketBuff->recycle(ask); }
    mSendQueue.clear();
#if CONN_TCP
    if (mRecvSpace >= RECV_PACKET_TIME_SPACE) {
        App::getInstance().removeEventHandler(this);
    }
#else
    App::getInstance().removeEventHandler(this);
#endif
}

int TcpClient::init() {

    LOGI("Uart: %s %d-%d-%d-%d-%C %p", mUartInfo.serialPort, mUartInfo.speed, mUartInfo.flow_ctrl, mUartInfo.databits,
         mUartInfo.stopbits, mUartInfo.parity, this);

    SocketClient::init(mIp.c_str(), mPort);
    if (mRecvSpace >= RECV_PACKET_TIME_SPACE) {
        App::getInstance().addEventHandler(this);
    }
    return 0;
}

bool TcpClient::isOk() {
    bool is_ok = isConn();
#if CONN_TCP
    return is_ok && mSerialOk;
#endif
    return is_ok;
}

int TcpClient::readData() {
    int read_len;  
#if CONN_TCP
    read_len = Client::readData();
#else
    int64_t curr_tick = SystemClock::uptimeMillis();
    if (curr_tick - mLastRecvTime < mRecvSpace) { return 1; }
    mLastRecvTime = curr_tick;

    read_len = UART0_Recv(mFd, mRSBuf->last, mRSBuf->end - mRSBuf->last);
    if (read_len > 0) {
        mRSBuf->last += read_len;
    } else {
        LOGE("uart recv error. result=%d", read_len);
        return 0;
    }
#endif
    
    return read_len;
}

int TcpClient::onRecvData() {
    int    count    = 0;
    size_t data_len = mRSBuf->last - mRSBuf->pos;

    // 串口数据包
    int64_t oldRecvCount = mRecvCount;
    onUartData(mRSBuf->pos, data_len);
    mRSBuf->pos  = mRSBuf->start;
    mRSBuf->last = mRSBuf->pos;
    count        = mRecvCount - oldRecvCount;

    return count;
}

int TcpClient::onUartData(uchar *buf, int len) {
    int offset = 0;

    if (len > 0) { LOG(VERBOSE) << "len:" << len << " hex:" << hexstr(buf, len); }

    for (; offset < len;) {
        offset += mPacketBuff->add(mCurrRecv, buf + offset, len - offset);
        if (!mPacketBuff->complete(mCurrRecv)) { break; }
        // LOG(VERBOSE) << "complete:" << hexstr(mCurrRecv->buf, mCurrRecv->len);

        mRecvCount++;

        if (mPacketBuff->check(mCurrRecv)) {
            mChkErrCount = 0;
            // 检查重复数据包
            if (!mPacketBuff->compare(mCurrRecv, mLastRecv) || checkDealData()) {
                LOG(VERBOSE) << "new packet:" << mPacketBuff->str(mCurrRecv);
                IAck *ack = mPacketBuff->ack(mCurrRecv);
                g_objHandler->onCommand(ack);

                // 保存本次数据包
                mLastRecv->len = mCurrRecv->len;
                memcpy(mLastRecv->buf, mCurrRecv->buf, mCurrRecv->len);

                mLastSendTime = 0;
            }
        } else {
            mChkErrCount++;
            hexdump("uart recv data, check fail", mCurrRecv->buf, mCurrRecv->len);
        }

        mCurrRecv->len = 0;
    }

    return 0;
}

void TcpClient::onTick() {
    int64_t now_tick = SystemClock::uptimeMillis();

    static int64_t s_last_time = 0;
    if (now_tick - s_last_time >= 60000) {
        s_last_time = now_tick;
        LOG(DEBUG) << mUartInfo.serialPort << " packet info. send=" << mSendCount << " recv=" << mRecvCount
                   << " check_fail=" << mChkErrCount;
    }

    SocketClient::onTick();
#if CONN_TCP
    if (now_tick - mLastSndHeart >= HEART_TIME && isTimeout(HEART_TIME)) {
        mLastSndHeart = now_tick;
        sendHeart();
    }
#endif

    // 上一个包接收超时，发送队列不为空
    if (now_tick - mLastSendTime >= SEND_PACKET_TIME_SPACE && !mSendQueue.empty()) {
        mLastSendTime = now_tick;

        BuffData *ask = mSendQueue.front();
        mSendQueue.pop_front();

        mPacketBuff->check_code(ask);
        sendTrans(ask);
        mPacketBuff->recycle(ask);
        mSendCount++;
    }
}

int TcpClient::send(BuffData *ask) {

    if (!isConn()) {
        mPacketBuff->recycle(ask);
        return -1;
    }

    // 检查队列是否有相同的包还未发送
    for (auto it = mSendQueue.begin(); it != mSendQueue.end(); it++) {
        BuffData *dst = *it;
        if (mPacketBuff->compare(ask, dst)) {
            LOG(WARN) << "same package exists! str=" << mPacketBuff->str(ask);
            mPacketBuff->recycle(ask);
            return 1;
        }
    }

    mSendQueue.push_back(ask);

    return 0;
}

void TcpClient::onStatusChange() {
#if CONN_TCP
    SocketClient::onStatusChange();
    if (isConn()) { sendConn(); }
#else
    LOGE_IF(!isConn(), "errno=%d errstr=%s", errno, strerror(errno));
#endif
}

bool TcpClient::isTimeout(int out_time /* = 0*/) {
#if CONN_TCP
    return SocketClient::isTimeout(out_time);
#endif
    return false;
}

void TcpClient::sendConn() {
    char        buf[LEN_4K];
    UartHeader *head = (UartHeader *)buf;
    head->mcmd       = main_cmd_uart;
    head->scmd       = sub_cmd_uart_open_req;
    head->size       = sizeof(UartHeader);

    UartOpenReq *req = (UartOpenReq *)(buf + head->size);
    memcpy(req, &mUartInfo, sizeof(UartOpenReq));
    head->size += sizeof(UartOpenReq);

    sendData(buf, head->size);
    LOG(INFO) << "connect uart. name=" << req->serialPort;
}

void TcpClient::sendTrans(BuffData *ask) {
#if CONN_TCP
    char        buf[LEN_4K];
    UartHeader *head = (UartHeader *)buf;
    head->mcmd       = main_cmd_data;
    head->scmd       = sub_cmd_data_trans;
    head->size       = sizeof(UartHeader);

    memcpy(buf + head->size, ask->buf, ask->len);
    head->size += ask->len;

    LOG(VERBOSE) << mFd << " sock send:" << hexstr(ask->buf, ask->len);
    sendData(buf, head->size);
#else
    LOG(VERBOSE) << mFd << " uart send:" << hexstr(ask->buf, ask->len);
    UART0_Send(mFd, ask->buf, ask->len);
    mLastRecvTime = SystemClock::uptimeMillis();
#endif
}

void TcpClient::sendHeart() {
#if CONN_TCP
    char        buf[LEN_4K];
    UartHeader *head = (UartHeader *)buf;
    head->mcmd       = main_cmd_data;
    head->scmd       = sub_cmd_heart_ask;
    head->size       = sizeof(UartHeader);

    int64_t tt = SystemClock::uptimeMillis();
    memcpy(buf + head->size, &tt, sizeof(int64_t));
    head->size += sizeof(int64_t);

    sendData(buf, head->size);
#endif
}

int TcpClient::checkEvents() {
    int64_t now_time_tick = SystemClock::uptimeMillis();
    
    if (now_time_tick - mLastRecvTime >= mRecvSpace) {
        mLastRecvTime = now_time_tick;
        return 1;
    }

    return 0;
}

int TcpClient::handleEvents(){    
#if CONN_TCP
    if (!isConn()) return 0;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(mFd, &read_fds);

    struct timeval vtime;
    vtime.tv_sec  = 0;
    vtime.tv_usec = 1000;
    int ready = select(mFd + 1, &read_fds, NULL, NULL, &vtime);
    if (ready < 0)
    {
        handleEvent(mFd, Looper::EVENT_INPUT, 0);
        return 0;
    }

    if (FD_ISSET(mFd, &read_fds))
    {
        handleEvent(mFd, Looper::EVENT_INPUT, 0);
        return 1;
    }

#else
    int read_len = UART0_Recv(mFd, mRSBuf->last, mRSBuf->end - mRSBuf->last);

    LOGI("uart recv. len=%d", read_len);

    if (read_len > 0) {
        mRSBuf->last += read_len;
    } else {        
        return 0;
    }

    onRecvData();
    return 1;
#endif

    return 0;
}

int TcpClient::getRecvSpace() {
    return mRecvSpace;
}

bool TcpClient::checkDealData(){
    int64_t now_time_tick = SystemClock::uptimeMillis();
    if (now_time_tick - mLastDealDataTime >= 1000) {
        mLastDealDataTime = now_time_tick;
        return true;
    }
    return false;
}
