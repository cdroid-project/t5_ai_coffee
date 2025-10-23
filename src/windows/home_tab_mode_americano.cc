/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 17:42:15
 * @LastEditTime: 2025-10-23 14:05:36
 * @FilePath: /t5_ai_coffee/src/windows/home_tab_mode_americano.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "home_tab_mode_americano.h"
#include "conf_mgr.h"
#include "conn_mgr.h"
#include "wind_mgr.h"
#include "tuya_os_mgr.h"
HomeTabModeAmericano::HomeTabModeAmericano(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){

    mAmericanData = g_objConf->getAmericanData();
    mCacheData = mAmericanData.sndData;
    mPageEditType  = HOME_PAGE_NORMAL;
    initGroup();
}

HomeTabModeAmericano::~HomeTabModeAmericano(){

}

void HomeTabModeAmericano::initGroup(){
    mResetTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_reset);
    mHotWaterTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_hot_water);
    mSteamTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::btn_steam);

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

    ameRightInfoList = {{RIGHT_INFO_HOT_WATER_TEMP,"热水温度","℃"},{RIGHT_INFO_SOAK,"预浸泡时间","s"},{RIGHT_INFO_EXT_WATER,"萃取水量","ml"},
                        {RIGHT_INFO_EXT_TEMP,"萃取温度","℃"},{RIGHT_INFO_HOT_WATER,"热水水量","ml"}};
}

void HomeTabModeAmericano::updateGroupData(){
    mCacheData = mAmericanData.sndData;
    mPageEditType  = HOME_PAGE_NORMAL;

    mInfoRVPicker->setMaxValue(ameRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(ameRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.extractWaterDef) + "ml"; break;}
            case RIGHT_INFO_EXT_TEMP:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}
            case RIGHT_INFO_HOT_WATER:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.hotWaterDef) + "ml";break;}
            case RIGHT_INFO_HOT_WATER_TEMP:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.hotWaterTempDef) + "℃";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(ameRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return ameRightInfoList.at(value).name; break;}
            case RIGHT_INFO_EXT_TEMP:{ return ameRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return ameRightInfoList.at(value).name;break;}
            case RIGHT_INFO_HOT_WATER:{ return ameRightInfoList.at(value).name;break;}
            case RIGHT_INFO_HOT_WATER_TEMP:{ return ameRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(2);
    mArcPowder->setShowSlider(true);
    mArcPowder->setMin(mAmericanData.powderMin);
    mArcPowder->setMax(mAmericanData.powderMax);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    mInfoImg->setImageResource(mAmericanData.pic);
    ((BitmapDrawable*) mInfoImg->getDrawable())->setFilterBitmap(true);
    LOGE("mAmericanData.pic = %s",mAmericanData.pic.c_str());

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

void HomeTabModeAmericano::updatePageData(){
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

void HomeTabModeAmericano::updateBeanCupImgStatus(){
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

void HomeTabModeAmericano::updateRightInfoGroup(int pickPos){
    mRightInfoTitleTv->setText(ameRightInfoList.at(pickPos).name);
    mRightInfoUnitsTv->setText(ameRightInfoList.at(pickPos).units);
    switch(ameRightInfoList.at(pickPos).type){
        case RIGHT_INFO_EXT_WATER:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.extractWaterDef-1);
                mArcRight->setMax(mCacheData.extractWaterDef);
            }else{
                mArcRight->setMin(mAmericanData.extractWaterMin);
                mArcRight->setMax(mAmericanData.extractWaterMax);
            }
            mArcRight->setProgress(mCacheData.extractWaterDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractWaterDef));
            break;
        }case RIGHT_INFO_EXT_TEMP:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.extractTempDef-1);
                mArcRight->setMax(mCacheData.extractTempDef);
            }else{
                mArcRight->setMin(mAmericanData.extractTempMin);
                mArcRight->setMax(mAmericanData.extractTempMax);
            }
            mArcRight->setProgress(mCacheData.extractTempDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
            break;
        }case RIGHT_INFO_SOAK:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.soakingTimeDef-1);
                mArcRight->setMax(mCacheData.soakingTimeDef);
            }else{
                mArcRight->setMin(mAmericanData.soakingTimeMin);
                mArcRight->setMax(mAmericanData.soakingTimeMax);
            }
            mArcRight->setProgress(mCacheData.soakingTimeDef);
            mRightInfoTv->setText(std::to_string(mCacheData.soakingTimeDef));
            break;
        }case RIGHT_INFO_HOT_WATER:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.hotWaterDef-1);
                mArcRight->setMax(mCacheData.hotWaterDef);
            }else{
                 mArcRight->setMin(mAmericanData.hotWaterMin);
                mArcRight->setMax(mAmericanData.hotWaterMax);
            }
            mArcRight->setProgress(mCacheData.hotWaterDef);
            mRightInfoTv->setText(std::to_string(mCacheData.hotWaterDef));
            break;
        }case RIGHT_INFO_HOT_WATER_TEMP:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.hotWaterTempDef-1);
                mArcRight->setMax(mCacheData.hotWaterTempDef);
            }else{
                mArcRight->setMin(mAmericanData.hotWaterTempMin);
                mArcRight->setMax(mAmericanData.hotWaterTempMax);
            }
            mArcRight->setProgress(mCacheData.hotWaterTempDef);
            mRightInfoTv->setText(std::to_string(mCacheData.hotWaterTempDef));
            break;
        } 
    }
}


