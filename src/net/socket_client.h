/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:47:28
 * @FilePath: /t5_ai_demo/src/net/socket_client.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __socket_client_h__
#define __socket_client_h__

#include "client.h"

// tcp client
class SocketClient : public Client {
public:
    SocketClient();
    ~SocketClient();

    virtual int  init(const char *ip = 0, ushort port = 0);
    virtual void onTick();

protected:
    virtual int  onRecvData();
    virtual void onStatusChange();

    bool         connectServer();
    bool         onCheckConnecting();

protected:
    bool        mInit;
    std::string mIp;
    ushort      mPort;
    ushort      mRPort;
    int64_t     mLastConnTime;
    int         mSockId;
};

#endif
