/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:46:51
 * @FilePath: /t5_ai_demo/src/net/curldownload.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <cdlog.h>
#include <functional>

#include <fstream>
#include <iostream>

typedef std::function<void(int, char*, std::string&)>downloadCallBack;
typedef std::function<void(double,double)> downloadProgressCallBack;

struct SockInfo;
class CurlDownloader {
private:
    CurlDownloader();
    ~CurlDownloader();
public:
    class ConnectionData {
    private:
        std::string url;
        std::string savePath;
        std::fstream* out;
        bool syncWriting;
        char* data;
        double totalTime;
        int nbBytes;
        CURLcode res;
        int httpStatus;
        time_t startTime;
        int stopppedByTimeout;
        downloadCallBack callback;
        downloadProgressCallBack progressCallback;
    public:
        ConnectionData(const std::string& url, const std::string& path = "", downloadCallBack l = nullptr, downloadProgressCallBack pl= nullptr, bool sync = false);
        virtual ~ConnectionData();
        const std::string getUrl()const;
        int getHttpStatus() const;
        int getNbBytes()const;
        int hasElapsed(time_t timeout);
        int isStoppedByTimeout() const;
        void onDataRead(char* input, size_t size);
        void onConnectionComplete(double time, CURLcode r, int status, int stoppedByTimeout);
        void onDownloadProgress(double dlnow, double dltotal);
        void cleanCallBack();
    };
private:
    int mTimerFD;
    CURLM* mMulti;
    int mActiveHandles;
    std::vector<CURL*>mEasys;
    static int SocketCallback(CURL* easy, curl_socket_t s, int action, void* userp, void* socketp);
    static int TimerCallback(int fd, int events, void* data);
    static int MultiTimeCallback(CURLM* multi, long timeout_ms, void* data);
    static int EventHandler(int fd, int events, void* data);
    static size_t WriteHandler(char* ptr, size_t size, size_t nmemb, void* userdata);
    static int ProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    void setsock(SockInfo* f, curl_socket_t s, CURL* e, int act);
    void addsock(curl_socket_t s, CURL* easy, int action);
    void remove_sock(SockInfo* fdp);
    void check_for_timeout();
    void cleanup(int still_running);
    void cleanup_one_connection(ConnectionData* priv, CURL* easy,
        double total_time, CURLcode res, int httpStatus, int stoppedByTimeout);
    CURL* createConnection(ConnectionData* connection);
public:
    static CurlDownloader *ins() {
        static CurlDownloader stIns;
        return &stIns;
    }

    void init();
    void closeAllDownload();
    int addConnection(ConnectionData* connections);
};

extern CurlDownloader *g_objCurl;
