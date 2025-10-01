#include "page_setting.h"
#include "wind_mgr.h"
#include "data.h"
#include "conf_mgr.h"
#include "ota.h"
#include "hv_version.h"
#include <core/build.h>
#include "pop_normal.h"
#include "conn_mgr.h"

PageSetting::PageSetting(ViewGroup *wind_page_box)
    :PageBase(wind_page_box,"@layout/page_setting",PAGE_SETTING){

    initPageView();
    initPageData();
}

PageSetting::~PageSetting(){
    delete mLinearLyManager;
    delete mRecyclerAdapter;
    delete mWifishuaxinAnim;
}


void PageSetting::initPageView(){
    

    mWifiSwitch  = (Switch *)mPageLayout->findViewById(t5_ai_coffee::R::id::wifi_switch);
    mWifiSwLayout  = mPageLayout->findViewById(t5_ai_coffee::R::id::wifi_sw_layout);
    mShuaxinIcon= (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::wifi_shuaxin);
    mDisConnText= (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::disconnect_text);

    mAppVersionTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::version_tv);
    mMcuVersionTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::mcu_version_tv);
    mCdroidVersionTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::cdroid_version_tv);
    
    mUpdateTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::update_tv);
    mResetTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::reset_tv);

    mTimeGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::time_group);
    mGrindModeGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::grind_mode_group);
    mHotWaterGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::hot_water_group);
    mBindAppGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::bind_app_group);
    mVoiceGroup = (ViewGroup *)mPageLayout->findViewById(t5_ai_coffee::R::id::voice_group);

    mTimeTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::time_tv);
    mGrindModeTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::grind_mode_tv);
    mHotWaterTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::hot_water_tv);
    mBindAppTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::bind_app_tv);
    mVoiceSwitch = (Switch *)mPageLayout->findViewById(t5_ai_coffee::R::id::voice_switch);

    mFrothTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::froth_tv);
    mGrindTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::grind_tv);
    mBrewTv = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::brew_tv);
    
    // 旋转时有锯齿
    ((BitmapDrawable *)((RotateDrawable *)mShuaxinIcon->getDrawable())->getDrawable())->setFilterBitmap(true);

    auto btn_click_func = std::bind(&PageSetting::btnClickListener, this, std::placeholders::_1);
    mShuaxinIcon->setOnClickListener(btn_click_func);
    mWifiSwLayout->setOnClickListener(btn_click_func);
    mUpdateTv->setOnClickListener(btn_click_func);
    mResetTv->setOnClickListener(btn_click_func);
    mTimeGroup->setOnClickListener(btn_click_func);
    mGrindModeGroup->setOnClickListener(btn_click_func);
    mHotWaterGroup->setOnClickListener(btn_click_func);
    mBindAppGroup->setOnClickListener(btn_click_func);
    mVoiceGroup->setOnClickListener(btn_click_func);
    mPageLayout->findViewById(t5_ai_coffee::R::id::title_back)->setOnClickListener(btn_click_func);
    mPageLayout->findViewById(t5_ai_coffee::R::id::card_3_grind)->setOnClickListener(btn_click_func);
    mPageLayout->findViewById(t5_ai_coffee::R::id::card_3_grind)->setSoundEffectsEnabled(false);

    ///////////// recyclerview，wifi列表 ////////////////////
    mWifiRecyclerView = (RecyclerView *)mPageLayout->findViewById(t5_ai_coffee::R::id::wifi_recycler);

    mLinearLyManager = new LinearLayoutManager(mPageLayout->getContext(),RecyclerView::VERTICAL,false);
    mWifiRecyclerView->setLayoutManager(mLinearLyManager);
    mRecyclerAdapter = new wifiRecycAdapter(mPageLayout,mWifiRecyclerView);
    mRecyclerAdapter->setHeaderView(g_appData.netOk || g_wifiMgr->connStatus() == WIFI_CONNECTING);
    mRecyclerAdapter->setConnectViewClickCallback(std::bind(&PageSetting::connViewClickCallback, this, std::placeholders::_1));
    mWifiRecyclerView->setAdapter(mRecyclerAdapter);

    mWifiRecyclerView->addItemDecoration(new WifiItemDecoration(3,1,50,35,1,0x3cFFFFFF));
    /////////////////////////////////////

    /////////// 调节亮度的小动画 ////////////////////
    ImageView *image_bright = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::image_bright);
    SeekBar *mLightSeekbar = (SeekBar*)mPageLayout->findViewById(t5_ai_coffee::R::id::seekbar_light);

    RotateDrawable *rotate = (RotateDrawable *)image_bright->getDrawable();
    mLightSeekbar->setProgress(g_appData.light);

    rotate->setLevel(7250 + 80*27.5);

    SeekBar::OnSeekBarChangeListener seekbarListener;
    seekbarListener.onProgressChanged = [this,image_bright,rotate](AbsSeekBar& seek, int progress, bool fromuser) {
        LOGV("Light:%d", progress);
        float alpha = float(progress + 100)/200.f;

        rotate->setLevel(7500 + float(progress -10)*2500.0 /90.0); 
        setScreenLight(progress);      
        g_appData.light = progress;
                                      
    };
    mLightSeekbar->setOnSeekBarChangeListener(seekbarListener);
    /////////////////////////////////////

    RotateDrawable *shuaxinDrawable = (RotateDrawable*)mShuaxinIcon->getDrawable();
    mWifishuaxinAnim = ValueAnimator::ofInt({0,10000});
    mWifishuaxinAnim->setDuration(1500);
    mWifishuaxinAnim->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器，使得动画变化逐渐减慢
    mWifishuaxinAnim->setRepeatCount(ValueAnimator::INFINITE);       // 设置动画重复次数，这里设置为无限循环
    mWifishuaxinAnim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this,shuaxinDrawable](ValueAnimator&anim){
        // 获取当前动画的值
        const int t = GET_VARIANT(anim.getAnimatedValue(),int);
        shuaxinDrawable->setLevel(t);
        // LOGV("position = %f",position); 
    }));
    
    mViewAnimListener.onAnimationEnd = [this](Animator& animation, bool isReverse){
        mDisConnText->setVisibility(View::GONE);
    };
    
}

