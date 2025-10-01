#include "home_tab_mode_mas_esp.h"
#include "conf_mgr.h"
#include "wind_mgr.h"
#include <plotaxis.h>
#include <cyplotobject.h>
#include "conn_mgr.h"
#include "tuya_os_mgr.h"
HomeTabModeMasEsp::HomeTabModeMasEsp(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){
    mMasEspData = g_objConf->getMasEspData();
    mPageEditType = HOME_PAGE_NORMAL;
    initGroup();
}

HomeTabModeMasEsp::~HomeTabModeMasEsp(){

}

void HomeTabModeMasEsp::initGroup(){
    // 公有控件属性
    mOneBeanImg     = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_bean);
    mDoubleBeanImg  = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_bean);
    mOneCupImg      = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_cup);
    mDoubleCupImg   = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_cup);

    mArcPowder      = (ArcSeekBar *)mWindPageBox->findViewById(t5_ai_coffee::R::id::arc_powder);
    mArcRight       = (ArcSeekBar *)mWindPageBox->findViewById(t5_ai_coffee::R::id::arc_right);

    mPowderInfoTv   = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::powder_info_tv);
    mRightInfoTv    = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_tv);
    mRightInfoTitleTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_title_tv);
    mRightInfoUnitsTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_units_tv);

    mInfoRVPicker   = (RVNumberPicker *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_picker);

    masEspRightInfoList = {{RIGHT_INFO_SOAK,"预浸泡时间","s"},{RIGHT_INFO_EXT_TEMP,"萃取温度","℃"}};
    masEspPlotHistoryDataList.clear();

    // 特定控件
    mPlotInfoPlotView   = (CYPlotView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_plotview);

    mPlotInfoFavImg     = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_plot_favorites);
    mPlotInfoEditImg    =  (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_plot_edit);
    mPlotInfoModePicker = (NumberPicker *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_plot_hor_picker);

    mPlotInfoModePicker->setMaxValue(mMasEspData.sndModeList.size()-1);
    mPlotInfoModePicker->setFormatter([this](int value){ return value>= mMasEspData.sndModeList.size()?"error formatter":mMasEspData.sndModeList.at(value).sndModename; });

    
#if 0
    mMasEspHorPickerState = NumberPicker::OnScrollListener::SCROLL_STATE_IDLE;
    mPlotInfoModePicker->setOnValueChangedListener([this](NumberPicker&picker,int oldValue,int newValue){
        if(mMasEspHorPickerState == NumberPicker::OnScrollListener::SCROLL_STATE_IDLE){
            mWindPageBox->removeCallbacks(mHorPickerUpdateRun);
            mWindPageBox->postDelayed(mHorPickerUpdateRun,100);
        }
    });

    NumberPicker::OnScrollListener scrollListener;
    scrollListener.onScrollStateChange = [this](View &v,int state){
        if(state == NumberPicker::OnScrollListener::SCROLL_STATE_IDLE){
            mWindPageBox->removeCallbacks(mHorPickerUpdateRun);
            mWindPageBox->postDelayed(mHorPickerUpdateRun,300);
        }
        mMasEspHorPickerState = state;
        LOGI("mMasEspHorPickerState = %d",mMasEspHorPickerState);
    };
    mPlotInfoModePicker->setOnScrollListener(scrollListener);

    mHorPickerUpdateRun = [this](){
        onHorPickerChange
    };
#else
    mPlotInfoModePicker->setOnValueChangedListener([this](NumberPicker&picker,int oldValue,int newValue){
        onHorPickerChange();
    });
