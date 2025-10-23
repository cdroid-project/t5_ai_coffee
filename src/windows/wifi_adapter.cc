/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:13:22
 * @FilePath: /t5_ai_coffee/src/windows/wifi_adapter.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#include <wifi_adapter.h>
#include "wind_mgr.h"
#include "project.h"
#include "data.h"
#include "conf_mgr.h"

#ifndef TUYA_OS_DISABLE
#include "tuya_os_mgr.h"
#endif

std::vector<WIFIAdapterData> sWifiData;
WIFIConnectData connWifiData;

/////////////// WifiItemDecoration //////////////////////
WifiItemDecoration::WifiItemDecoration(int firItemSpac, int betweenItemBottom, int dividerLeft, int dividerRight, int dividerH,int dividerCol)
    :firstItemSpacing(firItemSpac),betweenItemBottomSpacing(betweenItemBottom),
    dividerLeftSpacing(dividerLeft),dividerRightSpacing(dividerRight),dividerHeight(dividerH),dividerColor(dividerCol){

    isHeaderFlag = false;
}

void WifiItemDecoration::getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state){
    int position = parent.getChildAdapterPosition(&view);
    int itemCount = parent.getAdapter()->getItemCount();

    if(isHeaderFlag && (position == 0)){
        outRect.top = firstItemSpacing;
        outRect.height = firstItemSpacing;
    }else if(position == itemCount-1){
        outRect.height = betweenItemBottomSpacing;
    }else{
        outRect.height = 0;
    }
}

void WifiItemDecoration::onDraw(Canvas& c,RecyclerView& parent,RecyclerView::State& state){

    int left = parent.getPaddingLeft();
    int right = parent.getWidth() - parent.getPaddingRight();
    int childCount = parent.getChildCount();
    for (int i = 0; i < childCount-1; i++) {
        View *child = parent.getChildAt(i);
        RecyclerView::LayoutParams *params = (RecyclerView::LayoutParams *)child->getLayoutParams();
        // divider的top 应该是 item的bottom 加上 marginBottom 再加上 Y方向上的位移
        int top = child->getBottom() + params->bottomMargin + round(child->getTranslationY());
        // divider的bottom就是top加上divider的高度了
        int bottom = (int) (top + dividerHeight);
        c.set_line_width(dividerHeight);
        c.set_color(dividerColor);
        c.move_to(left+dividerLeftSpacing,top);
        c.line_to(right-dividerRightSpacing,top);
        c.stroke();

        // c.move_to(left,top+5);
        // c.arc(left+5,top+5,5,180*degrees,270*degrees);
        // c.arc(right-5,top+5,5,270*degrees,0);
        // c.arc(right-5,bottom-5,5,0,90*degrees);
        // c.arc(left+5,bottom-5,5,90*degrees,180*degrees);
    }
}

