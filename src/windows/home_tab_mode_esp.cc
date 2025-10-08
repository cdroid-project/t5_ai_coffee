#include "home_tab_mode_esp.h"
#include "conf_mgr.h"
#include "conn_mgr.h"
#include "wind_mgr.h"
#include "tuya_os_mgr.h"
HomeTabModeEsp::HomeTabModeEsp(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){

    mEspData = g_objConf->getEspData();
    mCacheData = mEspData.sndData;
    mPageEditType  = HOME_PAGE_NORMAL;
    initGroup();
}

HomeTabModeEsp::~HomeTabModeEsp(){

}

void HomeTabModeEsp::initGroup(){
    mResetTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_reset);
    mHotWaterTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_hot_water);
    mSteamTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_steam);

    // tv->getBackground()->setLevel(5000);
    mOneBeanImg     = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_bean);
    mDoubleBeanImg  = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_bean);
    mOneCupImg      = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_cup);
    mDoubleCupImg   = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_cup);

    mCupInfoFavImg = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::cup_info_favorites);
    mArcPowder      = (ArcSeekBar *)mWindPageBox->findViewById(t5_ai_coffee::R::id::arc_powder);
    mArcRight       = (ArcSeekBar *)mWindPageBox->findViewById(t5_ai_coffee::R::id::arc_right);

    mPowderInfoTv   = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::powder_info_tv);
    mRightInfoTv    = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_tv);
    mRightInfoTitleTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_title_tv);
    mRightInfoUnitsTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_units_tv);

    mInfoRVPicker   = (RVNumberPicker *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_picker);
    mInfoImg = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::cup_info_img);

    espRightInfoList = {{RIGHT_INFO_SOAK,"预浸泡时间","s"},{RIGHT_INFO_EXT_WATER,"萃取水量","ml"},{RIGHT_INFO_EXT_TEMP,"萃取温度","℃"}};

}

void HomeTabModeEsp::updateGroupData(){
    mCacheData = mEspData.sndData;
    mPageEditType  = HOME_PAGE_NORMAL;

    mInfoRVPicker->setMaxValue(espRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(espRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.extractWaterDef) + "ml"; break;}
            case RIGHT_INFO_EXT_TEMP:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(espRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return espRightInfoList.at(value).name; break;}
            case RIGHT_INFO_EXT_TEMP:{ return espRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return espRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(1);
    mArcPowder->setShowSlider(true);
    mArcPowder->setMin(mEspData.powderMin);
    mArcPowder->setMax(mEspData.powderMax);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    mInfoImg->setImageResource(mEspData.pic);
    ((BitmapDrawable*) mInfoImg->getDrawable())->setFilterBitmap(true);
    LOGE("mEspData.pic = %s",mEspData.pic.c_str());

    updateRightInfoGroup(mInfoRVPicker->getValue());

    mCupInfoFavImg->setVisibility(View::VISIBLE);
    mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));

    mOneBeanImg->setVisibility(View::VISIBLE);
    mDoubleBeanImg->setVisibility(View::VISIBLE);
    mOneCupImg->setVisibility(View::VISIBLE);
    mDoubleCupImg->setVisibility(View::VISIBLE);

    mHotWaterTv->setVisibility(View::VISIBLE);
    mSteamTv->setVisibility(View::VISIBLE);
    mResetTv->setVisibility(View::VISIBLE);
    g_appData.statusChangeFlag |= CS_PRE_HEATING;
    updatePageData();
    g_appData.statusChangeFlag &= ~CS_PRE_HEATING;
    if(mCacheData.powderDef > 14){
        mBeanItemState = ITEM_ACTIVIT_SND;
        mOneBeanImg->setActivated(false);
        mDoubleBeanImg->setActivated(true);
    }else{
        mBeanItemState = ITEM_ACTIVIT_FIR;
        mOneBeanImg->setActivated(true);
        mDoubleBeanImg->setActivated(false);
    }
}

void HomeTabModeEsp::updatePageData(){
    if(g_appData.statusChangeFlag & CS_PRE_HEATING){
        if(g_appData.displayMode){
            mHotWaterTv->setText("测试中");
            mSteamTv->setText("测试中");
        }else{
            if(g_appData.coffeePreheatPtc >= 100){
                mHotWaterTv->setText("热水"); 
                mSteamTv->setText("蒸汽");
                mHotWaterTv->getBackground()->setLevel(0);
                mSteamTv->getBackground()->setLevel(0);
            }else{
                mHotWaterTv->setText("测试中");
                mSteamTv->setText("测试中");
                mHotWaterTv->getBackground()->setLevel(g_appData.coffeePreheatPtc*100);
                mSteamTv->getBackground()->setLevel(g_appData.coffeePreheatPtc*100);
            }
        }
        if(mPageEditType != HOME_PAGE_FAV_EDIT)updateBeanCupImgStatus();
    }
}