#endif
    
    mPlotInfoEditImg->setOnClickListener([this](View &v){
        g_windMgr->showSelectPosPopPage(POP_TYPE_MASTER_ESPRESSO,mWindPageBox,mPlotInfoModePicker->getValue(),
        [this](int selectPos){
            mMasEspData = g_objConf->getMasEspData();
            // 更新选中项的名字
            if(mPlotInfoModePicker->getValue() == selectPos){
                mCacheData.stepDataList = mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).stepDataList;
                setPlotViewData();
                mPlotInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
            }else{
                mPlotInfoModePicker->setValue(selectPos);
                onHorPickerChange();
            }
        },[this](int selectPos){
            if(mPlotInfoModePicker->getValue() != selectPos){
                mPlotInfoModePicker->setValue(selectPos);
                onHorPickerChange();
            }
        });
    });

    initPlotViewData();
}

void HomeTabModeMasEsp::updateGroupData(){
    mPageEditType = HOME_PAGE_NORMAL;
    mCacheData = mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue());

    mPlotInfoModePicker->setVisibility(View::VISIBLE);
    mPlotInfoFavImg->setVisibility(View::VISIBLE);
    mPlotInfoEditImg->setVisibility(View::VISIBLE);
    ((RelativeLayout::LayoutParams *)mPlotInfoPlotView->getLayoutParams())->setMargins(0,0,0,0);

    mInfoRVPicker->setMaxValue(masEspRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(masEspRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_TEMP:{ return masEspRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return masEspRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(masEspRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_TEMP:{ return masEspRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return masEspRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(1);
    mArcPowder->setShowSlider(true);
    mArcPowder->setMin(mMasEspData.powderMin);
    mArcPowder->setMax(mMasEspData.powderMax);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));

    updateRightInfoGroup(mInfoRVPicker->getValue());

    mPlotInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));

    mOneBeanImg->setVisibility(View::VISIBLE);
    mDoubleBeanImg->setVisibility(View::GONE);
    mOneCupImg->setVisibility(View::VISIBLE);
    mDoubleCupImg->setVisibility(View::GONE);

    updateBeanCupImgStatus();

    mOneBeanImg->setActivated(true);

    // 初始化 实时折线图
    masEspPlotHistoryDataList.clear(); // 清空历史数据
    mPlotRealObj->clearPoints();    // 先清空所有数据
    mPlotRealObj->addPoint(0,0);
    setPlotViewData();
}

void HomeTabModeMasEsp::updatePageData(){
    if(g_appData.statusChangeFlag & CS_PRE_HEATING){
        if(mPageEditType != HOME_PAGE_FAV_EDIT)updateBeanCupImgStatus();
    }
}

void HomeTabModeMasEsp::updateBeanCupImgStatus(){
    if(g_appData.coffeePreheatPtc >= 100){
        if(mPageEditType != HOME_PAGE_FAV_EDIT){
            if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)) {
                mOneCupImg->setActivated(true);
            }
        }
    }else{
        mOneCupImg->setActivated(false);
    }
}

void HomeTabModeMasEsp::updatePoltViewData(){
    if(g_appData.machineState & MC_STATE_EXT){
        if(masEspPlotHistoryDataList.size() > 0){
            if(g_appData.extractWater > masEspPlotHistoryDataList.back().extractWater){
                masEspPlotHistoryDataList.push_back({g_appData.extractWater,g_appData.extractPressure/10.0});
                mPlotRealObj->addPoint(g_appData.extractWater,g_appData.extractPressure/10.0);
                mPlotInfoPlotView->invalidate();
            }
        }else{
            masEspPlotHistoryDataList.push_back({g_appData.extractWater,g_appData.extractPressure/10.0});
            mPlotRealObj->addPoint(g_appData.extractWater,g_appData.extractPressure/10.0);
            mPlotInfoPlotView->invalidate();
        }
        LOGI("size = %d  water = %d   pressure = %d",masEspPlotHistoryDataList.size(),g_appData.extractWater,g_appData.extractPressure/10.0);
    }else {
        masEspPlotHistoryDataList.clear();
    }
}