void WifiItemDecoration::setHeaderType(bool headerFlag){
    isHeaderFlag = headerFlag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
wifiRecycAdapter::wifiRecycAdapter(ViewGroup *parent,RecyclerView *recyclerView):mParentView(parent),mRecyclerView(recyclerView){
    mWifi = sWifiData;
    mIsHeaderView = false;
    mConnectViewClickCallback = nullptr;
    // if(g_appData.netSwitch) g_wifiMgr->scanWifi();
}


RecyclerView::ViewHolder* wifiRecycAdapter::onCreateViewHolder(ViewGroup* parent, int viewType){
    ViewGroup *convertView = (ViewGroup *)LayoutInflater::from(parent->getContext())->inflate("@layout/wifi_card_layout", nullptr);
    return new wifiRecycAdapter::ViewHolder(convertView);
}
void wifiRecycAdapter::onBindViewHolder(RecyclerView::ViewHolder& holder, int position){
    // 设置背景图
    ViewGroup *viewGroup = ((wifiRecycAdapter::ViewHolder&)holder).viewGroup;
    LOGD("position = %d  viewGroup = %p",position,viewGroup);

    ImageView *wifiSignalImg = (ImageView*)viewGroup->findViewById(t5_ai_coffee::R::id::wifi_signal_img);
    ImageView *wifiLockImg = (ImageView*)viewGroup->findViewById(t5_ai_coffee::R::id::wifi_lock_img);
    TextView *wifiName = (TextView*)viewGroup->findViewById(t5_ai_coffee::R::id::wifi_name_tv);
    ImageView *wifiHookImg = (ImageView*)viewGroup->findViewById(t5_ai_coffee::R::id::wifi_hook_img);
    TextView *wifiListTitleTv = (TextView*)viewGroup->findViewById(t5_ai_coffee::R::id::wifi_list_title_tv);
    
    Runnable &connRun = ((wifiRecycAdapter::ViewHolder&)holder).connectingRun;
    connRun = [this,wifiSignalImg,&connRun,viewGroup](){
        wifiSignalImg->setImageLevel((wifiSignalImg->getDrawable()->getLevel())%4 + 1);
        mParentView->postDelayed(connRun,1000);
    };

    ((BitmapDrawable *)wifiHookImg->getDrawable())->setFilterBitmap(true);
    wifiHookImg->setVisibility(View::INVISIBLE);
    if((!mIsHeaderView && (position == 0)) || (mIsHeaderView && (position == 1))) 
            wifiListTitleTv->setVisibility(View::VISIBLE);
    else    wifiListTitleTv->setVisibility(View::GONE);

    if(mIsHeaderView && position == 0){
        if(g_wifiMgr->connStatus() == WIFI_CONNECTING){
            
            wifiSignalImg->setImageLevel(4);
            mParentView->postDelayed(connRun,1000);
            LOGE("viewGroup = %p  connRun = %p" ,viewGroup,&connRun);

            wifiName->setText(connWifiData.name);
            wifiLockImg->setVisibility(connWifiData.locked? View::VISIBLE: View::GONE);
        }else{
            // wifiSignalImg->setActivated(true);
            int level = g_appData.netStatus - PRO_STATE_NET_NONE;
            wifiSignalImg->setImageLevel(level<=0?1:(level>4?4:level));
            LOGI("network level = %d",level<=0?5:(level>4?4:level));

            wifiName->setText(g_objConf->getWifiName());
            wifiLockImg->setVisibility(connWifiData.locked? View::VISIBLE: View::GONE);
            wifiHookImg->setVisibility(View::VISIBLE);
        }
        viewGroup->setOnTouchListener([this](View&v, MotionEvent&event){
            if(mConnectViewClickCallback) mConnectViewClickCallback(event.getAction());
            switch (event.getAction()) {
                case MotionEvent::ACTION_DOWN:
                    v.setPressed(true);
                    return true;
                case MotionEvent::ACTION_UP:
                case MotionEvent::ACTION_CANCEL:
                    v.setPressed(false);
                    return true;
            }
            return true;
        });
    }else{
        WIFIAdapterData pdat;
        if(mIsHeaderView)   pdat = mWifi.at(position-1);
        else                pdat = mWifi.at(position);

        wifiName->setText(pdat.name); // 网络名

        // 信号图标
        wifiSignalImg->setImageLevel(pdat.level<=0?5:(pdat.level>4?4:pdat.level));

        // 密码锁
        wifiLockImg->setVisibility(pdat.locked? View::VISIBLE: View::GONE);

        viewGroup->setOnClickListener([this,pdat,position](View& v) { 
            LOGI("pdat.locked = %d pdat.name = %s position = %d",pdat.locked,pdat.name.c_str(),position); 
            g_windMgr->showKeyBoardPage("", "请输入 "+pdat.name+" 密码:",-1,
                [this,pdat](std::string inputData){
                    connWifiData.locked = pdat.locked;
                    connWifiData.name = pdat.name;
                    connWifiData.key = inputData;
                    for(auto it = sWifiData.begin(); it != sWifiData.end(); it++){
                        if(it->name == pdat.name){
                            g_appData.netStatus = it->level + PRO_STATE_NET_NONE;
                            sWifiData.erase(it);
                            break;
                        }
                    }
                    g_wifiMgr->delayConnWifi(connWifiData,1);
                },nullptr);
        });
    }
}
int wifiRecycAdapter::getItemCount(){
    LOGV("mWifi.size() = %d   mIsHeaderView = %d",mWifi.size(),mIsHeaderView);
    if(mIsHeaderView)   return mWifi.size()+1;
    else                return mWifi.size();
}

int wifiRecycAdapter::getItemViewType(int position){
    return (position == 0 && mIsHeaderView) ? VIEW_TYPE_HEADER : VIEW_TYPE_ITEM;
}

void wifiRecycAdapter::notifyData(){
    LOGE("wifiRecycAdapter::notifyData");
    mWifi = sWifiData;
    notifyDataSetChanged();
}

void wifiRecycAdapter::setHeaderView(bool isHeaderView){
    LOGE("isHeaderView = %d",isHeaderView);
    mIsHeaderView = isHeaderView;
}

void wifiRecycAdapter::setConnectViewClickCallback(connClickCallback callback){
    mConnectViewClickCallback = callback;
}

// void wifiRecycAdapter::onItemRangeChanged(int positionStart, int itemCount, Object* payload){
//     LOGE("positionStart = %d   itemCount = %d",positionStart,itemCount);
// }

void wifiRecycAdapter::onViewRecycled(RecyclerView::ViewHolder& holder){
    // ViewGroup *viewGroup = ((wifiRecycAdapter::ViewHolder&)holder).viewGroup;
    Runnable &connRun = ((wifiRecycAdapter::ViewHolder&)holder).connectingRun;
    mParentView->removeCallbacks(connRun);
}

void wifiRecycAdapter::onViewAttachedToWindow(RecyclerView::ViewHolder& holder){
    LOGV("onViewAttachedToWindow   %d",holder.getLayoutPosition());
    View *wifiName = ((wifiRecycAdapter::ViewHolder&)holder).viewGroup->findViewById(t5_ai_coffee::R::id::wifi_name_tv);
    wifiName->setSelected(false);
    wifiName->setSelected(true);
}

void wifiRecycAdapter::onViewDetachedFromWindow(RecyclerView::ViewHolder& holder){
    LOGV("onViewDetachedFromWindow   %d",holder.getLayoutPosition());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WIFIScanThread::onTask(void* data){
    std::vector<WIFIAdapterData> *dat = (std::vector<WIFIAdapterData> *)data;
    dat->clear();
    std::list<WifiSta::WIFI_ITEM_S> wifiList;
    WifiSta::ins()->scan(wifiList);
    std::string wifiName, wifiPasswd;
    g_objConf->getWifiInfo(wifiName, wifiPasswd); 

    for (WifiSta::WIFI_ITEM_S &item : wifiList) {
        if(item.signal <=0 ) continue;
        WIFIAdapterData wf_data;
        wf_data.locked      = item.isKey;
        wf_data.level       = item.signal;
        wf_data.conn_status = (wifiName == item.ssid) ? WIFI_CONNECTED : WIFI_DISCONNECTED;
        wf_data.name        = item.ssid;
        wf_data.quality     = item.quality;
        wf_data.signalLevel = item.signalLevel;
        if(((wf_data.name == wifiName) || (wf_data.name == connWifiData.name ))&& ((!g_appData.netOk && g_wifiMgr->connStatus() == WIFI_CONNECTING) || g_appData.netOk )){
            g_appData.netStatus = PRO_STATE_NET_NONE + wf_data.level;
            LOGI_IF(wifiName == wf_data.name," /*   g_appData.netStatus  */ ");
        } else if((wf_data.name == wifiName) && (g_wifiMgr->connStatus() == WIFI_DISCONNECTED) && !g_appData.netOk){
            connWifiData.name = wifiName;
            connWifiData.key = wifiPasswd;
            g_appData.netStatus = PRO_STATE_NET_NONE + wf_data.level;
            LOGE("/*  connWifiData.name = %s  connWifiData.key = %s  */ ",connWifiData.name.c_str(),connWifiData.key.c_str());
            g_wifiMgr->delayConnWifi(connWifiData,1000);
        }else{
            dat->push_back(wf_data);
            LOGI_IF(wifiName == wf_data.name," /*  dat->push_back(wf_data)  */");
        }
            
    }
    return 0;
}

void WIFIScanThread::onMain(void *data) {
    static int count = 0;
    if(g_appData.netSwitch){
        sWifiData = *(std::vector<WIFIAdapterData> *)data;
        g_appData.statusChangeFlag |= CS_WIFI_ADAPTER_NOTIFI;
        g_windMgr->updateDate();
        g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WIFI);
        LOGE("scan wifi complet  sWifiData.size() = %d",sWifiData.size());
    }else{
        sWifiData.clear();
        LOGE("scan wifi fail!");
    }
    g_wifiMgr->scanEnd();
}

int WIFIConnectThread::onTask(void* data){
    WIFIConnectData *dat = (WIFIConnectData *)data; 
    WifiSta::ins()->connect(dat->name,dat->key);
    return 0;
}

void WIFIConnectThread::onMain(void *data) {
    WIFIConnectData *dat = (WIFIConnectData *)data;
    connWifiData = {};
    if(g_appData.netSwitch){
        LOGE("connect wifi complet WifiSta::ins()->get_status() = %d",WifiSta::ins()->get_status());
        if(WifiSta::ins()->get_status() == WifiSta::E_CONNECT_SUCCESS){
            g_appData.netOk = true;
            g_objConf->setWifiInfo(dat->name,dat->key);
            for(auto it = sWifiData.begin(); it != sWifiData.end(); it++){
                if(it->name == dat->name){
                    g_appData.netStatus = it->level + PRO_STATE_NET_NONE;
                    sWifiData.erase(it);
                    break;
                }
            }
            g_wifiMgr->setConnStatus(WIFI_CONNECTED); 
            g_appData.statusChangeFlag |= CS_WIFI_CONNECT;
            g_appData.statusChangeFlag |= CS_NETWORK_CHANGE;
            g_windMgr->updateDate();
        }else{
            g_appData.netStatus = PRO_STATE_NET_NONE;
            g_wifiMgr->setConnStatus(WIFI_DISCONNECTED);
            g_wifiMgr->scanWifi();
        }
        
#ifndef TUYA_OS_DISABLE
        if(g_appData.netOk){
            std::string authUuid;
            std::string authKey;
            g_objConf->getTuyaAuthCode(authUuid,authKey);
            if(authUuid == "none" && authKey == "none" || authUuid.empty() && authKey.empty()){
                g_objConf->getFromServerAuth();
                g_windMgr->showPopPage(POP_GET_AUTH,nullptr);
            }else{
                g_tuyaOsMgr->init();
            }  
        }
#endif 
    }else{
        LOGE("g_appData.netSwitch is close !");
        g_appData.netStatus = PRO_STATE_NET_NONE;
        WifiSta::ins()->disEnableWifi();
    }
    g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WIFI);
}


