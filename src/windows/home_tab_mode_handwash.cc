#include "home_tab_mode_handwash.h"
#include "conf_mgr.h"
#include "wind_mgr.h"
#include "gaussfilterdrawable.h"
#include "conn_mgr.h"
#include "tuya_os_mgr.h"
HomeTabModeHandWash::HomeTabModeHandWash(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){
    mHorSelectPos = 0;
    mHandWashData = g_objConf->getHandWashData();
    mPageEditType  = HOME_PAGE_NORMAL;
    initGroup();
}

HomeTabModeHandWash::~HomeTabModeHandWash(){

}

void HomeTabModeHandWash::initGroup(){
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

    mHomeFavGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::hand_wash_fav_img);
    mHomeTipsGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::home_tips_group);
    mHomeTipsHandWashGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::hand_wash_tips_group);
    mHomeTipsTeaTv = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::home_tips_tea_tv);

    mHomeTipsWater = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::hand_wash_tips_water_tv);
    mHomeTipsTime = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::hand_wash_tips_time_tv);
    // 特定控件
    mHorModePicker = (NumberPicker *)mWindPageBox->findViewById(t5_ai_coffee::R::id::info_form_hor_picker);
    
    mFormGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_layout);
    mFormTitleGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_info_title);
    mFormStepGroup_0 = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_info_step_heating);
    mFormStepTitle_0 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_heating_title);
    mFormStepWater_0 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_heating_water);
    mFormStepSpeed_0 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_heating_speed);
    mFormStepTime_0 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_heating_time);

    mFormStepGroup_1 = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_info_step_1);
    mFormStepTitle_1 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_1_title);
    mFormStepWater_1 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_1_water);
    mFormStepSpeed_1 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_1_speed);
    mFormStepTime_1 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_1_time);

    mFormStepGroup_2 = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_info_step_2);
    mFormStepTitle_2 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_2_title);
    mFormStepWater_2 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_2_water);
    mFormStepSpeed_2 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_2_speed);
    mFormStepTime_2 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_2_time);

    mFormStepGroup_3 = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_info_step_3);
    mFormStepTitle_3 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_3_title);
    mFormStepWater_3 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_3_water);
    mFormStepSpeed_3 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_3_speed);
    mFormStepTime_3 = (TextView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::step_3_time);
    
    mFormStepAddImg = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::form_step_add);

}

void HomeTabModeHandWash::updateGroupData(){
    mPageEditType = HOME_PAGE_NORMAL;
    mCacheData = mHandWashData.sndModeList.at(mHorSelectPos);
    mHomeFavGroup->setVisibility(View::VISIBLE);
    mHorModePicker->setVisibility(View::VISIBLE);
    mHorModePicker->setMaxValue(mHandWashData.sndModeList.size()-1);
    mHorModePicker->setFormatter([this](int value){ return value>= mHandWashData.sndModeList.size()?"error formatter":mHandWashData.sndModeList.at(value).sndModename; });
    mHorModePicker->setValue(mHorSelectPos);
    mHorModePicker->getLayoutParams()->width = 650;

    mArcPowder->setShowSlider(true);
    mArcPowder->setMin(mHandWashData.powderMin);
    mArcPowder->setMax(mHandWashData.powderMax);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));

    mHomeTipsHandWashGroup->setVisibility(View::VISIBLE);
    mHomeTipsTeaTv->setVisibility(View::GONE);
    mHomeTipsGroup->getLayoutParams()->height = 107;

    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));

    mOneBeanImg->setVisibility(View::VISIBLE);
    mDoubleBeanImg->setVisibility(View::GONE);
    mOneCupImg->setVisibility(View::VISIBLE);
    mDoubleCupImg->setVisibility(View::GONE);

    mOneBeanImg->setActivated(true);
    updateBeanCupImgStatus();
    
    updateRightInfoGroup();
    setFormData();
}

void HomeTabModeHandWash::updateRightInfoGroup(){
    mRightInfoTitleTv->setText("萃取温度");
    mRightInfoUnitsTv->setText("℃");
    if(mPageEditType == HOME_PAGE_FAV_EXT){
        mArcRight->setMin(mCacheData.extractTempDef-1);
        mArcRight->setMax(mCacheData.extractTempDef);
    }else{
        mArcRight->setMin(mHandWashData.extractTempMin);
        mArcRight->setMax(mHandWashData.extractTempMax);
    }
    mArcRight->setProgress(mCacheData.extractTempDef);
    mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
}