void PageSetting::initPageData(){
    bool isMcuNeedUpdate = !g_appData.mcuUpgradeVer.empty()&&(g_appData.McuVersion != g_appData.mcuUpgradeVer) && !g_appData.displayMode;
    mUpdateTv->setEnabled((g_appData.isNeedUpdate || isMcuNeedUpdate));
    mUpdateTv->setText(g_appData.isNeedUpdate?"更新软件":(isMcuNeedUpdate?"更新MCU":"已是最新版本"));
    LOGE("isMcuNeedUpdate = %d g_appData.isNeedUpdate = %d  getMCUVersion = %s",isMcuNeedUpdate,g_appData.isNeedUpdate,getMCUVersion(g_appData.McuVersion).c_str());
    mCdroidVersionTv->setText("CDROID : V"+BUILD::VERSION::Release);
    char versionBuf[64];
    mMcuVersionTv->setText("xxx版本: " + getMCUVersion(g_appData.McuVersion));
    if(g_appData.isBeta)    mAppVersionTv->setText(HV_BETA_VERSION_STRING(versionBuf));
    else                    mAppVersionTv->setText(HV_FORMAT_VERSION_STRING(versionBuf));

    mVoiceSwitch->setChecked(!g_appData.isMute);
    mWifiSwitch->setChecked(g_appData.netSwitch);
    mTimeTv->setText(getDateTime());

    mGrindModeTv->setText(g_appData.beanGrindMode==BEAN_GRIND_WEI?"称重模式":"计时模式");
    mHotWaterTv->setText(std::to_string(g_appData.hotWaterTemp)+ "℃ "+std::to_string(g_appData.hotWater)+"ml");
    if(!g_appData.netOk){
        mBindAppTv->setText("请先连接WIFI");
    }else{
        if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
            mBindAppTv->setText("已绑定");
        }else{
            mBindAppTv->setText("未绑定");
        }
    }

    mShuaxinIcon->setVisibility(g_appData.netSwitch?View::VISIBLE:View::INVISIBLE);

    mFrothTv->setText(std::to_string(g_objConf->getFrothTotal())+"次");
    mGrindTv->setText(std::to_string(g_objConf->getGrindTotal())+"次");
    mBrewTv->setText(std::to_string(g_objConf->getEspTotal()+g_objConf->getAmericanoTotal()+g_objConf->getMasEspTotal()+g_objConf->getHandWashTotal()+g_objConf->getTeaTotal()+g_objConf->getHotWaterTotal())+"次");
    if(g_appData.eqStatus != ES_RESET) checkUpdata();
}

