#include "pop_normal.h"
#include "wind_mgr.h"
#include "conn_mgr.h"
#include "ota.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"
#include "tuya_os_mgr.h"

PopNormal::PopNormal(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb)
    :PopBase(wind_pop_box,"@layout/pop_normal",popType,enterCb,cancelCb){

    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));

    initPopView();
    initPopData();
}

PopNormal::~PopNormal(){
    if(mPopType == POP_OTA){
        mPopLayout->removeCallbacks(mConnectRun);
    }
    mPopLayout->removeCallbacks(mCloseRun);
}

void PopNormal::initPopView(){
    mPopGroup       = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_group);

    mNormalGroup    = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_normal_group);
    mOTAGroup       = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_ota_group);
    mBindAppGroup   = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_bind_app_group);

    mPopInfoGroup   = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_info_group);
    mOTAInfoScrollGroup = (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_ota_info_scroll_group);

    mPopBtnConfirm  = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_cancel);
    
    mPopImg         = (ImageView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_img);
    mPopTitleTv     = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_title_tv);
    mPopInfoTv      = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_info_tv);
    mPopOtaScrollInfoTv = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_ota_info_tv);

    mPopOtaProgress = (ProgressBar *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_ota_progress);
    mPopOtaTitleTv  = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_ota_scroll_title_tv);
    mPopOtaInfoTv   = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::ota_info_tv);
    mPopOtaProgressTv = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::ota_progress_tv);
    
    mBindAppImgGroup= (ViewGroup *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_bind_app_img_group);
    mBindAppImg     = (QRCodeView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_bind_app_img);
    mBindAppCilckTipsTv = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_click_tips_tv);
    mBindAppInfoTv  = (TextView *)mPopLayout->findViewById(kaidu_ms7_lqy::R::id::pop_bind_app_info_tv);

    auto clickFunc = std::bind(&PopNormal::onBtnClickListener,this,std::placeholders::_1);
    mPopLayout->setSoundEffectsEnabled(false);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);

    mBindAppImg->setOnClickListener(clickFunc);
    mBindAppCilckTipsTv->setOnClickListener(clickFunc);
}