void HomeTabModeAmericano::onFavModeEditListener(int Position, bool isEdit){
    Json::Value sndData = g_objConf->getFavModeList()[Position]["sndModeData"];
    mCacheData = mAmericanData.sndData;
    mPageEditType = isEdit?HOME_PAGE_FAV_EDIT:HOME_PAGE_FAV_EXT;
    mBeanItemState = ITEM_ACTIVIT_FIR;
    if(g_appData.coffeePreheatPtc >= 100)mCupItemState = ITEM_ACTIVIT_FIR;
    else mCupItemState = ITEM_ACTIVIT_NONE;
    mFavEditPos = Position;

    mCacheData.powderDef = getJsonInt(sndData,"powderDef");
    mCacheData.extractTempDef = getJsonInt(sndData,"extractTempDef");
    mCacheData.extractWaterDef = getJsonInt(sndData,"extractWaterDef");
    mCacheData.hotWaterTempDef = getJsonInt(sndData,"hotWaterTempDef");
    mCacheData.hotWaterDef = getJsonInt(sndData,"hotWaterDef");
    mCacheData.soakingTimeDef = getJsonInt(sndData,"soakingTimeDef");

    mInfoRVPicker->setMaxValue(ameRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(ameRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.extractWaterDef) + "ml"; break;}
            case RIGHT_INFO_EXT_TEMP:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}
            case RIGHT_INFO_HOT_WATER:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.hotWaterDef) + "ml";break;}
            case RIGHT_INFO_HOT_WATER_TEMP:{ return ameRightInfoList.at(value).name + std::to_string(mCacheData.hotWaterTempDef) + "℃";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(ameRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_WATER:{ return ameRightInfoList.at(value).name; break;}
            case RIGHT_INFO_EXT_TEMP:{ return ameRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return ameRightInfoList.at(value).name;break;}
            case RIGHT_INFO_HOT_WATER:{ return ameRightInfoList.at(value).name;break;}
            case RIGHT_INFO_HOT_WATER_TEMP:{ return ameRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(2);

    mInfoImg->setImageResource(mAmericanData.pic);
    mCupInfoFavImg->setVisibility(View::GONE);

    mResetTv->setVisibility(View::GONE);
    if(isEdit){
        mArcPowder->setShowSlider(true);
        mArcPowder->setMin(mAmericanData.powderMin);
        mArcPowder->setMax(mAmericanData.powderMax);

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

void HomeTabModeAmericano::dealRightInfoAddReductClick(bool isAdd){
    int infoGear;
    switch(ameRightInfoList.at(mInfoRVPicker->getValue()).type){
        case RIGHT_INFO_EXT_WATER:{
            infoGear = mAmericanData.extractWaterGear;
            break;
        }case RIGHT_INFO_EXT_TEMP:{
            infoGear = mAmericanData.extractTempGear;
            break;
        }case RIGHT_INFO_SOAK:{
            infoGear = mAmericanData.soakingTimeGear;
            break;
        }case RIGHT_INFO_HOT_WATER:{
            infoGear = mAmericanData.hotWaterGear;
            break;
        }case RIGHT_INFO_HOT_WATER_TEMP:{
            infoGear = mAmericanData.hotWaterTempGear;
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

void HomeTabModeAmericano::onItemClickListener(View &v){
    switch(v.getId()){
        case t5_ai_coffee::R::id::right_info_reduce:{
            dealRightInfoAddReductClick(false);
            break;
        }case t5_ai_coffee::R::id::right_info_add:{
            dealRightInfoAddReductClick(true);
            break;
        }case t5_ai_coffee::R::id::cup_info_favorites:{
            v.setActivated(!v.isActivated());
            mAmericanData.sndData = mCacheData;
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
                mAmericanData.sndData = g_objConf->getAmericanData(true).sndData;
                mCacheData = mAmericanData.sndData;
                switch(ameRightInfoList.at(mInfoRVPicker->getValue()).type){
                    case RIGHT_INFO_EXT_WATER:{
                        mArcRight->setProgress(mAmericanData.sndData.extractWaterDef);
                        mRightInfoTv->setText(std::to_string(mAmericanData.sndData.extractWaterDef));
                        break;
                    }case RIGHT_INFO_EXT_TEMP:{
                        mArcRight->setProgress(mAmericanData.sndData.extractTempDef);
                        mRightInfoTv->setText(std::to_string(mAmericanData.sndData.extractTempDef));
                        break;
                    }case RIGHT_INFO_SOAK:{
                        mArcRight->setProgress(mAmericanData.sndData.soakingTimeDef);
                        mRightInfoTv->setText(std::to_string(mAmericanData.sndData.soakingTimeDef));
                        break;
                    }case RIGHT_INFO_HOT_WATER:{
                        mArcRight->setProgress(mAmericanData.sndData.hotWaterDef);
                        mRightInfoTv->setText(std::to_string(mAmericanData.sndData.hotWaterDef));
                        break;
                    }case RIGHT_INFO_HOT_WATER_TEMP:{
                        mArcRight->setProgress(mAmericanData.sndData.hotWaterTempDef);
                        mRightInfoTv->setText(std::to_string(mAmericanData.sndData.hotWaterTempDef));
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
                mArcPowder->setProgress(mAmericanData.sndData.powderDef);
                mPowderInfoTv->setText(std::to_string(mAmericanData.sndData.powderDef));
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
            favSndItemData["hotWaterTempDef"] = mCacheData.hotWaterTempDef;
            favSndItemData["hotWaterDef"] = mCacheData.hotWaterDef;
            favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

            favItemData["sndModeData"] = favSndItemData;
            g_objConf->editFavModeData(favItemData,mFavEditPos);
            
            break;
        }
    }
}

void HomeTabModeAmericano::dealPowderItemClick(View &v){
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
                mAmericanData.sndData.powderDef = mArcPowder->getProgress();
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
                mAmericanData.sndData.extractTempDef = mCacheData.extractTempDef;
                mAmericanData.sndData.extractWaterDef = mCacheData.extractWaterDef;
                mAmericanData.sndData.hotWaterTempDef = mCacheData.hotWaterTempDef;
                mAmericanData.sndData.hotWaterDef = mCacheData.hotWaterDef;
                mAmericanData.sndData.soakingTimeDef = mCacheData.soakingTimeDef;
            }

            int extAmericanoTotal = g_objConf->getAmericanoTotal()+1;
            // 涂鸦状态上报 
            bool isStart = v.isSelected();
            g_appData.eqStatus = v.isSelected()?ES_EXTRACTING:(!(g_appData.machineState & MC_STATE_EXT))?ES_EXTRACT_DONE:ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); // 启动/暂停
            if(v.isSelected()){
                int extType = MAKE_TYPE_COFFEE;
                int coffeeMode = CFM_AMERICANO;
                g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); // 制作大类型
                g_tuyaOsMgr->reportDpData(TYCMD_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
                g_tuyaOsMgr->reportCoffeeDiyRawData(COFFEE_TYPE_NORMAL, CFM_AMERICANO, mAmericanData.sndData.extractTempDef,
                                                    mAmericanData.sndData.extractWaterDef, mAmericanData.sndData.soakingTimeDef,
                                                    mAmericanData.sndData.hotWaterDef, mAmericanData.sndData.hotWaterTempDef); // 咖啡DIY
            }else{
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_AMERICANO,PROP_VALUE, &extAmericanoTotal); // 美式咖啡萃取次数
            }
                
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
            if(v.isSelected())  g_appData.machineState |= MC_STATE_EXT;
            else{
                // 若已经是非工作状态，则不需要下发萃取指令
                if(!(g_appData.machineState & MC_STATE_EXT)){
                    g_objConf->setAmericanoTotal(extAmericanoTotal);
                    return;
                }
                g_appData.machineState &= ~MC_STATE_EXT;
            }
            g_objConnMgr->setExtraction(v.isSelected(),EXT_MODE_AMERICANO,mAmericanData.sndData.soakingTimeDef,90 
                                        ,mAmericanData.sndData.extractWaterDef,mAmericanData.sndData.extractTempDef,mAmericanData.sndData.hotWaterDef,mAmericanData.sndData.hotWaterTempDef);
            break;
        }
    }
}

void HomeTabModeAmericano::onArcValueChangeListener(View &v, int progress, bool fromUser){
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
            switch(ameRightInfoList.at(mInfoRVPicker->getValue()).type){
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
                }case RIGHT_INFO_HOT_WATER:{
                    mCacheData.hotWaterDef = progress;
                    break;
                }case RIGHT_INFO_HOT_WATER_TEMP:{
                    mCacheData.hotWaterTempDef = progress;
                    break;
                } 
            }
            mInfoRVPicker->notifyUpdatePostion(mInfoRVPicker->getValue());
            break;
        }
    }
    mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
}

void HomeTabModeAmericano::dealTuyaStartWork(){
    mAmericanData.sndData.extractTempDef = g_appData.tuyaDiyData[2];
    mAmericanData.sndData.extractWaterDef = g_appData.tuyaDiyData[3];
    mAmericanData.sndData.soakingTimeDef = g_appData.tuyaDiyData[4];
    mAmericanData.sndData.hotWaterDef = g_appData.tuyaDiyData[5];
    mAmericanData.sndData.hotWaterTempDef = g_appData.tuyaDiyData[6];
    g_objConf->setAmericanData(mAmericanData);
}

void HomeTabModeAmericano::dealAiData(){
    mAmericanData.sndData.extractTempDef = getJsonInt(g_appData.aiJsonText["params"],"extraction_temp");
    mAmericanData.sndData.extractWaterDef = getJsonInt(g_appData.aiJsonText["params"],"extraction_water");
    mAmericanData.sndData.soakingTimeDef = getJsonInt(g_appData.aiJsonText["params"],"pre_soak_time");
    mAmericanData.sndData.hotWaterDef = getJsonInt(g_appData.aiJsonText["params"],"hot_water_volume");
    mAmericanData.sndData.hotWaterTempDef = getJsonInt(g_appData.aiJsonText["params"],"hot_water_temp");
}

Json::Value HomeTabModeAmericano::getCacheDataToJson(){
    Json::Value favItemData;
    Json::Value favSndItemData;
    favSndItemData["powderDef"] = mCacheData.powderDef;
    favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
    favSndItemData["extractWaterDef"] = mCacheData.extractWaterDef;
    favSndItemData["hotWaterTempDef"] = mCacheData.hotWaterTempDef;
    favSndItemData["hotWaterDef"] = mCacheData.hotWaterDef;
    favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

    favItemData["coffeeMode"] = HOME_MT_AMERICANO;
    favItemData["name"] = "美式咖啡";
    favItemData["beanGrindMode"] = g_objConf->getBeanGrindMode();
    favItemData["sndModeData"] = favSndItemData;

    return favItemData;
}