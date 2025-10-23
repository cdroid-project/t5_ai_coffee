/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:46:37
 * @FilePath: /t5_ai_demo/src/net/curl_manager.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include <common.h>
#include <comm_func.h>


#include <core/app.h>
#include <unistd.h>
#include <thread>
#include <sys/prctl.h>

#include "curl_manager.h"

#define CONNECT_TIME 5   // 连接超时：秒
#define COMPLETE_TIME 10 // 完成时间：秒
#define HANDLE_COUNT 5   // 一帧处理个数
#define CURL_REUSE 1     // curl复用

typedef enum {
    CD_CS_NULL = 0,
    CD_CS_INIT,
    CD_CS_ERROR,
    CD_CS_RECV,
    CD_CS_REUSE,
    CD_CS_OVER,
} CURL_STATUS;

#pragma pack(1)
typedef struct tagCURLMData {
    CURL         *curl;
    int           status;
    int           connLen;
    int           recvLen;
    int64_t       begTime;
    int64_t       endTime;
    void         *userData;
    CURLCallback *callback;
    long          httpCode;
    bool          fileFlag;
    int           connectTime;
    int           completeTime;
    int           chunked_block_size;
    int           chunked_read_size;
    uchar         in_thread : 1;
    uchar         in_handle : 1;
    std::string   url;
    std::string   head;
    std::string   body;
    tagCURLMData()
        : curl(0), status(CD_CS_NULL), fileFlag(false), recvLen(0), connectTime(CONNECT_TIME),
          completeTime(COMPLETE_TIME), in_thread(0), in_handle(0) {}
} CURLMData;
//函数涉及的变量
typedef struct ThreadSignal_T
{
    BOOL  relativeTimespan; //是否采用相对时间

    pthread_cond_t cond;
    pthread_mutex_t mutex;

    pthread_condattr_t cattr;

} ThreadSignal;
#pragma pack()

static size_t write_handler(char *ptr, size_t size, size_t nmemb, void *userdata);
static int    multi_timer_cb(CURLM *multi, long timeout_ms, void *param);
static int    multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
static size_t recv_header(void *buffer, size_t size, size_t nmemb, void *userdata);

///////////////////////////////////////////////////////////////
CURLManager *g_objCURL = CURLManager::getInstance();

CURLManager *CURLManager::getInstance() {
    static CURLManager ins;
    return &ins;
}

CURLManager::CURLManager() {
    mMultiHandle = 0;
    mThreadRun   = false;
}

CURLManager::~CURLManager() {
    for (CURLMData *curldata : mUsedCurls) {
        curl_multi_remove_handle(mMultiHandle, curldata->curl);
        delete curldata;
    }
    mUsedCurls.clear();

    for (CURLMData *curldata : mFreeCurls) { delete curldata; }
    mFreeCurls.clear();

    App::getInstance().removeEventHandler(this);
    if (mMultiHandle) {
        curl_multi_cleanup(mMultiHandle);
        curl_global_cleanup();
    }
}

int CURLManager::init() {

    curl_global_init(CURL_GLOBAL_ALL);
    mMultiHandle = curl_multi_init();

    App::getInstance().addEventHandler(this);

    curl_version_info_data *version_info = curl_version_info(CURLVERSION_NOW);
    LOGI("version=%s host=%s ssl_version=%s", version_info->version, version_info->host, version_info->ssl_version);

    // 创建线程
    std::thread sub_thread(threadDeal, this);
    sub_thread.detach();

    return 0;
}

int CURLManager::checkEvents() {
    return mUsedCurls.size();
}

int CURLManager::handleEvents() {
    int     count    = 0;
    int64_t time_now = SystemClock::uptimeMillis();    

    for (auto it = mUsedCurls.begin(); it != mUsedCurls.end() && count < HANDLE_COUNT;) {
        CURLMData *pdata = *it;

        // 还在线程中处理
        if (pdata->in_thread) {
            it++;
            continue;        
        }

        if (pdata->status == CD_CS_OVER || pdata->status == CD_CS_ERROR ||
            (pdata->status == CD_CS_INIT && time_now - pdata->begTime >= pdata->completeTime * 1000)) {

            if (pdata->endTime == 0) { pdata->endTime = time_now; }

            LOG(VERBOSE) << "curl over. url=" << pdata->url << " status=" << pdata->status << " use_time=" << (pdata->endTime - pdata->begTime);

            if (pdata->callback) {
                pdata->callback->onHttpResponse(pdata->status == CD_CS_OVER ? 0 : -1, pdata->body, pdata->userData);
            }

            count++;
            cacheCURL(pdata);
            it = mUsedCurls.erase(it);

        } else {
            // 未超时的初始状态，需要添加到线程中处理
            if (pdata->status == CD_CS_INIT && mThreadLock.try_lock()) {
                pdata->in_thread = 1;
                mThreadCURL.push_back(pdata);
                mThreadLock.unlock();
            }

            it++;
        }
    }

    return 0;
}