void HomeTabModeEsp::updateBeanCupImgStatus(){
    if((g_appData.coffeePreheatPtc >= 100) && (g_appData.coffeePreheatPtc >= 100)){
        if(mPageEditType == HOME_PAGE_NORMAL){
            if(mCacheData.extractWaterDef > 50){
                mCupItemState = ITEM_ACTIVIT_SND;
                if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)){
                    mOneCupImg->setActivated(false);
                    mDoubleCupImg->setActivated(true);
                }
            }else{
                mCupItemState = ITEM_ACTIVIT_FIR;
                if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)){
                    mOneCupImg->setActivated(true);
                    mDoubleCupImg->setActivated(false);
                }
            }
        }else if(mPageEditType == HOME_PAGE_FAV_EXT){
            mCupItemState = ITEM_ACTIVIT_FIR;
            if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)){
                mOneCupImg->setActivated(true);
            }
        }
    }else{
        mCupItemState = ITEM_ACTIVIT_NONE;
        mOneCupImg->setActivated(false);
        mDoubleCupImg->setActivated(false);
    }
}

void HomeTabModeEsp::updateRightInfoGroup(int pickPos){
    mRightInfoTitleTv->setText(espRightInfoList.at(pickPos).name);
    mRightInfoUnitsTv->setText(espRightInfoList.at(pickPos).units);
    switch(espRightInfoList.at(pickPos).type){
        case RIGHT_INFO_EXT_WATER:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.extractWaterDef-1);
                mArcRight->setMax(mCacheData.extractWaterDef);
            }else{
                mArcRight->setMin(mEspData.extractWaterMin);
                mArcRight->setMax(mEspData.extractWaterMax);
            }
            mArcRight->setProgress(mCacheData.extractWaterDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractWaterDef));
            break;
        }case RIGHT_INFO_EXT_TEMP:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.extractTempDef-1);
                mArcRight->setMax(mCacheData.extractTempDef);
            }else{
                mArcRight->setMin(mEspData.extractTempMin);
                mArcRight->setMax(mEspData.extractTempMax);
            }
            mArcRight->setProgress(mCacheData.extractTempDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
            break;
        }case RIGHT_INFO_SOAK:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.soakingTimeDef-1);
                mArcRight->setMax(mCacheData.soakingTimeDef);
            }else{
                mArcRight->setMin(mEspData.soakingTimeMin);
                mArcRight->setMax(mEspData.soakingTimeMax);
            }
            mArcRight->setProgress(mCacheData.soakingTimeDef);
            mRightInfoTv->setText(std::to_string(mCacheData.soakingTimeDef));
            break;
        }
    }
}