void HomeTabModeHandWash::updatePageData(){
    if(g_appData.statusChangeFlag & CS_PRE_HEATING){
        if(mPageEditType != HOME_PAGE_FAV_EDIT)updateBeanCupImgStatus();
    }
}

void HomeTabModeHandWash::updateBeanCupImgStatus(){
    if(g_appData.coffeePreheatPtc >= 100){
        if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)) 
            mOneCupImg->setActivated(true);
    }else{
        mOneCupImg->setActivated(false);
    }
}

void HomeTabModeHandWash::onFavModeEditListener(int Position, bool isEdit){
    mPageEditType = isEdit?HOME_PAGE_FAV_EDIT:HOME_PAGE_FAV_EXT;
    mFavEditPos = Position;

    Json::Value sndData = g_objConf->getFavModeList()[Position]["sndModeData"];
    mCacheData.powderDef = getJsonInt(sndData,"powderDef");
    mCacheData.extractTempDef = getJsonInt(sndData,"extractTempDef");

    mCacheData.stepDataList.clear();
    for(auto & stepItemJsonData: sndData["stepDataList"]){
        mCacheData.stepDataList.push_back({getJsonInt(stepItemJsonData,"water"),
                                            getJsonInt(stepItemJsonData,"flowRate"),
                                            getJsonInt(stepItemJsonData,"interTime")});
    }
    LOGE("mCacheData.stepDataList size = %d mCacheData.powderDef = %d",mCacheData.stepDataList.size(),mCacheData.powderDef);
    mHorModePicker->setVisibility(View::INVISIBLE);

    mHomeTipsHandWashGroup->setVisibility(View::VISIBLE);
    mHomeTipsTeaTv->setVisibility(View::GONE);
    mHomeTipsGroup->getLayoutParams()->height = 107;

    mHomeFavGroup->setVisibility(View::GONE);

    if(isEdit){
        mArcPowder->setShowSlider(true);
        mArcPowder->setMin(mHandWashData.powderMin);
        mArcPowder->setMax(mHandWashData.powderMax);

    }else{
        mArcPowder->setMin(mCacheData.powderDef-1);
        mArcPowder->setMax(mCacheData.powderDef);
        mArcPowder->setShowSlider(false);
        mArcRight->setShowSlider(false);

    }
    updateRightInfoGroup();
    setFormData();
    LOGE("mCacheData.stepDataList size = %d mCacheData.powderDef = %d",mCacheData.stepDataList.size(),mCacheData.powderDef);
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    LOGE("mCacheData.stepDataList size = %d mCacheData.powderDef = %d",mCacheData.stepDataList.size(),mCacheData.powderDef);
}

// 显示 测试2咖啡 的修改弹窗页面
void HomeTabModeHandWash::showHandWashPop(int selectStep){
    if((mPageEditType == HOME_PAGE_NORMAL) && (g_appData.machineState == MC_STATE_NONE)){
        g_windMgr->showSndStepPopPage(POP_TYPE_HAND_WASH,mWindPageBox,mHorModePicker->getValue(),selectStep,
            [this](int selectPos){
                mHandWashData = g_objConf->getHandWashData();
                if(mHorModePicker->getValue() != selectPos){
                    mHorSelectPos = selectPos;
                    mHorModePicker->setValue(selectPos);
                    onHorPickerChange();
                }else{
                    mHorSelectPos = mHorModePicker->getValue();
                    mCacheData.stepDataList = mHandWashData.sndModeList.at(mHorSelectPos).stepDataList;
                    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
                    setFormData(); 
                }
                
            },[this](int selectPos){
                if((mHorSelectPos != selectPos)){
                    mHorSelectPos = selectPos;
                    mHorModePicker->setValue(selectPos);
                    onHorPickerChange();
                }
            });
    }
}