CURLMData *CURLManager::allocCURL(const std::string &url) {
    CURLMData *curldata = 0;

    if (mFreeCurls.empty()) {
        for (int i = 0; i < HANDLE_COUNT; i++) { mFreeCurls.push_back(new CURLMData()); }
    }

    // 查找host匹配
#if CURL_REUSE
    auto it  = mFreeCurls.begin();
    auto def = mFreeCurls.end();
    for (it = mFreeCurls.begin(); it != mFreeCurls.end(); it++) {
        CURLMData *item = *it;
        if (item->status != CD_CS_REUSE) {
            if (def == mFreeCurls.end()) def = it;
        } else if (compareHost(item->url, url) == 0) {
            curldata = item;
            mFreeCurls.erase(it);
            break;
        }
    }
    if (!curldata) {
        if (def == mFreeCurls.end()) {
            curldata = new CURLMData();
        } else {
            curldata = *def;
            mFreeCurls.erase(def);
        }
    }
#else
    if (!curldata) {
        curldata = mFreeCurls.front();
        mFreeCurls.pop_front();
    }
#endif

    mUsedCurls.push_back(curldata);

    if (!curldata->curl) {
        CURL *curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0); // Content-Length:
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                  // 允许重定向
        curl_easy_setopt(curl, CURLOPT_HEADER, 0L); // 启用时会将头文件的信息作为数据流输

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recv_header); // 写入接受头的回调
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, curldata);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_handler);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, curldata);

        // 打印调试信息
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        // 关闭 ssl 验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        /* enable TCP keep-alive for this transfer */
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);
        /* keep-alive idle time to 120 seconds */
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
        /* interval time between keep-alive probes: 60 seconds */
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

        /* 如果在60秒内低于30字节/秒，则中止 */
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1); // 以下3个为重定向设置
        // 返回的头部中有Location(一般直接请求的url没找到)，则继续请求Location对应的数据
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1); // 查找次数，防止查找太深

        // 连接成功后断网会卡在dns解析，缓存足够长的时间
        curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 30 * DAY_SECONDS);

        curldata->curl = curl;
        mCURLData.insert(std::make_pair(curldata->curl, curldata));
    }

    curldata->status             = CD_CS_INIT;
    curldata->connLen            = -1;
    curldata->begTime            = SystemClock::uptimeMillis();
    curldata->endTime            = 0;
    curldata->userData           = 0;
    curldata->callback           = 0;
    curldata->httpCode           = 0;
    curldata->recvLen            = 0;
    curldata->chunked_block_size = 0;
    curldata->chunked_read_size  = 0;
    curldata->in_thread          = 0;
    curldata->in_handle          = 0;
    curldata->head.clear();
    curldata->body.clear();

    LOG(VERBOSE) << "curl="<< curldata->curl << " begin=" << curldata->begTime;

    return curldata;
}

int CURLManager::compareHost(const std::string &url, const std::string &url2) {
    size_t schemePos = url.find("://");

    if (schemePos == std::string::npos) {
        size_t pathPos = url.find('/');
        if (pathPos == std::string::npos) { return url.compare(url2); }

        return strncmp(url.c_str(), url2.c_str(), pathPos);
    }

    size_t pathPos = url.find('/', schemePos + sizeof("://") - 1);
    if (pathPos == std::string::npos) { return url.compare(url2); }

    return strncmp(url.c_str(), url2.c_str(), pathPos);
}

void CURLManager::cacheCURL(CURLMData *curldata) {
    if (CURL_REUSE && curldata->status == CD_CS_OVER) {
        curldata->status = CD_CS_REUSE;
    } else {
        mCURLData.erase(curldata->curl);
        curl_easy_cleanup(curldata->curl);
        curldata->status = CD_CS_NULL;
        curldata->curl   = 0;
    }
    mFreeCurls.push_back(curldata);
}