void HomeTabModeEsp::onFavModeEditListener(int Position, bool isEdit){
    mCacheData = mEspData.sndData;
    mPageEditType = isEdit?HOME_PAGE_FAV_EDIT:HOME_PAGE_FAV_EXT;
    mBeanItemState = ITEM_ACTIVIT_FIR;
    if(g_appData.coffeePreheatPtc >= 100)mCupItemState = ITEM_ACTIVIT_FIR;
    else mCupItemState = ITEM_ACTIVIT_NONE;
    mFavEditPos = Position;

    Json::Value sndData = g_objConf->getFavModeList()[Position]["sndModeData"];
    mCacheData.powderDef = getJsonInt(sndData,"powderDef");
    mCacheData.extractTempDef = getJsonInt(sndData,"extractTempDef");
    mCacheData.extractWaterDef = getJsonInt(sndData,"extractWaterDef");
    mCacheData.soakingTimeDef = getJsonInt(sndData,"soakingTimeDef");

    mInfoRVPicker->setMaxValue(espRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(espRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.extractWaterDef) + "ml"; break;}
            case RIGHT_INFO_EXT_TEMP:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return espRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(espRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return espRightInfoList.at(value).name; break;}
            case RIGHT_INFO_EXT_TEMP:{ return espRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return espRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(1);

    mInfoImg->setImageResource(mEspData.pic);
    LOGE("mEspData.pic = %s",mEspData.pic.c_str());

    mCupInfoFavImg->setVisibility(View::GONE);

    mResetTv->setVisibility(View::GONE);
    if(isEdit){
        mArcPowder->setShowSlider(true);
        mArcPowder->setMin(mEspData.powderMin);
        mArcPowder->setMax(mEspData.powderMax);

        mHotWaterTv->setVisibility(View::GONE);
        mSteamTv->setVisibility(View::GONE);
    }else{
        mArcPowder->setMin(mCacheData.powderDef-1);
        mArcPowder->setMax(mCacheData.powderDef);
        mArcPowder->setShowSlider(false);
        mArcRight->setShowSlider(false);

        mHotWaterTv->setVisibility(View::VISIBLE);
        mSteamTv->setVisibility(View::VISIBLE);
        if(g_appData.displayMode){
            mHotWaterTv->setText("测试中");
            mSteamTv->setText("测试中");
        }else{
            if(g_appData.coffeePreheatPtc >= 100){
                mHotWaterTv->setText("热水"); 
                mSteamTv->setText("蒸汽");
                mHotWaterTv->getBackground()->setLevel(0);
                mSteamTv->getBackground()->setLevel(0);
            }else{
                mHotWaterTv->setText("测试中");
                mSteamTv->setText("测试中");
                mHotWaterTv->getBackground()->setLevel(g_appData.coffeePreheatPtc*100);
                mSteamTv->getBackground()->setLevel(g_appData.coffeePreheatPtc*100);
            }
        }
    }
    updateRightInfoGroup(mInfoRVPicker->getValue());
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
}

void HomeTabModeEsp::dealRightInfoAddReductClick(bool isAdd){
    int infoGear;
    switch(espRightInfoList.at(mInfoRVPicker->getValue()).type){
        case RIGHT_INFO_EXT_WATER:{
            infoGear = mEspData.extractWaterGear;
            break;
        }case RIGHT_INFO_EXT_TEMP:{
            infoGear = mEspData.extractTempGear;
            break;
        }case RIGHT_INFO_SOAK:{
            infoGear = mEspData.soakingTimeGear;
            break;
        }
    }
    int calcProgress;
    if(isAdd){
        calcProgress = mArcRight->getProgress()+infoGear;
        calcProgress = calcProgress<=mArcRight->getMax()?calcProgress:mArcRight->getMax();
    }else{
        calcProgress = mArcRight->getProgress()-infoGear;
        calcProgress = calcProgress>=mArcRight->getMin()?calcProgress:mArcRight->getMin();
    }

    mArcRight->setProgress(calcProgress);
    onArcValueChangeListener(*mArcRight,calcProgress,true);
}

void HomeTabModeEsp::onItemClickListener(View &v){
    switch(v.getId()){
        case t5_ai_coffee::R::id::right_info_reduce:{
            dealRightInfoAddReductClick(false);
            break;
        }case t5_ai_coffee::R::id::right_info_add:{
            dealRightInfoAddReductClick(true);
            break;
        }case t5_ai_coffee::R::id::cup_info_favorites:{
            v.setActivated(!v.isActivated());

            mEspData.sndData = mCacheData;
            Json::Value favItemData = getCacheDataToJson();
            if(v.isActivated()){
                g_objConf->addFavModeData(favItemData);
            }else{
                g_objConf->deleteFavModeData(favItemData);
            }
            break;
        }case t5_ai_coffee::R::id::powder_add:
        case t5_ai_coffee::R::id::powder_reduce:{
            mCacheData.powderDef = mArcPowder->getProgress();
            break;
        }case t5_ai_coffee::R::id::icon_one_bean:
        case t5_ai_coffee::R::id::icon_double_bean:
        case t5_ai_coffee::R::id::icon_one_cup:
        case t5_ai_coffee::R::id::icon_double_cup:{
            dealPowderItemClick(v);
            break;
        }case t5_ai_coffee::R::id::btn_reset:{
            if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)){
                mEspData.sndData = g_objConf->getEspData(true).sndData;
                mCacheData = mEspData.sndData;
                switch(espRightInfoList.at(mInfoRVPicker->getValue()).type){
                    case RIGHT_INFO_EXT_WATER:{
                        mArcRight->setProgress(mEspData.sndData.extractWaterDef);
                        mRightInfoTv->setText(std::to_string(mEspData.sndData.extractWaterDef));
                        break;
                    }case RIGHT_INFO_EXT_TEMP:{
                        mArcRight->setProgress(mEspData.sndData.extractTempDef);
                        mRightInfoTv->setText(std::to_string(mEspData.sndData.extractTempDef));
                        break;
                    }case RIGHT_INFO_SOAK:{
                        mArcRight->setProgress(mEspData.sndData.soakingTimeDef);
                        mRightInfoTv->setText(std::to_string(mEspData.sndData.soakingTimeDef));
                        break;
                    }
                }
                updateBeanCupImgStatus();
                if(mCacheData.powderDef > 14){
                    mBeanItemState = ITEM_ACTIVIT_SND;
                    mOneBeanImg->setActivated(false);
                    mDoubleBeanImg->setActivated(true);
                }else{
                    mBeanItemState = ITEM_ACTIVIT_FIR;
                    mOneBeanImg->setActivated(true);
                    mDoubleBeanImg->setActivated(false);
                }
                mArcPowder->setProgress(mEspData.sndData.powderDef);
                mPowderInfoTv->setText(std::to_string(mEspData.sndData.powderDef));
                mCupInfoFavImg->setActivated(false);
                mInfoRVPicker->notifyUpdate();
                mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
            }
            break;
        }case t5_ai_coffee::R::id::btn_hot_water:{
            if((g_appData.machineState & MC_STATE_POWDER) || (g_appData.machineState & MC_STATE_EXT)){
                LOGE("error!!! 工作中不能出热水");
            }else{
                if(mHotWaterTv->getBackground()->getLevel() == 10000){
                    if(mCupItemState & ITEM_ACTIVIT_FIR)mOneCupImg->setActivated(true);
                    else if(mCupItemState & ITEM_ACTIVIT_SND)mDoubleCupImg->setActivated(true);
                    if(mBeanItemState & ITEM_ACTIVIT_FIR)mOneBeanImg->setActivated(true);
                    else if(mBeanItemState & ITEM_ACTIVIT_SND)mDoubleBeanImg->setActivated(true);
                    updateRightInfoGroup(mInfoRVPicker->getValue());
                }
            }
            break;
        }case t5_ai_coffee::R::id::fav_enter_img:{
            Json::Value favItemData = g_objConf->getFavModeList()[mFavEditPos];
            Json::Value favSndItemData;
            favSndItemData["powderDef"] = mArcPowder->getProgress();
            favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
            favSndItemData["extractWaterDef"] = mCacheData.extractWaterDef;
            favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

            favItemData["sndModeData"] = favSndItemData;
            g_objConf->editFavModeData(favItemData,mFavEditPos);
            
            break;
        }
    }
}