void HomeTabModeMasEsp::updateRightInfoGroup(int pickPos){
    mRightInfoTitleTv->setText(masEspRightInfoList.at(pickPos).name);
    mRightInfoUnitsTv->setText(masEspRightInfoList.at(pickPos).units);
    switch(masEspRightInfoList.at(pickPos).type){
        case RIGHT_INFO_EXT_TEMP:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.extractTempDef-1);
                mArcRight->setMax(mCacheData.extractTempDef);
            }else{
                mArcRight->setMin(mMasEspData.extractTempMin);
                mArcRight->setMax(mMasEspData.extractTempMax);
            }
            mArcRight->setProgress(mCacheData.extractTempDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
            break;
        }case RIGHT_INFO_SOAK:{
            if(mPageEditType == HOME_PAGE_FAV_EXT){
                mArcRight->setMin(mCacheData.soakingTimeDef-1);
                mArcRight->setMax(mCacheData.soakingTimeDef);
            }else{
                mArcRight->setMin(mMasEspData.soakingTimeMin);
                mArcRight->setMax(mMasEspData.soakingTimeMax);
            }
            mArcRight->setProgress(mCacheData.soakingTimeDef);
            mRightInfoTv->setText(std::to_string(mCacheData.soakingTimeDef));
            break;
        }
    }
}