void PageSetting::updatePageData(){
    if(g_appData.statusChangeFlag & CS_WIFI_ADAPTER_NOTIFI){
        if(g_appData.netSwitch) mRecyclerAdapter->notifyData();
        else                    sWifiData.clear();
        mWifishuaxinAnim->end();
    }

    if(g_appData.statusChangeFlag & CS_WIFI_CONNECT){
        LOGD("wifi Connect status change");
        mRecyclerAdapter->setHeaderView(g_appData.netOk || g_wifiMgr->connStatus() == WIFI_CONNECTING);
        mRecyclerAdapter->notifyData();
        mWifiRecyclerView->scrollToPosition(0);
        LOGD("wifi Connect status change  end ");
    }
    
    if(g_appData.statusChangeFlag & CS_CHECK_OTA){
        char versionBuf[64];
        bool isMcuNeedUpdate = !g_appData.mcuUpgradeVer.empty()&&(g_appData.McuVersion != g_appData.mcuUpgradeVer) && !g_appData.displayMode;
        mUpdateTv->setEnabled((g_appData.isNeedUpdate || isMcuNeedUpdate));
        mUpdateTv->setText(g_appData.isNeedUpdate?"更新软件":(isMcuNeedUpdate?"更新MCU":"已是最新版本"));
        if(g_appData.isBeta)    mAppVersionTv->setText(HV_BETA_VERSION_STRING(versionBuf));
        else                    mAppVersionTv->setText(HV_FORMAT_VERSION_STRING(versionBuf));
    }

    if(g_appData.statusChangeFlag & CS_NETWORK_CHANGE){
        if(!g_appData.netOk){
            mUpdateTv->setEnabled(false);
            mUpdateTv->setText("已是最新版本");
            mBindAppTv->setText("请先连接WIFI");
        }else{
            if(g_appData.eqStatus != ES_RESET) checkUpdata();
            if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
                mBindAppTv->setText("已绑定");
            }else{
                mBindAppTv->setText("未绑定");
            }
        }
    }

    if(g_appData.statusChangeFlag & CS_TUYA_NET_CHANGE){
        if(!g_appData.netOk){
            mBindAppTv->setText("请先连接wifi");
        }else if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
            mBindAppTv->setText("已绑定");
        }else {
            mBindAppTv->setText("未绑定");
        }
    }

    if(g_appData.statusChangeFlag & CS_SETPAGE_SYNC_TIME){
        mTimeTv->setText(getDateTime());
    }
}

void PageSetting::stopPageAction(){
    mPageLayout->setVisibility(View::GONE);
    Looper::getMainLooper()->removeMessages(this);
    mWifishuaxinAnim->cancel();
    mDisConnText->animate().cancel();
}

void PageSetting::resetPageAction(){
    mPageLayout->setVisibility(View::VISIBLE);
    changeTitleBarStatus(0xFF);

    initPageData();
    g_appData.statusChangeFlag |=  (CS_WIFI_ADAPTER_NOTIFI | CS_WIFI_CONNECT);
    updatePageData();
    g_appData.statusChangeFlag &= ~(CS_WIFI_ADAPTER_NOTIFI | CS_WIFI_CONNECT);

    Message mesg;
    mesg.what = MSG_DELAY_SCAN;
    Looper::getMainLooper()->removeMessages(this,MSG_DELAY_SCAN);
    Looper::getMainLooper()->sendMessageDelayed(1*1000,this,mesg);
}