void HomeTabModeHandWash::dealRightInfoAddReductClick(bool isAdd){
    int infoGear = mHandWashData.extractTempGear;
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

void HomeTabModeHandWash::onItemClickListener(View &v){
    switch(v.getId()){
        case t5_ai_coffee::R::id::right_info_reduce:{
            dealRightInfoAddReductClick(false);
            break;
        }case t5_ai_coffee::R::id::right_info_add:{
            dealRightInfoAddReductClick(true);
            break;
        }case t5_ai_coffee::R::id::powder_add:
        case t5_ai_coffee::R::id::powder_reduce:{
            mCacheData.powderDef = mArcPowder->getProgress();
            break;
        }case t5_ai_coffee::R::id::form_info_step_heating:{
            showHandWashPop(0);
            break;
        }case t5_ai_coffee::R::id::form_info_step_1:{
            showHandWashPop(1);
            break;
        }case t5_ai_coffee::R::id::form_info_step_2:{
            showHandWashPop(2);
            break;
        }case t5_ai_coffee::R::id::form_info_step_3:{
            showHandWashPop(3);
            break;
        }case t5_ai_coffee::R::id::form_step_add:{
            showHandWashPop(mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList.size());
            break;
        }case t5_ai_coffee::R::id::hand_wash_fav_img:{
            v.setActivated(!v.isActivated());

            mHandWashData.sndModeList.at(mHorModePicker->getValue()) = mCacheData;
            Json::Value favItemData = getCacheDataToJson();

            if(v.isActivated()){
                g_objConf->addFavModeData(favItemData);
            }else{
                g_objConf->deleteFavModeData(favItemData);
            }
            break;
        }case t5_ai_coffee::R::id::hand_wash_tips_img:{
            if(mHomeTipsGroup->getVisibility() != View::VISIBLE){
                int sumWater = 0,sumTime = 0;
                std::vector<FormStepDataStr> stepData = mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList;
                for(int i=0; i<stepData.size(); i++){
                    sumWater += stepData.at(i).water;
                    sumTime  += stepData.at(i).interTime;
                }
                mHomeTipsWater->setText(std::to_string(sumWater)+"ml");
                mHomeTipsTime->setText(std::to_string(sumTime)+"s");
                GaussFilterDrawable *bgDrawable = new GaussFilterDrawable(mWindPageBox,Rect::Make(1340,215,265,107),30,2,0x33bda279);
                bgDrawable->setPadding(0,0,14,0);
                bgDrawable->setCornerRadii(20,20,-20,20);
                bgDrawable->setGaussColorDev(0.8);
                bgDrawable->setGaussColorNoiseRange(-1,1);
                mHomeTipsGroup->setVisibility(View::VISIBLE);
                mHomeTipsGroup->setBackground(bgDrawable);
            }else{
                mHomeTipsGroup->setVisibility(View::GONE);
            }
            
            break;
        }case t5_ai_coffee::R::id::icon_one_bean:
        case t5_ai_coffee::R::id::icon_one_cup:{
            dealPowderItemClick(v);
            break;
        }case t5_ai_coffee::R::id::fav_enter_img:{
            Json::Value favItemData = g_objConf->getFavModeList()[mFavEditPos];
            Json::Value favSndItemData = favItemData["sndModeData"];
            Json::Value favSndStepItemData;
            favSndItemData["powderDef"] = mArcPowder->getProgress();
            favSndItemData["extractTempDef"] = mCacheData.extractTempDef;

            // for(auto & stepItemData: mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList){
            //     favSndStepItemData["water"] = stepItemData.water;
            //     favSndStepItemData["flowRate"] = stepItemData.flowRate;
            //     favSndStepItemData["interTime"] = stepItemData.interTime;
            //     favSndItemData["stepDataList"].append(favSndStepItemData); 
            // }

            favItemData["sndModeData"] = favSndItemData;
            g_objConf->editFavModeData(favItemData,mFavEditPos);

            break;
        }
    }
}


void HomeTabModeHandWash::dealPowderItemClick(View &v){
    if(!v.isActivated()) return;

    LOGE("%d %d",v.isSelected(),v.isActivated());
    // 不管单双，共同的操作
    switch(v.getId()){
        case t5_ai_coffee::R::id::icon_one_bean:{
            if(v.isSelected()){
                v.setSelected(false);
                if(g_appData.coffeePreheatPtc >= 100)mOneCupImg->setActivated(true);
            }else if(v.isActivated()){
                v.setSelected(true);
                if(g_appData.coffeePreheatPtc >= 100)mOneCupImg->setActivated(false);
                mHandWashData.sndModeList.at(mHorModePicker->getValue()).powderDef = mCacheData.powderDef;
            }
            break;
        }case t5_ai_coffee::R::id::icon_one_cup:{
            if(v.isSelected()){
                v.setSelected(false);
                mOneBeanImg->setActivated(true);
                if(mCacheData.stepDataList.size() >= 4 || mPageEditType != HOME_PAGE_NORMAL)    mFormStepAddImg->setVisibility(View::GONE);
                else                                                                            mFormStepAddImg->setVisibility(View::VISIBLE);
            }else if(v.isActivated()){
                v.setSelected(true);
                mOneBeanImg->setActivated(false);
                mFormStepAddImg->setVisibility(View::GONE);
            }
            mHorModePicker->setEnabled(!v.isSelected());
            mHorModePicker->setAlpha(v.isSelected()?0.4f:1.f);
            mFormGroup->setAlpha(v.isSelected()?0.4f:1.f);
            mFormTitleGroup->setAlpha(v.isSelected()?0.4f:1.f);

            int extHandWashTotal = g_objConf->getHandWashTotal()+1;
            // 涂鸦状态上报 
            bool isStart = v.isSelected();
            g_appData.eqStatus = v.isSelected()?ES_EXTRACTING:(!(g_appData.machineState & MC_STATE_EXT))?ES_EXTRACT_DONE:ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            // 启动/暂停 -----------
            g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
            if(v.isSelected()){
                int extType = MAKE_TYPE_COFFEE;
                int coffeeMode = mHandWashData.sndModeList.at(mHorModePicker->getValue()).sndModeType;
                // 制作大类型 -----------
                g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); 
                g_tuyaOsMgr->reportDpData(TYCMD_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
                // 咖啡DIY ----------
                g_tuyaOsMgr->reportCoffeeDiyRawData(COFFEE_TYPE_HANDWASH, mHandWashData.sndModeList.at(mHorModePicker->getValue()).sndModeType, 
                                                    mCacheData.extractTempDef,0,0,0,0,{},
                                                    mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList); 
                LOGE("sndModeType = %d",mHandWashData.sndModeList.at(mHorModePicker->getValue()).sndModeType);
            }else{
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_POUR,PROP_VALUE, &extHandWashTotal); // 测试2咖啡萃取次数
            }
            // 工作状态 ------------
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
#endif
            if(v.isSelected())  g_appData.machineState |= MC_STATE_EXT;
            else{
                // 若已经是非工作状态，则不需要下发萃取指令
                if(!(g_appData.machineState & MC_STATE_EXT)){
                    g_objConf->setHandWashTotal(extHandWashTotal);
                    return;
                }
                g_appData.machineState &= ~MC_STATE_EXT;
            }
            mHandWashData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = mCacheData.extractTempDef;
            g_objConnMgr->setHandWash(v.isSelected(),mCacheData.extractTempDef,mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList);
            break;
        }
    }
}

