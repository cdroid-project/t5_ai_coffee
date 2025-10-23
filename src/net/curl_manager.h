/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:46:41
 * @FilePath: /t5_ai_demo/src/net/curl_manager.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __curl_manager_h__
#define __curl_manager_h__

#include <curl/curl.h>

#include <list>
#include <string>

class CURLCallback {
public:
    virtual void onHttpResponse(int result, const std::string &body, void *data) = 0;
    virtual void onHttpDownloadFile(int tsize, const char *ptr, int rsize, void *data){}
};

//////////////////////////////////////////////////////////////
struct tagCURLMData;
typedef struct tagCURLMData CURLMData;

class CURLManager : public EventHandler {
public:
    static CURLManager *getInstance();

    int init();
    int get(const std::string &url, CURLCallback *cb, void *data, int connectTime = 5, int completeTime = 10);
    int post(const std::string &url, const std::string &body, CURLCallback *cb, void *data, int connectTime = 5, int completeTime = 10);

    virtual int checkEvents();
    virtual int handleEvents();

protected:
    CURLManager();
    ~CURLManager();
    CURLMData *allocCURL(const std::string &url);
    void       cacheCURL(CURLMData *curldata);
    int        compareHost(const std::string &url, const std::string &url2);

    static void threadDeal(void *arg);
    void   onThreadHandleEvent();

private:
    CURLM *                       mMultiHandle;
    std::list<CURLMData *>        mFreeCurls;
    std::list<CURLMData *>        mUsedCurls;
    std::map<CURL *, CURLMData *> mCURLData;

    bool                          mThreadRun;
    std::mutex                    mThreadLock;
    std::vector<CURLMData *>      mThreadCURL;
};

extern CURLManager *g_objCURL;

#endif