void PageSetting::btnClickListener(View& view){
    switch(view.getId()){
        case t5_ai_coffee::R::id::title_back:{
            g_windMgr->showPrevPage();
            break;
        }case t5_ai_coffee::R::id::wifi_shuaxin:{
            mWifishuaxinAnim->start();
            WIFIMgr::ins()->scanWifi();
            break;
        }case t5_ai_coffee::R::id::wifi_sw_layout:{
            if(!mWifiSwitch->isChecked()){
                g_appData.netSwitch = true;
                mShuaxinIcon->setVisibility(View::VISIBLE);
                // WifiSta::ins()->resetWifi();
                WIFIMgr::ins()->setWifiEnable(true);
                WIFIMgr::ins()->scanWifi();
                mWifishuaxinAnim->start();
                g_objConf->getWifiInfo(connWifiData.name,connWifiData.key);
                if(!connWifiData.name.empty()){
                    WIFIMgr::ins()->delayConnWifi(connWifiData,2000);
                }
                
            }else{
                Looper::getMainLooper()->removeMessages(this,MSG_DELAY_SCAN);
                // system("ifconfig wlan0 down");
                WIFIMgr::ins()->setWifiEnable(false);
                g_appData.netSwitch = false;
                g_appData.netOk = false;
                mRecyclerAdapter->setHeaderView(false);
                mShuaxinIcon->setVisibility(View::INVISIBLE);
                sWifiData.clear();
                g_appData.netStatus = PRO_STATE_NET_NONE;
                changeTitleBarStatus(TITLE_BAR_WIFI);

                mRecyclerAdapter->notifyData();
            }
            mWifiSwitch->setChecked(!mWifiSwitch->isChecked());
            break;
        }case t5_ai_coffee::R::id::update_tv:{
            if(g_appData.isNeedUpdate || (!g_appData.mcuUpgradeVer.empty()&&(g_appData.McuVersion != g_appData.mcuUpgradeVer))){
                g_windMgr->showPopPage(POP_OTA,mPageLayout);
            }
            break;
        }case t5_ai_coffee::R::id::reset_tv:{
            g_windMgr->showPopPage(POP_RESTORE_FACTORY,mPageLayout);
            break;
        }case t5_ai_coffee::R::id::time_group:{
            if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
                showToast("已经同步网络时间");
            }else{
                g_windMgr->showPopPage(POP_SET_TIME,mPageLayout);
            }
            break;
        }case t5_ai_coffee::R::id::grind_mode_group:{
            g_windMgr->showPopPage(POP_BEAN_GRIND,mPageLayout,[this](){
                mGrindModeTv->setText(g_appData.beanGrindMode==BEAN_GRIND_WEI?"称豆模式":"计时模式");
                g_objConnMgr->setDeviceData();
            });
            break;
        }case t5_ai_coffee::R::id::hot_water_group:{
            g_windMgr->showPopPage(POP_HOT_WATER,mPageLayout,[this](){
                mHotWaterTv->setText(std::to_string(g_appData.hotWaterTemp)+ "℃ "+std::to_string(g_appData.hotWater)+"ml");
            });
            break;
        }case t5_ai_coffee::R::id::bind_app_group:{
            if(!g_appData.netOk){
                showToast("请先连接WIFI");
            }else if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
                g_windMgr->showPopPage(POP_UNBIND_APP,mPageLayout);
            }else{
                g_windMgr->showPopPage(POP_BIND_APP,mPageLayout);
            }
            
            break;
        }case t5_ai_coffee::R::id::voice_group:{
            g_appData.isMute = !g_appData.isMute;
            mVoiceSwitch->setChecked(!g_appData.isMute);
            g_objConf->setMuteFlag(g_appData.isMute);
            g_objConnMgr->setDeviceData();
            break;
        }case t5_ai_coffee::R::id::card_3_grind:{
            static int clickCount = 0;
            if(SystemClock::uptimeMillis() <= 40*1000){
                clickCount++;
                if(clickCount >= 5){
                    clickCount = 0;
                    g_windMgr->showPage(PAGE_SCALE_CALIBRATION,false);
                }
            }
            break;
        }
    }
}


void PageSetting::connViewClickCallback(int eventAction){
    if(eventAction == MotionEvent::ACTION_DOWN){
        Message mesg;
        mesg.what = MSG_DELAY_DISCONN;
        Looper::getMainLooper()->removeMessages(this,MSG_DELAY_DISCONN);
        Looper::getMainLooper()->sendMessageDelayed(2*1000,this,mesg);
    }else if(eventAction == MotionEvent::ACTION_UP || eventAction == MotionEvent::ACTION_CANCEL){
        Looper::getMainLooper()->removeMessages(this,MSG_DELAY_DISCONN);
    }
}

void PageSetting::onTicker(){
    if(mTimeTv->getText() != getDateTime()){
        mTimeTv->setText(getDateTime());
        LOGI("update time : getDateTime = %s",getDateTime().c_str());
    }
}

void PageSetting::handleMessage(Message& message){
    switch(message.what){
        case MSG_DELAY_DISCONN:{
            LOGE("disConnWifi");
            g_appData.netOk = false;
            mRecyclerAdapter->setHeaderView(false);
            g_wifiMgr->setWifiEnable(false);
            g_wifiMgr->disConnWifi();
            mDisConnText->setVisibility(View::VISIBLE);
            mDisConnText->setAlpha(1.0f);
            mDisConnText->animate().alpha(0.01f).setDuration(2000).setListener(mViewAnimListener).start();
            mBindAppTv->setText("请先连接WIFI");
            break;
        }case MSG_DELAY_SCAN:{
            g_wifiMgr->scanWifi();
            break;
        }
    }
    
}