void PopNormal::initPopData(){
    mNormalGroup->setVisibility(View::GONE);
    mOTAGroup->setVisibility(View::GONE);
    mBindAppGroup->setVisibility(View::GONE);

    mOTAInfoScrollGroup->setVisibility(View::GONE);
    mPopInfoGroup->setVisibility(View::VISIBLE);
    mPopTitleTv->setVisibility(View::VISIBLE);
    mPopImg->setVisibility(View::GONE);
    mPopBtnConfirm->setVisibility(View::VISIBLE);
    mPopBtnCancel->setVisibility(View::VISIBLE);
    mPopBtnConfirm->setText("确定");
    mPopBtnCancel->setText("取消");
    mPopGroup->getLayoutParams()->width = 880;
    mCloseRun = [](){
        g_windMgr->dealClosePopPage();
    };
    if(mPopType == POP_OTA){
        mNormalGroup->setVisibility(View::VISIBLE);
        mOTAInfoScrollGroup->setVisibility(View::VISIBLE);
        mPopInfoGroup->setVisibility(View::GONE);

        if(g_appData.isNeedUpdate){
            mPopOtaTitleTv->setText("软件更新");
            mPopOtaScrollInfoTv->setText("V"+g_appData.otaVersion + "\n" + g_appData.otaVersionDes);
        }else{
            mPopOtaTitleTv->setText("MCU更新");
            mPopOtaScrollInfoTv->setText("V"+getMCUVersion(g_appData.mcuUpgradeVer) + "\n" + g_appData.mcuUpgradeText);
        }
        mCloseRun = [](){
            g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 设备状态
#endif
            g_windMgr->dealClosePopPage();
        };
        
        mConnectRun = [this](){
            static int LastUpDateFlag = 0;
            static int64_t timeoutTick = SystemClock::uptimeMillis();
            if(mUpDateFlag == OTA_FLAG_SUCCESS || mUpDateFlag == OTA_FLAG_MCU_SUCCES){
                if(LastUpDateFlag != mUpDateFlag){
                    if(mUpDateFlag == OTA_FLAG_SUCCESS){
                        mPopOtaProgress->setProgress(100);
                        mPopOtaProgressTv->setText("100%");
                        mPopOtaInfoTv->setText("下载成功，请勿断电!\n稍后系统将进行重启");
                    }else{
                        mPopOtaProgress->setProgress(0);
                        mPopOtaProgressTv->setText("0%");
                        g_objConnMgr->checkMCUOTA();
                        mPopOtaInfoTv->setText("下载成功，请勿断电!\n正在安装MCU，稍后系统将进行重启");
                    }
                    timeoutTick = SystemClock::uptimeMillis();
                }else if(SystemClock::uptimeMillis() - timeoutTick >= 60 *1000){
                    std::string text = std::string("更新超时!");
                    mPopOtaInfoTv->setText(text);
                    mPopLayout->postDelayed(mCloseRun,10*1000);
                    return;
                }else if(mUpDateFlag == OTA_FLAG_MCU_SUCCES){
                    mPopOtaProgress->setProgress(g_appData.mcuOTAProgress,true);
                    mPopOtaProgressTv->setText(std::to_string(g_appData.mcuOTAProgress) + "%");
                }
                mPopLayout->postDelayed(mConnectRun,100);
            }else if(mUpDateFlag == OTA_FLAG_ERROR){
                std::string text = std::string("下载更新包失败!");
                mPopOtaInfoTv->setText(text);
                mPopLayout->postDelayed(mCloseRun,10*1000);
                mPopOtaProgress->setProgress(100);
                // mPopOtaProgressTv->setText("100%");
                mPopOtaProgressTv->setVisibility(View::GONE);
            }else if(mUpDateFlag == OTA_FLAG_UPDATE_ERROR){
                std::string text = std::string("更新失败!");
                mPopOtaInfoTv->setText(text);
                mPopLayout->postDelayed(mCloseRun,10*1000);
                mPopOtaProgress->setProgress(100);
                // mPopOtaProgressTv->setText("100%");
                mPopOtaProgressTv->setVisibility(View::GONE);
            }else if(mUpDateFlag == OTA_FLAG_MD5_ERROR){
                std::string text = std::string("更新包下载受损，请检查网络后重试!");
                mPopOtaInfoTv->setText(text);
                mPopLayout->postDelayed(mCloseRun,10*1000);
                mPopOtaProgress->setProgress(100);
                // mPopOtaProgressTv->setText("100%");
                mPopOtaProgressTv->setVisibility(View::GONE);
            }else{
                mPopOtaProgress->setProgress(mOtaProgress,true);
                LOGV("mOtaProgress = %d",mOtaProgress);
                mPopOtaProgressTv->setText(std::to_string(mOtaProgress) + "%");
                mPopLayout->postDelayed(mConnectRun,100);
            }
            if(LastUpDateFlag != mUpDateFlag) LastUpDateFlag = mUpDateFlag;
            LOGI("ota running  mUpDateFlag = %d",mUpDateFlag);
        };
    }else if(mPopType == POP_BIND_APP){
        mBindAppImgGroup->setVisibility(View::VISIBLE);
        
        mBindAppGroup->setVisibility(View::VISIBLE);
        mBindAppImg->setVisibility(View::VISIBLE);
        mBindAppCilckTipsTv->setVisibility(View::VISIBLE);
#ifdef CDROID_SIGMA
        mBindAppImg->setText(g_appData.tuyaQrcode);
#else   
        mBindAppImg->setText("https://m.smart321.com/AYSMh4xh1x");
#endif 
        mBindAppInfoTv->setText("打开APP（扫一扫）绑定设备");
    }else if(mPopType == POP_UNBIND_APP){
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopTitleTv->setVisibility(View::VISIBLE);
        mPopImg->setVisibility(View::GONE);
        mPopTitleTv->setText("设备绑定");
        mPopInfoTv->setText("若需解绑，请点击解绑按钮进行解绑，或通过手机APP进行设备绑定。");
        mPopInfoTv->setTextSize(22);
        mPopBtnConfirm->setText("解绑");
        mCloseRun = [this](){
#ifdef CDROID_X64
            g_windMgr->dealClosePopPage();
#else
            setScreenLight(0,false);
            system("reboot");
#endif
        };
    }else if(mPopType == POP_RESTORE_FACTORY){
        mPopGroup->getLayoutParams()->width = 1000;
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopImg->setVisibility(View::VISIBLE);
        mPopTitleTv->setText("恢复出厂设置");
        mPopInfoTv->setText("恢复出厂设置，将清除一切数据，包括菜谱数据、收藏数据、历史记录、通知数据、Wi-Fi数据和解除绑定等，将恢复到出厂状态。");
        mCloseRun = [this](){
#ifdef CDROID_X64
            g_windMgr->dealClosePopPage();
#else
            setScreenLight(0,false);
            system("reboot");
#endif
        };
    }else if(mPopType == POP_STOP_CLEAN){
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopTitleTv->setVisibility(View::GONE);
        mPopInfoTv->setText("是否停止清洁？");
        mPopInfoTv->setTextSize(26);
    }else if(mPopType == POP_GET_AUTH){
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopTitleTv->setText("获取授权码");
        mPopInfoTv->setText("正在获取授权码，获取成功后将自动重启，请稍等...");
        mPopInfoTv->setTextSize(26);
        mPopBtnConfirm->setVisibility(View::GONE);
        mPopBtnCancel->setVisibility(View::GONE);
        mCloseRun = [this](){
#ifdef CDROID_X64
            g_windMgr->dealClosePopPage();
#else
            setScreenLight(0,false);
            system("reboot");
#endif
        };
    }else if(mPopType == POP_TYPE_TEA_EXT_TIPS){
        mPopGroup->getLayoutParams()->width = 1000;
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopImg->setVisibility(View::VISIBLE);
        mPopTitleTv->setVisibility(View::GONE);
        mPopInfoTv->setText("已完成洗茶，是否进行冲泡？");
        mPopInfoTv->setTextSize(26);
    }else if(mPopType == POP_TYPE_TEA_REBREW_TIPS){
        mPopGroup->getLayoutParams()->width = 1000;
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopImg->setVisibility(View::VISIBLE);
        mPopTitleTv->setText("再沏一次");
        mPopInfoTv->setText("茶叶可多次萃取，据研究，第一次冲泡与第二次冲泡在口感、味道以及成分上的差异较大，是否再次冲泡？");
    }else if(mPopType == POP_TYPE_TEA_CANCEL_WORK){
        mPopGroup->getLayoutParams()->width = 1000;
        mNormalGroup->setVisibility(View::VISIBLE);
        mPopImg->setVisibility(View::VISIBLE);
        mPopTitleTv->setVisibility(View::GONE);
        mPopInfoTv->setText("测茶工作中，是否取消？");
        mPopInfoTv->setTextSize(26);
        mPopBtnConfirm->setText("取消");
        mPopBtnCancel->setText("继续");
    }
}