int CURLManager::get(const std::string &url, CURLCallback *cb, void *data, int connectTime /* = 5*/,
                     int completeTime /* = 10*/) {
    if (!mMultiHandle || url.empty()) { return -1; }

    CURLMData *curldata    = allocCURL(url);
    curldata->url          = url;
    curldata->callback     = cb;
    curldata->userData     = data;
    curldata->connectTime  = connectTime;
    curldata->completeTime = completeTime;

    curl_easy_setopt(curldata->curl, CURLOPT_URL, curldata->url.c_str());

    // 将请求设置为非阻塞模式
    curl_easy_setopt(curldata->curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curldata->curl, CURLOPT_CONNECTTIMEOUT, curldata->connectTime);
    curl_easy_setopt(curldata->curl, CURLOPT_TIMEOUT, curldata->completeTime);

    LOG(VERBOSE) << "curl get. url=[" << curldata->url << "]";

    return 0;
}

int CURLManager::post(const std::string &url, const std::string &body, CURLCallback *cb, void *data,
                      int connectTime /* = 5*/, int completeTime /* = 10*/) {
    if (!mMultiHandle || url.empty()) { return -1; }

    CURLMData *curldata    = allocCURL(url);
    curldata->url          = url;
    curldata->callback     = cb;
    curldata->userData     = data;
    curldata->connectTime  = connectTime;
    curldata->completeTime = completeTime;

    curl_easy_setopt(curldata->curl, CURLOPT_URL, curldata->url.c_str());

    // 将请求设置为非阻塞模式
    curl_easy_setopt(curldata->curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curldata->curl, CURLOPT_CONNECTTIMEOUT, curldata->connectTime);
    curl_easy_setopt(curldata->curl, CURLOPT_TIMEOUT, curldata->completeTime);

    // libcur的相关POST配置项
    curl_easy_setopt(curldata->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curldata->curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curldata->curl, CURLOPT_POSTFIELDSIZE, body.size());

    LOG(VERBOSE) << "curl post. url=[" << curldata->url << "] len=" << body.size();

    return 0;
}

static size_t recv_header(void *buffer, size_t size, size_t nmemb, void *userdata) {
    LOGV("%s", (char *)buffer);

    CURLMData *curldata = (CURLMData *)userdata;
    curldata->head.append((char *)buffer, size * nmemb);

    if (curldata->connLen == -1) {
        char *lenStr = stristr((char *)buffer, "Content-Length: ");
        if (lenStr) {
            curldata->connLen = atoi(lenStr + sizeof("Content-Length: ") - 1);
            curldata->status  = CD_CS_RECV;
        } else {
            lenStr = stristr((char *)buffer, "Accept-Length: ");
            if (lenStr) {
                curldata->connLen = atoi(lenStr + sizeof("Accept-Length: ") - 1);
                curldata->status  = CD_CS_RECV;
            }
        }
    }

    // 文件标识
    // Accept-Ranges: bytes
    if (stristr((char *)buffer, "Accept-Ranges: bytes")) { curldata->fileFlag = true; }

    return size * nmemb;
}

static size_t write_handler(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t     slen = size * nmemb;
    CURLMData *wd   = (CURLMData *)userdata;

    LOGV("url=%s size=%d nmemb=%d data=%s", wd->url.c_str(), (int)size, (int)nmemb, ptr);

    if (wd->fileFlag && wd->callback) {
        wd->callback->onHttpDownloadFile(wd->connLen, ptr, slen, wd->userData);
    } else {
        wd->body.append(ptr, slen);
    }
    wd->recvLen += slen;

    // 检查接收完成
    if (wd->connLen > 0 && wd->recvLen >= wd->connLen) {
        wd->status = CD_CS_OVER;
        LOG(VERBOSE) << "body end. url=" << wd->url << " rlen=" << wd->recvLen << " clen=" << wd->connLen;
    }

    return slen;
}

/*
 * multi 第一步创建的句柄
 * timeout_ms libcurl库维护的一个超时时间，具体怎么算不清楚，回调时会自动赋值
 * param 第二步设置的参数
 * return 错误码
 */
static int multi_timer_cb(CURLM *multi, long timeout_ms, void *param) {

    return 0;
}

/*
 * e 第三步添加的easy句柄
 * s libcurl创建维护的socket
 * what 执行动作(读或写)
 */
static int multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
    LOG(DEBUG) << "s=" << s << " what=" << what;

    return 0;
}

void CURLManager::threadDeal(void *arg) {
    int64_t last_tick, diff_tick;    
    int     idle_count;

    prctl(PR_SET_NAME, "curl_thread");

    idle_count = 0;
    g_objCURL->mThreadRun = true;

    while (g_objCURL->mThreadRun) {        
        last_tick = SystemClock::uptimeMillis();        
        g_objCURL->onThreadHandleEvent();
        diff_tick = SystemClock::uptimeMillis() - last_tick;
        if (diff_tick < 10) {
            if (++idle_count >= 10) {
                idle_count = 0;
                usleep(10000);
            }
        }
    }
}