void HomeTabModeMasEsp::onFavModeEditListener(int Position, bool isEdit){
    mPageEditType = isEdit?HOME_PAGE_FAV_EDIT:HOME_PAGE_FAV_EXT;
    mFavEditPos = Position;

    Json::Value sndData = g_objConf->getFavModeList()[Position]["sndModeData"];
    mCacheData.powderDef = getJsonInt(sndData,"powderDef");
    mCacheData.extractTempDef = getJsonInt(sndData,"extractTempDef");
    mCacheData.soakingTimeDef = getJsonInt(sndData,"soakingTimeDef");
    mCacheData.stepDataList.clear();
    for(auto & stepItemJsonData: sndData["stepDataList"]){
        mCacheData.stepDataList.push_back({getJsonInt(stepItemJsonData,"pressure"),
                                            getJsonInt(stepItemJsonData,"water")});
    }
    LOGE("mCacheData.stepDataList size = %d mCacheData.powderDef = %d",mCacheData.stepDataList.size(),mCacheData.powderDef);
    mPlotInfoModePicker->setVisibility(View::INVISIBLE);

    mPlotInfoFavImg->setVisibility(View::GONE);
    mPlotInfoEditImg->setVisibility(View::GONE);

    mInfoRVPicker->setMaxValue(masEspRightInfoList.size()-1);
    mInfoRVPicker->setFormatter([this](int value){ 
        switch(masEspRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_TEMP:{ return masEspRightInfoList.at(value).name + std::to_string(mCacheData.extractTempDef) + "℃"; break;}
            case RIGHT_INFO_SOAK:{ return masEspRightInfoList.at(value).name + std::to_string(mCacheData.soakingTimeDef) + "s";break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setSelectFormatter([this](int value){ 
        switch(masEspRightInfoList.at(value).type){
            case RIGHT_INFO_EXT_TEMP:{ return masEspRightInfoList.at(value).name; break;}
            case RIGHT_INFO_SOAK:{ return masEspRightInfoList.at(value).name;break;}   
        }
        return std::string("error");
    });
    mInfoRVPicker->setValue(1);

    ((RelativeLayout::LayoutParams *)mPlotInfoPlotView->getLayoutParams())->setMargins(55,0,0,0);
    if(isEdit){
        mArcPowder->setShowSlider(true);
        mArcPowder->setMin(mMasEspData.powderMin);
        mArcPowder->setMax(mMasEspData.powderMax);

    }else{
        mArcPowder->setMin(mCacheData.powderDef-1);
        mArcPowder->setMax(mCacheData.powderDef);
        mArcPowder->setShowSlider(false);
        mArcRight->setShowSlider(false);
    }

    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));

    updateRightInfoGroup(mInfoRVPicker->getValue());
    // 初始化 实时折线图
    masEspPlotHistoryDataList.clear(); // 清空历史数据
    mPlotRealObj->clearPoints();    // 先清空所有数据
    mPlotRealObj->addPoint(0,0);
    setPlotViewData();
}

void HomeTabModeMasEsp::dealRightInfoAddReductClick(bool isAdd){
    int infoGear;
    switch(masEspRightInfoList.at(mInfoRVPicker->getValue()).type){
        case RIGHT_INFO_EXT_TEMP:{
            infoGear = mMasEspData.extractTempGear;
            break;
        }case RIGHT_INFO_SOAK:{
            infoGear = mMasEspData.soakingTimeGear;
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

void HomeTabModeMasEsp::onItemClickListener(View &v){
    switch(v.getId()){
        case t5_ai_coffee::R::id::right_info_reduce:{
            dealRightInfoAddReductClick(false);
            break;
        }case t5_ai_coffee::R::id::right_info_add:{
            dealRightInfoAddReductClick(true);
            break;
        }case t5_ai_coffee::R::id::info_plot_favorites:{
            v.setActivated(!v.isActivated());

            mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()) = mCacheData;
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
        }case t5_ai_coffee::R::id::fav_enter_img:{
            Json::Value favItemData = g_objConf->getFavModeList()[mFavEditPos];
            Json::Value favSndItemData = favItemData["sndModeData"];
            Json::Value favSndStepItemData;

            mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()) = mCacheData;
            favSndItemData["sndModename"] = mCacheData.sndModename;
            favSndItemData["powderDef"] = mCacheData.powderDef;
            favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
            favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

            // for(auto & stepItemData: mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).stepDataList){
            //     favSndStepItemData["pressure"] = stepItemData.pressure;
            //     favSndStepItemData["time"] = stepItemData.time;
            //     favSndItemData["stepDataList"].append(favSndStepItemData);
            // }
            favItemData["sndModeData"] = favSndItemData;

            g_objConf->editFavModeData(favItemData,mFavEditPos);
            break;
        }
    }
}


void HomeTabModeMasEsp::dealPowderItemClick(View &v){
    if(!v.isActivated()) return;

    LOGE("%d %d",v.isSelected(),v.isActivated());
    // 不管单双，共同的操作
    switch(v.getId()){
        case t5_ai_coffee::R::id::icon_one_bean:
        case t5_ai_coffee::R::id::icon_double_bean:{
            if(v.isSelected()){
                v.setSelected(false);
                mOneCupImg->setActivated(true);
            }else if(v.isActivated()){
                v.setSelected(true);
                mOneCupImg->setActivated(false);
                mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).powderDef = mCacheData.powderDef;
            }
            break;
        }case t5_ai_coffee::R::id::icon_one_cup:
        case t5_ai_coffee::R::id::icon_double_cup:{
            if(v.isSelected()){
                v.setSelected(false);
                mOneBeanImg->setActivated(true);
                masEspPlotHistoryDataList.clear(); // 清空历史数据
                setPlotViewData();
            }else if(v.isActivated()){
                v.setSelected(true);
                mOneBeanImg->setActivated(false);
                mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).extractTempDef = mCacheData.extractTempDef;
                mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).soakingTimeDef = mCacheData.soakingTimeDef;
            }
            mPlotInfoEditImg->setAlpha(v.isSelected()?0.4f:1.f);
            mPlotInfoEditImg->setEnabled(!v.isSelected());
            mPlotInfoModePicker->setAlpha(v.isSelected()?0.4f:1.f);
            mPlotInfoModePicker->setEnabled(!v.isSelected());

            int extMasEspTotal = g_objConf->getMasEspTotal()+1;
            // 涂鸦状态上报 
            bool isStart = v.isSelected();
            g_appData.eqStatus = v.isSelected()?ES_EXTRACTING:(!(g_appData.machineState & MC_STATE_EXT))?ES_EXTRACT_DONE:ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            // 启动/暂停 -----------
            g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
            if(v.isSelected()){
                int extType = MAKE_TYPE_COFFEE;
                int coffeeMode = mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).sndModeType;
                // 制作大类型 -----------
                g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); 
                g_tuyaOsMgr->reportDpData(TYCMD_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
                // 咖啡DIY ----------
                g_tuyaOsMgr->reportCoffeeDiyRawData(COFFEE_TYPE_MAS, mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).sndModeType, 
                                                    mCacheData.extractTempDef,0,mCacheData.soakingTimeDef,0,0,
                                                    mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).stepDataList); 
                LOGE("sndModeType = %d",mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).sndModeType);
            }else{
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_MASTER,PROP_VALUE, &extMasEspTotal); // 测试浓缩咖啡萃取次数
            }
            // 工作状态 ------------
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
#endif

            if(v.isSelected())  g_appData.machineState |= MC_STATE_EXT;
            else{
                // 若已经是非工作状态，则不需要下发萃取指令
                if(!(g_appData.machineState & MC_STATE_EXT)){
                    g_objConf->setMasEspTotal(extMasEspTotal);
                    return;
                }
                g_appData.machineState &= ~MC_STATE_EXT;
            }
            g_objConnMgr->setMasEsp(v.isSelected(),mCacheData.extractTempDef,mCacheData.soakingTimeDef,mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).stepDataList);
            break;
        }
    }
}