void PopNormal::updatePageData(){
    if((g_appData.statusChangeFlag & CS_TUYA_NET_CHANGE)){
        if(mPopType == POP_UNBIND_APP){
            mPopBtnConfirm->setVisibility(View::GONE);
            mPopBtnCancel->setVisibility(View::GONE);
            mPopInfoTv->setText("已断开云端连接，请稍后再试");
            mPopLayout->removeCallbacks(mCloseRun);
            mCloseRun = [this](){
                g_windMgr->dealClosePopPage();
            };
            mPopLayout->postDelayed(mCloseRun,5*1000);
        }else if(mPopType == POP_BIND_APP){
            if(g_appData.tuyaWifiStatus == PRO_STATE_TUYA_GB_CLOUD_CONN){
                mBindAppImgGroup->setVisibility(View::INVISIBLE);
                ((MarginLayoutParams *)mBindAppInfoTv->getLayoutParams())->leftMargin = 280;
                mPopLayout->postDelayed(mCloseRun,5*1000);
                mBindAppInfoTv->setText("配网成功！正在关闭窗口...");
                mBindAppInfoTv->setTextSize(26);
            }else{
                mBindAppImgGroup->setVisibility(View::VISIBLE);
                mPopLayout->removeCallbacks(mCloseRun);
                mBindAppInfoTv->setText("打开APP（扫一扫）绑定设备");
                mBindAppInfoTv->setTextSize(22);
            }
        }
    }

    if(g_appData.statusChangeFlag & CS_GET_AUTH_STATUS){
        if(g_appData.getAuthStatus == PRO_AUTH_SUCCESS){
            mPopLayout->postDelayed(mCloseRun,8*1000);
        }else if(g_appData.getAuthStatus == PRO_AUTH_ERROR){
            mPopInfoTv->setText(g_appData.getAuthError);
            mPopInfoTv->setTextColor(0xc23616);
            mPopLayout->postDelayed(mCloseRun,20*1000);
        }else if(g_appData.getAuthStatus == PRO_AUTH_CURL_ERROR){
            mPopInfoTv->setText(g_appData.getAuthError + "  Curl error! 请重启重新获取");
            mPopInfoTv->setTextColor(0xc23616);
            mPopLayout->postDelayed(mCloseRun,20*1000);
        }else{
            mPopInfoTv->setText("未知错误，请稍后重试");
            mPopInfoTv->setTextColor(0xc23616);
            mPopLayout->postDelayed(mCloseRun,20*1000);
        }
    }

    if(g_appData.statusChangeFlag & CS_CHECK_OTA){
        if((mPopType == POP_OTA) && (mNormalGroup->getVisibility() == View::VISIBLE)){
            if(g_appData.isNeedUpdate){
                mPopOtaTitleTv->setText("软件更新");
                mPopOtaScrollInfoTv->setText("V"+g_appData.otaVersion + "\n" + g_appData.otaVersionDes);
            }else{
                mPopOtaTitleTv->setText("MCU更新");
                mPopOtaScrollInfoTv->setText("V"+getMCUVersion(g_appData.mcuUpgradeVer) + "\n" + g_appData.mcuUpgradeText);
            }
        }
    }

    if(g_appData.statusChangeFlag & CS_MCU_OTA_ERROR){
        if((mPopType == POP_OTA) && (mOTAGroup->getVisibility() == View::VISIBLE)){
            mPopOtaInfoTv->setText( g_appData.mcuOTAError+"\nMCU更新失败！请稍后再试。");
            mPopOtaProgress->setProgress(100);
            mPopOtaProgressTv->setText("100%");
            mPopLayout->removeCallbacks(mConnectRun);
            mPopLayout->removeCallbacks(mCloseRun);
            mPopLayout->postDelayed(mCloseRun,10*1000);
            g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 设备状态
#endif
        }
    }

    if(g_appData.statusChangeFlag & CS_MCU_OTA_SUCCESS){
        if((mPopType == POP_OTA) && (mOTAGroup->getVisibility() == View::VISIBLE)){
            mPopOtaInfoTv->setText("MCU更新成功，正在重启！");
            mPopOtaProgress->setProgress(100);
            mPopOtaProgressTv->setText("100%");
            mPopLayout->removeCallbacks(mConnectRun);
            mPopLayout->removeCallbacks(mCloseRun);
            mCloseRun = [this](){
#ifdef CDROID_X64
                g_windMgr->dealClosePopPage();
#else
                setScreenLight(0,false);
                system("reboot");
#endif
            };
            mPopLayout->postDelayed(mCloseRun,10*1000);
        }
    }
}