void CURLManager::onThreadHandleEvent() {
    int64_t    now_tick, last_tick;
    CURLMcode  mc;
    int        still_running, ret, diff_tick;
    int        i, j;
    CURLMData *curldata;

    if (mThreadCURL.empty()) return;

    // 添加处理处理
    now_tick  = SystemClock::uptimeMillis();
    last_tick = now_tick;
    mThreadLock.lock();
    for (i = 0, j = mThreadCURL.size(); i < j; i++) {
        curldata = mThreadCURL[i];
        if (curldata->in_handle) continue;
        
        // 超时的不再加入处理
        if (now_tick - curldata->begTime >= curldata->completeTime * 1000) {
            continue;
        }

        curl_multi_add_handle(mMultiHandle, curldata->curl);
        curldata->in_handle = 1;
    }
    mThreadLock.unlock();
    diff_tick = (now_tick = SystemClock::uptimeMillis()) - last_tick;
    if (diff_tick > 100) LOGE("curl_multi_add_handle use time %dms", diff_tick);    

    // 开始处理
    now_tick  = SystemClock::uptimeMillis();
    last_tick = now_tick;
    mc        = curl_multi_perform(mMultiHandle, &still_running);
    diff_tick = (now_tick = SystemClock::uptimeMillis()) - last_tick;
    if (diff_tick > 100) LOGE("curl_multi_perform use time %dms", diff_tick);

    // 等待有任务完成的通知，有结果时立刻返回，没有结果时10ms后等待结束返回,ret返回完成的任务数量
    last_tick = now_tick;
    mc        = curl_multi_poll(mMultiHandle, NULL, 0, 10, &ret);
    if (mc == CURLM_OK) {
        // 有任务
        CURLMsg *m;
        int      msgq;

        do {
            m = curl_multi_info_read(mMultiHandle, &msgq);
            if (!m) break;

            if (m->msg != CURLMSG_DONE) {
                LOGV("msg not done");
                continue;
            }

            // 获取请求的状态码
            long http_code;
            curl_easy_getinfo(m->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
            LOGV("Request completed with status code: %ld curl: %p", http_code, m->easy_handle);

            auto it = mCURLData.find(m->easy_handle);
            if (it != mCURLData.end()) {
                CURLMData *curldata = it->second;
                curldata->httpCode  = http_code;
                curldata->status    = (m->data.result != CURLE_OK ? CD_CS_ERROR : CD_CS_OVER);
                curldata->endTime   = SystemClock::uptimeMillis();
                if (m->data.result == CURLE_OK) {
                    LOGV("msg done. result=%d url=%s", m->data.result, curldata->url.c_str());
                } else {
                    LOG(ERROR) << "msg done. result=" << m->data.result << " msg=" << curl_easy_strerror(m->data.result)
                        << " url=" << curldata->url << " begTime=" << curldata->begTime
                        << " use=" << (curldata->endTime - curldata->begTime);
                }
            } else {
                if (m->data.result == CURLE_OK) {
                    LOGE("msg done, but not reg. curl=%p result=%d", m->easy_handle, m->data.result);
                } else {
                    LOGE("msg done, but not reg. curl=%p result=%d msg=%s", m->easy_handle, m->data.result,
                        curl_easy_strerror(m->data.result));
                }
            }

        } while (true);
    }
    diff_tick = (now_tick = SystemClock::uptimeMillis()) - last_tick;
    if (diff_tick > 100) LOGE("curl_multi_poll use time %dms", diff_tick);

    // 处理完成、过期的请求
    int     count    = 0;
    int64_t time_now = SystemClock::uptimeMillis();

    mThreadLock.lock();
    for (auto it = mThreadCURL.begin(); it != mThreadCURL.end() && count < HANDLE_COUNT;) {
        curldata = *it;

        // 结束、错误、超时
        if (curldata->status == CD_CS_OVER || curldata->status == CD_CS_ERROR ||
            time_now - curldata->begTime >= curldata->completeTime * 1000) {
            it = mThreadCURL.erase(it);

            if (curldata->endTime == 0) { curldata->endTime = time_now; }

            count++;            
            curldata->in_thread = 0;
            if (curldata->in_handle) {
                curl_multi_remove_handle(mMultiHandle, curldata->curl);
                curldata->in_handle = 0;
            }            
        } else {
            it++;
        }
    }
    mThreadLock.unlock();
}