void HomeTabModeHandWash::dealTuyaStartWork(){
    int Mode = g_appData.tuyaDiyData[1];
    

    for(int i=0; i<mHandWashData.sndModeList.size(); i++){
        if(mHandWashData.sndModeList.at(i).sndModeType == Mode){
            mHorSelectPos = i;
            mHorModePicker->setValue(mHorSelectPos);
            break;
        }
    }
    mHandWashData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = g_appData.tuyaDiyData[2];
    std::vector<FormStepDataStr> &stepData = mHandWashData.sndModeList.at(mHorModePicker->getValue()).stepDataList;
    stepData.clear();
    for(int i=0;i<4;i++){
        if(g_appData.tuyaDiyData[7+i*3] >= 10){
            stepData.push_back({g_appData.tuyaDiyData[7+i*3],g_appData.tuyaDiyData[8+i*3],g_appData.tuyaDiyData[9+i*3]});
        }else{
            break;
        }
    }
    g_objConf->setHandWashData(mHandWashData);
}

void HomeTabModeHandWash::onArcValueChangeListener(View &v, int progress, bool fromUser){
    if(!fromUser) return;
    switch(v.getId()){
        case t5_ai_coffee::R::id::arc_powder:{
            mPowderInfoTv->setText(std::to_string(progress));
            mCacheData.powderDef = progress;
            break;
        }
        case t5_ai_coffee::R::id::arc_right:{
            mRightInfoTv->setText(std::to_string(progress));
            mCacheData.extractTempDef = progress;
            break;
        }
    }
    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
}

// 横向picker的变化
void HomeTabModeHandWash::onHorPickerChange(){
    mCacheData = mHandWashData.sndModeList.at(mHorModePicker->getValue());
    mHorSelectPos = mHorModePicker->getValue();
    mArcPowder->setProgress(mCacheData.powderDef);
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    
    mArcRight->setProgress(mCacheData.extractTempDef);
    mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
    setFormData();   
}