void PopNormal::onBtnClickListener(View&v){
    LOGI("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::pop_enter:{
            switch(mPopType){
                case POP_OTA:{
                    mNormalGroup->setVisibility(View::GONE);
                    mOTAGroup->setVisibility(View::VISIBLE);
                    mPopOtaInfoTv->setText(g_appData.isNeedUpdate?"软件更新中...":"MCU更新中...");
                    mPopOtaProgressTv->setText("0%");
                    mPopOtaProgress->setProgress(0);
                    // g_objConnMgr->setResetStatus(true);
                    // g_objConnMgr->send2MCU();
                    g_appData.eqStatus = ES_RESET;
#ifndef TUYA_OS_DISABLE
                    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 设备状态
#endif
                    upDateDownLo(mUpDateFlag,mOtaProgress);
                    mPopLayout->postDelayed(mConnectRun,1000);
                    return;
                }case POP_RESTORE_FACTORY:{
                    g_objConf->setInitSetFlag(true);
                    g_objConf->update();
                    mPopGroup->setVisibility(View::INVISIBLE);
                    mPopLayout->setBackgroundResource("#FF000000");
                    g_objConnMgr->setShutDown(false);
                    // g_objConnMgr->setResetStatus(true);
                    // g_objConnMgr->send2MCU();
                    g_appData.eqStatus = ES_RESET;
#ifndef TUYA_OS_DISABLE
                    bool isreset = true;
                    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 设备状态
                    // g_tuyaOsMgr->reportDpData(TYCMD_FACTORY_RESET,PROP_BOOL, &isreset); // 设备状态
                    g_tuyaOsMgr->unBindTuya(true);
#endif
                    mPopLayout->postDelayed(mCloseRun,5*1000);
                    return;
                }case POP_UNBIND_APP:{
                    mPopGroup->setVisibility(View::INVISIBLE);
                    mPopLayout->setBackgroundResource("#FF000000");
#ifndef TUYA_OS_DISABLE
                    g_tuyaOsMgr->unBindTuya(false);
#endif
                    mPopLayout->postDelayed(mCloseRun,10*1000);
                    return;
                }
            }
            if(mEnterCallback) mEnterCallback();
            g_windMgr->dealClosePopPage();
            break;
        }case kaidu_ms7_lqy::R::id::pop_cancel:{
            if(mCancelCallback) mCancelCallback();
            g_windMgr->dealClosePopPage();
            break;
        }case kaidu_ms7_lqy::R::id::pop_click_tips_tv:
        case kaidu_ms7_lqy::R::id::pop_bind_app_img:{
            View *view = mPopLayout->findViewById(kaidu_ms7_lqy::R::id::qrcode_box);
            view->setActivated(!view->isActivated());
            view->setScaleX(view->isActivated()?2.f:1.f);
            view->setScaleY(view->isActivated()?2.f:1.f);
            mBindAppCilckTipsTv->setVisibility(view->isActivated()?View::GONE:View::VISIBLE);
            break;
        }case kaidu_ms7_lqy::R::id::pop_layout:{
            if(mPopType == POP_BIND_APP){
                g_windMgr->dealClosePopPage();
            }
            break;
        }
    }
}