void HomeTabModeEsp::dealPowderItemClick(View &v){
    if(!v.isActivated()) return;

    LOGE("%d %d",v.isSelected(),v.isActivated());
    // 不管单双，共同的操作
    switch(v.getId()){
        case t5_ai_coffee::R::id::icon_one_bean:
        case t5_ai_coffee::R::id::icon_double_bean:{
            if(v.isSelected()){
                v.setSelected(false);
                if(mCupItemState & ITEM_ACTIVIT_FIR)mOneCupImg->setActivated(true);
                else if(mCupItemState & ITEM_ACTIVIT_SND)mDoubleCupImg->setActivated(true);
            }else if(v.isActivated()){
                v.setSelected(true);
                mDoubleCupImg->setActivated(false);
                mOneCupImg->setActivated(false);
                mCacheData.powderDef = mArcPowder->getProgress();
                mEspData.sndData.powderDef = mArcPowder->getProgress();
            }
            break;
        }case t5_ai_coffee::R::id::icon_one_cup:
        case t5_ai_coffee::R::id::icon_double_cup:{
            if(v.isSelected()){
                v.setSelected(false);
                if(mBeanItemState & ITEM_ACTIVIT_FIR)mOneBeanImg->setActivated(true);
                else if(mBeanItemState & ITEM_ACTIVIT_SND)mDoubleBeanImg->setActivated(true);
            }else if(v.isActivated()){
                v.setSelected(true);
                mOneBeanImg->setActivated(false);
                mDoubleBeanImg->setActivated(false);

                mEspData.sndData.extractTempDef = mCacheData.extractTempDef;
                mEspData.sndData.extractWaterDef = mCacheData.extractWaterDef;
                mEspData.sndData.soakingTimeDef = mCacheData.soakingTimeDef;
            }

            int extEspTotal = g_objConf->getEspTotal()+1;
            // 涂鸦状态上报 
            bool isStart = v.isSelected();
            g_appData.eqStatus = v.isSelected()?ES_EXTRACTING:(!(g_appData.machineState & MC_STATE_EXT))?ES_EXTRACT_DONE:ES_STANDBY;
#ifndef TUYA_OS_DISABLE

            g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); // 启动/暂停
            if(v.isSelected()){
                int extType = MAKE_TYPE_COFFEE;
                int coffeeMode = CFM_ESP;
                g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); // 制作大类型
                g_tuyaOsMgr->reportDpData(TYCMD_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
                g_tuyaOsMgr->reportCoffeeDiyRawData(COFFEE_TYPE_NORMAL, CFM_ESP, mEspData.sndData.extractTempDef,
                                                    mEspData.sndData.extractWaterDef, mEspData.sndData.soakingTimeDef,0,0); // 咖啡DIY
            }else{
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_ESPRESSO,PROP_VALUE, &extEspTotal); // 意式咖啡萃取次数
            }
                
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif

            if(v.isSelected())  g_appData.machineState |= MC_STATE_EXT;
            else{
                // 若已经是非工作状态，则不需要下发萃取指令
                if(!(g_appData.machineState & MC_STATE_EXT)){
                    g_objConf->setEspTotal(extEspTotal);
                    return;
                }
                g_appData.machineState &= ~MC_STATE_EXT;
            }
            g_objConnMgr->setExtraction(v.isSelected(),EXT_MODE_ESP,mEspData.sndData.soakingTimeDef,90 
                                    ,mEspData.sndData.extractWaterDef,mEspData.sndData.extractTempDef,0,0);
            break;
        }
    }
}

