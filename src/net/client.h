/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:46:21
 * @FilePath: /t5_ai_coffee/src/net/client.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __client_h__
#define __client_h__

#include <arpa/inet.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/syscall.h> // for SYS_xxx definitions
#include <unistd.h>      // for syscall()

#include <comm_func.h>
#include <common.h>

#define LEN_4K 4096

typedef struct {
    uchar *start;
    uchar *pos;
    uchar *last;
    uchar *end;
    uchar  buf[1];
} SockBuf;

class Client : public LooperCallback, public EventHandler {
public:
    constexpr static int TIME_OUT   = 30000; // δ���յ���Ϣ��ʱ
    constexpr static int HEART_TIME = 5000;  // �������
public:
    typedef enum {
        ST_NULL = 0,
        ST_CONNECTING,
        ST_CONNECTED,
        ST_DISCONNECT,
    } STATUS;

public:
    Client();
    Client(int fd);
    ~Client();

    virtual int  init()   = 0;
    virtual void onTick() = 0;
    virtual bool isConn();
    virtual bool isAuthentication();
    virtual bool isTimeout(int out_time = 0);

    virtual int sendData(const char *data, int len);
    virtual int sendData(const uchar *data, int len);

protected:
    // �հ�������0����ѭ�����գ�!0�жϱ��ν���
    virtual int onRecvData() = 0;

    virtual int  handleEvent(int fd, int events, void *data);
    virtual int  readData();
    virtual void onStatusChange();

    // �Զ�����Ϣ����
    virtual int checkEvents();
    virtual int handleEvents();
    virtual int getRecvSpace();

    void initData();
    void setFd(int fd);
    void closeFd();

protected:
    int      mFd;
    int      mStatus;
    SockBuf *mRSBuf;       // ���ջ�����
    bool     mIsAuth;      // �Ƿ��½
    int64_t  mLastRcvTime; // ����հ�ʱ��
    int64_t  mLastSndTime; // ��󷢰�ʱ��
};

#endif