// 设置表格数据
void HomeTabModeHandWash::setFormData(){
    TextView *stepViewWater,*stepViewSpeed,*stepViewTime;
    std::string flowRateTxt;
    mFormStepGroup_0->setVisibility(View::GONE);
    mFormStepGroup_1->setVisibility(View::GONE);
    mFormStepGroup_2->setVisibility(View::GONE);
    mFormStepGroup_3->setVisibility(View::GONE);
    std::vector<FormStepDataStr> stepData;
    stepData = mCacheData.stepDataList;
    mFormStepTitle_0->setText("");
    mFormStepTitle_1->setText("1");
    mFormStepTitle_2->setText("2");
    mFormStepTitle_0->setCompoundDrawablesWithIntrinsicBounds("@mipmap/form_heating","nullptr","nullptr","nullptr");
    mFormStepTitle_1->setCompoundDrawablesWithIntrinsicBounds("@mipmap/form_water","nullptr","nullptr","nullptr");
    mFormStepTitle_2->setCompoundDrawablesWithIntrinsicBounds("@mipmap/form_water","nullptr","nullptr","nullptr");
    mFormStepTitle_1->setCompoundDrawablePadding(-15);
    mFormStepTitle_2->setCompoundDrawablePadding(-15);
    mFormStepTitle_3->setCompoundDrawablePadding(-15);
    mFormStepTitle_0->setPadding(23,0,0,0);
    mFormStepTitle_1->setPadding(20,0,0,0);
    mFormStepTitle_2->setPadding(20,0,0,0);
    mFormStepTitle_3->setPadding(20,0,0,0);
    mFormStepTitle_0->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepTitle_1->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepTitle_2->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepTitle_3->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepGroup_0->setAlpha(1.f);
    mFormStepGroup_1->setAlpha(1.f);
    mFormStepGroup_2->setAlpha(1.f);
    for(int i=0; i<stepData.size(); i++){
        switch(i){
            case 0:{
                mFormStepGroup_0->setVisibility(View::VISIBLE);
                stepViewWater = mFormStepWater_0;
                stepViewSpeed = mFormStepSpeed_0;
                stepViewTime  = mFormStepTime_0;
                break;
            }case 1:{
                mFormStepGroup_1->setVisibility(View::VISIBLE);
                stepViewWater = mFormStepWater_1;
                stepViewSpeed = mFormStepSpeed_1;
                stepViewTime  = mFormStepTime_1;
                break;
            }case 2:{
                mFormStepGroup_2->setVisibility(View::VISIBLE);
                stepViewWater = mFormStepWater_2;
                stepViewSpeed = mFormStepSpeed_2;
                stepViewTime  = mFormStepTime_2;
                break;
            }case 3:{
                mFormStepGroup_3->setVisibility(View::VISIBLE);
                stepViewWater = mFormStepWater_3;
                stepViewSpeed = mFormStepSpeed_3;
                stepViewTime  = mFormStepTime_3;
                break;
            }
        }
        switch(stepData.at(i).flowRate){
            case 0: { flowRateTxt = "慢";break;}
            case 1: { flowRateTxt = "中";break;}
            case 2: { flowRateTxt = "快";break;}
        }
        stepViewWater->setText(std::to_string(stepData.at(i).water)+"ml");
        stepViewSpeed->setText(flowRateTxt);
        stepViewTime->setText(std::to_string(stepData.at(i).interTime)+"s");
 
        LOGI("water = %d  flowRateTxt = %s interTime = %d %s",stepData.at(i).water,flowRateTxt.c_str(),stepData.at(i).interTime,(i==stepData.size()-1)?"\n":"");
    }
    mFormStepAddImg->setActivated(true);
    if(stepData.size() >= 4 || mPageEditType != HOME_PAGE_NORMAL)   mFormStepAddImg->setVisibility(View::GONE);
    else                                                            mFormStepAddImg->setVisibility(View::VISIBLE);
}

Json::Value HomeTabModeHandWash::getCacheDataToJson(){
    Json::Value favItemData;
    Json::Value favSndItemData;
    Json::Value favSndStepItemData;
    favSndItemData["sndModename"] = mCacheData.sndModename;
    favSndItemData["powderDef"] = mCacheData.powderDef;
    favSndItemData["extractTempDef"] = mCacheData.extractTempDef;

    for(auto & stepItemData: mCacheData.stepDataList){
        favSndStepItemData["water"] = stepItemData.water;
        favSndStepItemData["flowRate"] = stepItemData.flowRate;
        favSndStepItemData["interTime"] = stepItemData.interTime;
        favSndItemData["stepDataList"].append(favSndStepItemData); 
    }

    favItemData["coffeeMode"] = HOME_MT_HAND_WASHED;
    favItemData["name"] = "测试2咖啡";
    favItemData["beanGrindMode"] = g_objConf->getBeanGrindMode();
    favItemData["sndModeData"] = favSndItemData;

    return favItemData;
}