void HomeTabModeEsp::onArcValueChangeListener(View &v, int progress, bool fromUser){
    if(!fromUser) return;
    switch(v.getId()){
        case t5_ai_coffee::R::id::arc_powder:{
            mPowderInfoTv->setText(std::to_string(progress));
            mCacheData.powderDef = progress;
            if(progress>14){
                mBeanItemState = ITEM_ACTIVIT_SND;
                mOneBeanImg->setActivated(false);
                mDoubleBeanImg->setActivated(true);
            }else{
                mBeanItemState = ITEM_ACTIVIT_FIR;
                mOneBeanImg->setActivated(true);
                mDoubleBeanImg->setActivated(false);
            }
            break;
        }case t5_ai_coffee::R::id::arc_right:{
            mRightInfoTv->setText(std::to_string(progress));
            switch(espRightInfoList.at(mInfoRVPicker->getValue()).type){
                case RIGHT_INFO_EXT_WATER:{
                    mCacheData.extractWaterDef = progress;
                    if(g_appData.coffeePreheatPtc >= 100){
                        if(progress>50){
                            mCupItemState = ITEM_ACTIVIT_SND;
                            mOneCupImg->setActivated(false);
                            mDoubleCupImg->setActivated(true);
                        }else{
                            mCupItemState = ITEM_ACTIVIT_FIR;
                            mOneCupImg->setActivated(true);
                            mDoubleCupImg->setActivated(false);
                        }
                    }
                    break;
                }case RIGHT_INFO_EXT_TEMP:{
                    mCacheData.extractTempDef = progress;
                    break;
                }case RIGHT_INFO_SOAK:{
                    mCacheData.soakingTimeDef = progress;
                    break;
                }
            }
            mInfoRVPicker->notifyUpdatePostion(mInfoRVPicker->getValue());
            break;
        }
    }
    mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
}

void HomeTabModeEsp::dealTuyaStartWork(){
    mEspData.sndData.extractTempDef = g_appData.tuyaDiyData[2];
    mEspData.sndData.extractWaterDef = g_appData.tuyaDiyData[3];
    mEspData.sndData.soakingTimeDef = g_appData.tuyaDiyData[4];
    g_objConf->setEspData(mEspData);
}

void HomeTabModeEsp::dealAiData(){
    mEspData.sndData.extractTempDef = getJsonInt(g_appData.aiJsonText["params"],"extraction_temp");
    mEspData.sndData.extractWaterDef = getJsonInt(g_appData.aiJsonText["params"],"extraction_water");
    mEspData.sndData.soakingTimeDef = getJsonInt(g_appData.aiJsonText["params"],"pre_soak_time");
}


Json::Value HomeTabModeEsp::getCacheDataToJson(){
    Json::Value favItemData;
    Json::Value favSndItemData;
    favSndItemData["powderDef"] = mCacheData.powderDef;
    favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
    favSndItemData["extractWaterDef"] = mCacheData.extractWaterDef;
    favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

    favItemData["coffeeMode"] = HOME_MT_ESPRESSO;
    favItemData["name"] = "意式咖啡";
    favItemData["beanGrindMode"] = g_objConf->getBeanGrindMode();
    favItemData["sndModeData"] = favSndItemData;

    return favItemData;
}