/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:13:17
 * @FilePath: /t5_ai_coffee/src/windows/wifi_adapter.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#ifndef __WIFI_ADAPTER_H_
#define __WIFI_ADAPTER_H_

#include <hv_wifi_sta.h>
#include <common.h>
#include <comm_class.h>
#include <core/handler.h>
#include <widgetEx/recyclerview/recyclerview.h>

// WIFI列表
typedef struct tagWIFIAdapterData {
    uchar       locked : 1;      // 需要密码
    int         quality;         // 质量
    int         signalLevel;     // 信号强度
    int         level;           // 信号强度
    uchar       conn_status : 2; // 连接状态
    std::string name;            // 名称
} WIFIAdapterData;

// WIFI 连接信息
typedef struct {
    uchar       locked : 1;      // 需要密码
    std::string name;            // 名称
    std::string key;
} WIFIConnectData;

enum emWifiStatus {
    WIFI_CONNECTING,        // 连接中
    WIFI_CONNECTED,         // 已连接
    WIFI_DISCONNECTED,      // 未连接
};

// recyclerview 每一项之间的间隔
class WifiItemDecoration : public RecyclerView::ItemDecoration{
private:
    int firstItemSpacing;
    int betweenItemBottomSpacing;

    int dividerLeftSpacing;
    int dividerRightSpacing;
    int dividerHeight;
    int dividerColor;

    bool isHeaderFlag;
public:
    WifiItemDecoration(int firItemSpac, int betweenItemBottom, int dividerLeft, int dividerRight, int dividerH,int dividerCol);

    void getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state) override;
    void onDraw(Canvas& c,RecyclerView& parent,RecyclerView::State& state)override;

    void setHeaderType(bool headerFlag);
};


//     recyclerView  的 适配器
class wifiRecycAdapter : public RecyclerView::Adapter,RecyclerView::AdapterDataObserver{
public:
    typedef std::function<void(int eventAction)> connClickCallback;
private:

    static constexpr int VIEW_TYPE_HEADER   = 0;
    static constexpr int VIEW_TYPE_ITEM     = 1;

    int skipdata = 1;
    bool        mIsHeaderView = false;

    ViewGroup   *mParentView;
    RecyclerView *mRecyclerView;

    Runnable    mWifiConnectingRun;
    connClickCallback mConnectViewClickCallback;
    std::vector<WIFIAdapterData> mWifi;
public:
    class ViewHolder:public RecyclerView::ViewHolder {
    public:
        ViewGroup* viewGroup;
        Runnable  connectingRun;       // 连接中的Runnable
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            viewGroup =(ViewGroup*)itemView;
        }
    };

    wifiRecycAdapter(ViewGroup *parent,RecyclerView *recyclerView);
    ~wifiRecycAdapter(){LOGE("~wifiRecycAdapter()");}

    RecyclerView::ViewHolder*onCreateViewHolder(ViewGroup* parent, int viewType)override;
    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override;
    int getItemCount()override;
    int getItemViewType(int position) override;
    // void onItemRangeChanged(int positionStart, int itemCount, Object* payload)override;
    void onViewRecycled(RecyclerView::ViewHolder& holder)override;
    void onViewAttachedToWindow(RecyclerView::ViewHolder& holder)override;
    void onViewDetachedFromWindow(RecyclerView::ViewHolder& holder)override;

    void notifyData();
    void setHeaderView(bool isHeaderView);
    void setConnectViewClickCallback(connClickCallback callback);
    
};


/////////////////////////////////////////////////////////////////////////////

class WIFIScanThread :public ThreadTask{
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};

class WIFIConnectThread :public ThreadTask{
public:
    const std::string name;
    const std::string key;
private:
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};

#define g_wifiMgr WIFIMgr::ins()
class WIFIMgr:public MessageHandler{
protected:
    enum {
        MSG_DELAY_CONN,         // 延迟 连接wifi
        MSG_DELAY_DISCONN,      // 延迟 断开连接wifi
    };
private:
    WIFIConnectData mConnData;
    std::vector<WIFIAdapterData> sWifiScanData;

    bool    mIsStartScan;
    int     mIsConnStatus;
protected:
    WIFIMgr();
    ~WIFIMgr();

    void handleMessage(Message& message)override;
public:
    static WIFIMgr *ins() {
        static WIFIMgr ins;
        return &ins;
    }
    void init();
    void scanWifi();
    void delayConnWifi(WIFIConnectData connWifiData,int time);
    void disConnWifi();

    void scanEnd();
    void setConnStatus(int status);
    int  connStatus();

    void setWifiEnable(bool isEnable);
};

extern std::vector<WIFIAdapterData> sWifiData;
extern WIFIConnectData connWifiData;
#endif