void HomeTabModeMasEsp::dealTuyaStartWork(){
    int Mode = g_appData.tuyaDiyData[1];

    for(int i=0; i<mMasEspData.sndModeList.size(); i++){
        if(mMasEspData.sndModeList.at(i).sndModeType == Mode){
            mPlotInfoModePicker->setValue(i);
            break;
        }
    }
    mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).extractTempDef = g_appData.tuyaDiyData[2];
    mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).soakingTimeDef = g_appData.tuyaDiyData[4];
    std::vector<MasEspStepDataStr> &stepData = mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue()).stepDataList;
    stepData.clear();
    for(int i=0;i<5;i++){
        if(g_appData.tuyaDiyData[8+i*2] >= 1){
            stepData.push_back({g_appData.tuyaDiyData[7+i*2],g_appData.tuyaDiyData[8+i*2]});
            LOGV("pressure = %d  water = %d",g_appData.tuyaDiyData[7+i*2],g_appData.tuyaDiyData[8+i*2]);
        }else{
            break;
        }
    }
    LOGV("temp = %d water = %d soaktime = %d stepData.size = %d",g_appData.tuyaDiyData[2],g_appData.tuyaDiyData[3],g_appData.tuyaDiyData[4],stepData.size());
    g_objConf->setMasEspData(mMasEspData);
}