WIFIMgr::WIFIMgr() {
}

void WIFIMgr::init(){
    mIsStartScan = false;
    mIsConnStatus = WIFI_DISCONNECTED;
    // HV_POPEN("ifconfig wlan0 up");
}
WIFIMgr::~WIFIMgr() {

}

void WIFIMgr::handleMessage(Message& message){
    // if(message.what == MSG_DELAY_CONN){
    //     if(g_appData.netSwitch){
    //         mIsConnStatus = WIFI_CONNECTING;
    //         WifiSta::ins()->disconnect();
    //         g_windMgr->changeTitleBar(PageBase::TITLE_BAR_WIFI);
    //         g_appData.statusChangeFlag |= CS_WIFI_CONNECT;
    //         g_windMgr->updateDate();
    //         ThreadPool::ins()->add(new WIFIConnectThread, &mConnData,true);
    //     }
    // }else if(message.what == MSG_DELAY_DISCONN){
    //     if(mIsConnStatus == WIFI_CONNECTING && WifiSta::ins()->get_status() == WifiSta::E_STA_CONENCTING) disConnWifi();
    // }
}

void WIFIMgr::scanWifi(){
    if(g_appData.netSwitch && !mIsStartScan){
        // system("ifconfig wlan0 up");
        mIsStartScan = true;
        // ThreadPool::ins()->add(new WIFIScanThread, &sWifiScanData,true);
    }
}
void WIFIMgr::delayConnWifi(WIFIConnectData connWifiData,int time){
    if(g_appData.netSwitch){
        mConnData = connWifiData;
        Message mesg,mesg1;
        mesg.what = MSG_DELAY_CONN;
        mesg1.what = MSG_DELAY_DISCONN;

        Looper::getMainLooper()->removeMessages(this);
        Looper::getMainLooper()->sendMessageDelayed(time+30*1000,this,mesg1);
        Looper::getMainLooper()->sendMessageDelayed(time,this,mesg);
    }
}

void WIFIMgr::disConnWifi(){
    // if(mIsConnStatus != WIFI_CONNECTING){ g_objConf->setWifiInfo("",""); }
    // connWifiData = {};
    // g_appData.netOk = false;
    // g_appData.netStatus = PRO_STATE_NET_NONE;
    // mIsConnStatus = WIFI_DISCONNECTED;
    // WifiSta::ins()->disconnect();
    // scanWifi();
    // g_appData.statusChangeFlag |= CS_WIFI_CONNECT;
    // g_windMgr->updateDate();
}

void WIFIMgr::scanEnd(){
    mIsStartScan = false;
}

void WIFIMgr::setConnStatus(int status){
    mIsConnStatus = status;
}
int  WIFIMgr::connStatus(){
    return mIsConnStatus;
}

void WIFIMgr::setWifiEnable(bool isEnable){
    // if(isEnable){
    //     WifiSta::ins()->enableWifi();
    // }else{
    //     WifiSta::ins()->disEnableWifi();
    //     WifiSta::ins()->disconnect();
    //     mIsConnStatus = WIFI_DISCONNECTED;
    // }
}