void HomeTabModeMasEsp::onArcValueChangeListener(View &v, int progress, bool fromUser){
    if(!fromUser) return;
    switch(v.getId()){
        case t5_ai_coffee::R::id::arc_powder:{
            mPowderInfoTv->setText(std::to_string(progress));
            mCacheData.powderDef = progress;
            break;
        }
    case t5_ai_coffee::R::id::arc_right:{
            mRightInfoTv->setText(std::to_string(progress));
            switch(masEspRightInfoList.at(mInfoRVPicker->getValue()).type){
                case RIGHT_INFO_EXT_TEMP:{
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
    
    mPlotInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
    
}

void HomeTabModeMasEsp::onHorPickerChange(){
    mCacheData = mMasEspData.sndModeList.at(mPlotInfoModePicker->getValue());
     // 初始化 实时折线图
    masEspPlotHistoryDataList.clear(); // 清空历史数据
    mPlotRealObj->clearPoints();    // 先清空所有数据
    mPlotRealObj->addPoint(0,0);
    setPlotViewData();
    // updateGroupData();

    mInfoRVPicker->notifyUpdate();
    mInfoRVPicker->setValue(1,true);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    mPlotInfoFavImg->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));

    switch(masEspRightInfoList.at(mInfoRVPicker->getValue()).type){
        case RIGHT_INFO_EXT_TEMP:{
            mArcRight->setProgress(mCacheData.extractTempDef);
            mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
            break;
        }case RIGHT_INFO_SOAK:{
            mArcRight->setProgress(mCacheData.soakingTimeDef);
            mRightInfoTv->setText(std::to_string(mCacheData.soakingTimeDef));
            break;
        }
    }
}

// 初始化 折线图
void HomeTabModeMasEsp::initPlotViewData(){
    // 是否显示辅助线
    mPlotInfoPlotView->setShowGrid(true);
    // 设置 坐标轴颜色（包括文本）
    mPlotInfoPlotView->setForegroundColor(0x33FFFFFF);
    // 设置 辅助线颜色
    mPlotInfoPlotView->setGridColor(0x33FFFFFF);
    
    // 辅助线
    mPlotInfoPlotView->axis(CYPlotView::LeftAxis)->setTickmarkVisible(true);

    mPlotInfoPlotView->axis(CYPlotView::RightAxis)->setTickmarkVisible(false);
    mPlotInfoPlotView->axis(CYPlotView::TopAxis)->setTickmarkVisible(false);
    mPlotInfoPlotView->axis(CYPlotView::BottomAxis)->setTickmarkVisible(false);

    // 坐标轴
    mPlotInfoPlotView->axis(CYPlotView::LeftAxis)->setVisible(false);
    mPlotInfoPlotView->axis(CYPlotView::RightAxis)->setVisible(false);
    mPlotInfoPlotView->axis(CYPlotView::TopAxis)->setVisible(false);
    mPlotInfoPlotView->axis(CYPlotView::BottomAxis)->setVisible(false);
    
    // 坐标轴 信息
    mPlotInfoPlotView->axis(CYPlotView::LeftAxis)->setTickLabelsShown(true);
    mPlotInfoPlotView->axis(CYPlotView::BottomAxis)->setTickLabelsShown(true);

    mPlotInfoPlotView->axis(CYPlotView::TopAxis)->setTickLabelsShown(false);
    mPlotInfoPlotView->axis(CYPlotView::RightAxis)->setTickLabelsShown(false);

    mPlotInfoPlotView->axis(CYPlotView::LeftAxis)->setLabel(std::string("bar"));  // 自己修改过源码从而达到效果
    mPlotInfoPlotView->axis(CYPlotView::BottomAxis)->setLabel(std::string("ml"));  // 自己修改过源码从而达到效果
    

    // 设置数据点，包括属性
    mPlotObj = new CYPlotObject(Color::BLUE, CYPlotObject::Lines, 10, CYPlotObject::Pentagon);
    mPlotRealObj = new CYPlotObject(Color::BLUE, CYPlotObject::Lines, 10, CYPlotObject::Pentagon);

    Cairo::RefPtr<Cairo::LinearGradient> patternColor = Cairo::LinearGradient::create(0,320,0,0);
    patternColor->add_color_stop_rgb(0,0.310,0.035,0.553);
    patternColor->add_color_stop_rgb(1,0.694,0.318,0.565);
    
    Color clr(0xFFACB6E5);
    Color clr2(0xFFD66E18);
    RefPtr<Pattern> labelColor = Cairo::SolidPattern::create_rgba(clr.red(),clr.green(),clr.blue(),clr.alpha());
    RefPtr<Pattern> labelColor2 = Cairo::SolidPattern::create_rgba(clr2.red(),clr2.green(),clr2.blue(),clr2.alpha());
    // 柱状图 生效的属性
    mPlotObj->setLabelPen(labelColor);   // 设置 柱状 label文本的颜色
    mPlotObj->setBarBrush(patternColor); // 设置 柱状 的内部颜色
    mPlotObj->setBarPen(patternColor);   // 设置 柱状 的边框颜色

    //线条 生效的属性
    mPlotObj->setLinePen(labelColor); // 设置 线状 颜色
    mPlotObj->setLineWidth(3);        // 设置线条宽度

    mPlotRealObj->setLabelPen(labelColor);   // 设置 柱状 label文本的颜色
    mPlotRealObj->setLinePen(labelColor2); // 设置 线状 颜色
    mPlotRealObj->setLineWidth(3);
    mPlotRealObj->setDashedLine(8,8);       // 设置线条为虚线

    mPlotInfoPlotView->addPlotObject(mPlotObj);
    mPlotInfoPlotView->addPlotObject(mPlotRealObj);
}

// 设置折线图的数据
void HomeTabModeMasEsp::setPlotViewData(){
    // 数据点
    mPlotObj->clearPoints();    // 先清空所有数据
    // mPlotRealObj->clearPoints();

    std::vector<MasEspStepDataStr> stepList = mCacheData.stepDataList;
    mPlotObj->addPoint(0,0);
    // 自定义规则，每秒增长2bar
    float slope = 2.0;  // 斜率
    PointF PrePoint({0,0}); // 上一个点

    // 这个模拟斜率的计算还有点问题...可以让gpt给你写一段再优化优化。
    for(int i=0; i<stepList.size(); i++){
        LOGI("i = %d  (%d,%d)",i,stepList.at(i).water,stepList.at(i).pressure);
        // 计算虚拟点 模拟压力缓慢增加效果
        PointF TarPoint({PrePoint.x+stepList.at(i).water,(float)stepList.at(i).pressure}); // 目标点

        int offsetY = TarPoint.y-PrePoint.y; // 相差的压力值（Y）
        double VirPointOffsetX  = std::abs(offsetY)/slope;  // 虚拟点的时间相差值（X）
        double VirPointOffsetY = (TarPoint.x-PrePoint.x)*slope*(offsetY==0?0:(offsetY>0?1:-1)); // 虚拟点的压力相差值（Y）

        if(VirPointOffsetX < (TarPoint.x-PrePoint.x)){
            // 在斜率 slope 以内
            mPlotObj->addPoint(PrePoint.x + VirPointOffsetX,TarPoint.y);
            mPlotObj->addPoint(TarPoint.x,TarPoint.y);
        }else{   
            // 超过了斜率 slope
            mPlotObj->addPoint(TarPoint.x,PrePoint.y+VirPointOffsetY);
        }
        PrePoint = TarPoint;
    }

    // 设置坐标轴 信息的数据范围
    if(stepList.size() > 0)mPlotInfoPlotView->setLimits(0,PrePoint.x, 0, 20);

    // mPlotInfoPlotView->invalidate();
}

Json::Value HomeTabModeMasEsp::getCacheDataToJson(){
    Json::Value favItemData;
    Json::Value favSndItemData;
    Json::Value favSndStepItemData;
    favSndItemData["sndModename"] = mCacheData.sndModename;
    favSndItemData["powderDef"] = mCacheData.powderDef;
    favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
    favSndItemData["soakingTimeDef"] = mCacheData.soakingTimeDef;

    for(auto & stepItemData: mCacheData.stepDataList){
        favSndStepItemData["pressure"] = stepItemData.pressure;
        favSndStepItemData["water"] = stepItemData.water;
        favSndItemData["stepDataList"].append(favSndStepItemData);
    }
    favItemData["coffeeMode"] = HOME_MT_MASTER_ESPRESSO;
    favItemData["name"] = "测试浓缩";
    favItemData["beanGrindMode"] = g_objConf->getBeanGrindMode();
    favItemData["sndModeData"] = favSndItemData;

    return favItemData;
}