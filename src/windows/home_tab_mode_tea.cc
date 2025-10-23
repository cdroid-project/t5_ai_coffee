/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:08:05
 * @FilePath: /t5_ai_demo/src/windows/home_tab_mode_tea.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "home_tab_mode_tea.h"
#include "conf_mgr.h"
#include "wind_mgr.h"
#include "gaussfilterdrawable.h"
#include "conn_mgr.h"
#include "tuya_os_mgr.h"

HomeTabModeTea::HomeTabModeTea(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){
    mHorSelectPos = 0;
    mExtTeaData = g_objConf->getExtractTeaData();
    mPageEditType  = HOME_PAGE_NORMAL;
    initGroup();
}

HomeTabModeTea::~HomeTabModeTea(){
    delete mTeaAdapter;
    delete mLinearLyManager;
    delete mItemTouchHelper;
    delete mSimpleCallback;
}

void HomeTabModeTea::initGroup(){
    // 公有控件属性
    mOneBeanImg     = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_bean);
    mDoubleBeanImg  = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_bean);
    mOneCupImg      = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_one_cup);
    mDoubleCupImg   = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_double_cup);
    mTeaRebrewImg   = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::icon_tea_rebrew);
    mFavBackImg     = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::fav_back_img);

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

    mRightInfoReduceImg = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_reduce);
    mRightInfoAddImg = (ImageView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_info_add);

    mRightAnim = (AnimatedImageDrawable *)mWindPageBox->findViewById(t5_ai_coffee::R::id::right_anim)->getBackground();
    // 特定控件
    mCoffeeGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::layout_coffee);
    mTeaListGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::layout_tea_list);
    mTeaListRecycler = (RecyclerView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::tea_list);

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
    
    mTeaAdapter = new TeaRecycAdapter(mWindPageBox);
    mSimpleCallback = new TeaSimpleCallback(mTeaAdapter);
    mLinearLyManager = new LinearLayoutManager(mWindPageBox->getContext(),RecyclerView::HORIZONTAL,false);
    mTeaAdapter->setOnCardClickListener(std::bind(&HomeTabModeTea::onTeaCardClickListener,this,std::placeholders::_1,std::placeholders::_2));
    mTeaListRecycler->setLayoutManager(mLinearLyManager);
    mTeaListRecycler->setAdapter(mTeaAdapter);
    mTeaListRecycler->addItemDecoration(new TeaItemDecoration(10,18,30));
    mItemTouchHelper = new ItemTouchHelper(mSimpleCallback);
    mItemTouchHelper->attachToRecyclerView(mTeaListRecycler);
}

void HomeTabModeTea::updateCoffeeGroupData(){
    mTeaListGroup->setVisibility(View::GONE);
    mCoffeeGroup->setVisibility(View::VISIBLE);
    mHorModePicker->setVisibility(View::VISIBLE);

    mHomeFavGroup->setVisibility(View::VISIBLE);
    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));

    mCacheData = mExtTeaData.sndModeList.at(mHorSelectPos);
    mHorModePicker->setMaxValue(mExtTeaData.sndModeList.size()-1);
    mHorModePicker->setFormatter([this](int value){ return value>= mExtTeaData.sndModeList.size()?"error formatter":mExtTeaData.sndModeList.at(value).sndModename; });
    mHorModePicker->setValue(mHorSelectPos);
    mHorModePicker->getLayoutParams()->width = 405;

    mArcPowder->setProgress(mArcPowder->getMax());
    mArcPowder->setShowSlider(false);

    mHomeTipsGroup->getLayoutParams()->height = 220;
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    updateRightInfoGroup();
    setFormData();
}

void HomeTabModeTea::updateGroupData(){
    mPageEditType  = HOME_PAGE_NORMAL;
    mFavBackImg->setVisibility(View::VISIBLE);
    mTeaListGroup->setVisibility(View::VISIBLE);
    mCoffeeGroup->setVisibility(View::GONE);

    mTeaListRecycler->scrollToPosition(0);
    mHomeTipsHandWashGroup->setVisibility(View::GONE);
    mHomeTipsTeaTv->setVisibility(View::VISIBLE);

    mOneBeanImg->setVisibility(View::GONE);
    mDoubleBeanImg->setVisibility(View::GONE);
    mOneCupImg->setVisibility(View::VISIBLE);
    mDoubleCupImg->setVisibility(View::GONE);
    mTeaRebrewImg->setVisibility(View::VISIBLE);
    mTeaRebrewImg->setActivated(true);
    updateBeanCupImgStatus();

    mExtractStep = MAKE_TEA_WASH;
}

void HomeTabModeTea::updateRightInfoGroup(){
    mRightInfoTitleTv->setText("萃取温度");
    mRightInfoUnitsTv->setText("℃");
    if(mPageEditType == HOME_PAGE_FAV_EXT){
        mArcRight->setMin(mCacheData.extractTempDef-1);
        mArcRight->setMax(mCacheData.extractTempDef);
    }else{
        mArcRight->setMin(mExtTeaData.extractTempMin);
        mArcRight->setMax(mExtTeaData.extractTempMax);
    }
    mArcRight->setProgress(mCacheData.extractTempDef);
    mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
}

void HomeTabModeTea::onFavModeEditListener(int Position, bool isEdit){
    mPageEditType = isEdit?HOME_PAGE_FAV_EDIT:HOME_PAGE_FAV_EXT;
    mFavEditPos = Position;
    

    Json::Value sndData = g_objConf->getFavModeList()[Position]["sndModeData"];
    mCacheData.powderDef = getJsonInt(sndData,"powderDef");
    mCacheData.extractTempDef = getJsonInt(sndData,"extractTempDef");
    mCacheData.pressure = getJsonInt(sndData,"pressure");

    mCacheData.washTeaWater = getJsonInt(sndData,"washTeaWater");
    mCacheData.washTeaFlowRate = getJsonInt(sndData,"washTeaFlowRate");
    mCacheData.washTeaInterTime = getJsonInt(sndData,"washTeaInterTime");
    mCacheData.soakTeaWater = getJsonInt(sndData,"soakTeaWater");
    mCacheData.soakTeaFlowRate = getJsonInt(sndData,"soakTeaFlowRate");

    mCacheData.soakTeaInterTime = getJsonInt(sndData,"soakTeaInterTime");
    mCacheData.makeTeaWater = getJsonInt(sndData,"makeTeaWater");
    mCacheData.makeTeaFlowRate = getJsonInt(sndData,"makeTeaFlowRate");
    mCacheData.makeTeaInterTime = getJsonInt(sndData,"makeTeaInterTime");

    mHorModePicker->setVisibility(View::INVISIBLE);
    mTeaRebrewImg->setVisibility(View::GONE);

    mOneBeanImg->setVisibility(View::GONE);
    mHomeTipsHandWashGroup->setVisibility(View::GONE);
    mHomeTipsTeaTv->setVisibility(View::VISIBLE);
    mHomeTipsGroup->getLayoutParams()->height = 220;

    mHomeFavGroup->setVisibility(View::GONE);

    mArcPowder->setProgress(mArcPowder->getMax());
    mArcPowder->setShowSlider(false);
    if(!isEdit){
        mArcRight->setShowSlider(false);
        mTeaRebrewImg->setVisibility(View::VISIBLE);
        mTeaRebrewImg->setActivated(true);
    }

    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));

    updateRightInfoGroup();
    setFormData();

}

void HomeTabModeTea::updatePageData(){
    if(g_appData.statusChangeFlag & CS_PRE_HEATING){
        if(mPageEditType != HOME_PAGE_FAV_EDIT)updateBeanCupImgStatus();
    }
}

void HomeTabModeTea::updateBeanCupImgStatus(){
    if(g_appData.coffeePreheatPtc >= 100){
        if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)) {
            mOneCupImg->setActivated(true);
            mTeaRebrewImg->setActivated(true);
        }
    }else{
        mOneCupImg->setActivated(false);
        mTeaRebrewImg->setActivated(false);
    }
}

// 显示 的修改弹窗页面
void HomeTabModeTea::showHandWashPop(int selectStep){
    if((mPageEditType == HOME_PAGE_NORMAL) && (g_appData.machineState == MC_STATE_NONE)){
        g_windMgr->showSndStepPopPage(POP_TYPE_TEA,mWindPageBox,mHorModePicker->getValue(),selectStep,
            [this](int selectPos){
                mExtTeaData = g_objConf->getExtractTeaData();
                if(mHorModePicker->getValue() != selectPos){
                    mHorSelectPos = selectPos;
                    mHorModePicker->setValue(selectPos);
                    onHorPickerChange();
                }else{
                    mHorSelectPos = mHorModePicker->getValue();
                    int temp =  mCacheData.extractTempDef;
                    int powder =  mCacheData.powderDef;
                    mCacheData = mExtTeaData.sndModeList.at(mHorModePicker->getValue());
                    mCacheData.extractTempDef = temp;
                    mCacheData.powderDef = powder;
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

void HomeTabModeTea::dealRightInfoAddReductClick(bool isAdd){
    int infoGear = mExtTeaData.extractTempGear;
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

void HomeTabModeTea::onTeaCardClickListener(View &v ,int Position){
    switch(v.getId()){
        case t5_ai_coffee::R::id::tea_card_Layout:{
            mExtTeaData = g_objConf->getExtractTeaData();
            if(mHorSelectPos != Position){
                mHorSelectPos = Position;
            }else{
                // 更新选中项的名字
                mHorModePicker->setMaxValue(mHorModePicker->getMaxValue());
            }
            updateCoffeeGroupData();
            break;
        }
    }
}

void HomeTabModeTea::onItemClickListener(View &v){
    switch(v.getId()){
        case t5_ai_coffee::R::id::right_info_reduce:{
            dealRightInfoAddReductClick(false);
            break;
        }case t5_ai_coffee::R::id::right_info_add:{
            dealRightInfoAddReductClick(true);
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
            showHandWashPop(0);
            break;
        }case t5_ai_coffee::R::id::hand_wash_fav_img:{
            v.setActivated(!v.isActivated());

            mExtTeaData.sndModeList.at(mHorModePicker->getValue()).powderDef = mCacheData.powderDef;
            mExtTeaData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = mCacheData.extractTempDef;
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
                
                GaussFilterDrawable *bgDrawable = new GaussFilterDrawable(mWindPageBox,Rect::Make(1340,102,265,220),30,2,0x33bda279);
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
        }case t5_ai_coffee::R::id::icon_one_cup:{
            dealPowderItemClick(v);
            break;
        }case t5_ai_coffee::R::id::fav_enter_img:{
            Json::Value favItemData = g_objConf->getFavModeList()[mFavEditPos];
            Json::Value favSndItemData = favItemData["sndModeData"];

            // favSndItemData["powderDef"] = mCacheData.powderDef;
            favSndItemData["extractTempDef"] = mCacheData.extractTempDef;
            // favSndItemData["pressure"] = mCacheData.pressure;

            // favSndItemData["washTeaWater"] = mCacheData.washTeaWater;
            // favSndItemData["washTeaFlowRate"] = mCacheData.washTeaFlowRate;
            // favSndItemData["washTeaInterTime"] = mCacheData.washTeaInterTime;

            // favSndItemData["soakTeaWater"] = mCacheData.soakTeaWater;
            // favSndItemData["soakTeaFlowRate"] = mCacheData.soakTeaFlowRate;
            // favSndItemData["soakTeaInterTime"] = mCacheData.soakTeaInterTime;

            // favSndItemData["makeTeaWater"] = mCacheData.makeTeaWater;
            // favSndItemData["makeTeaFlowRate"] = mCacheData.makeTeaFlowRate;
            // favSndItemData["makeTeaInterTime"] = mCacheData.makeTeaInterTime;

            favItemData["sndModeData"] = favSndItemData;
            g_objConf->editFavModeData(favItemData,mFavEditPos);
            break;
        }case t5_ai_coffee::R::id::fav_back_img:{
            if(mExtractStep > MAKE_TEA_WASH){
                mExtractStep--;
                setStepStatus();
            }else{
                updateGroupData();
            }
            break;
        }case t5_ai_coffee::R::id::icon_tea_rebrew:{
            dealPowderItemClick(v);
            break;
        } 
    }
}

void HomeTabModeTea::dealPowderItemClick(View &v){
    if(!v.isActivated()) return;

    LOGE("%d %d  mExtractStep = %d",v.isSelected(),v.isActivated(),mExtractStep);
    // 不管单双，共同的操作
    switch(v.getId()){
        case t5_ai_coffee::R::id::icon_tea_rebrew:
        case t5_ai_coffee::R::id::icon_one_cup:{
            if(v.isSelected()){
                if(!(g_appData.machineState & MC_STATE_EXT)){
                    stopWork();
                }else{
                    g_windMgr->showPopPage(POP_TYPE_TEA_CANCEL_WORK,mWindPageBox,
                        [this](){ 
                            mExtractStep = MAKE_TEA_WASH; 
                            stopWork(); 
                            setStepStatus();
                        });
                }
            }else if(v.isActivated()){
                if(v.getId() == t5_ai_coffee::R::id::icon_one_cup) startWork();
                else {
                    g_windMgr->showSelectPosPopPage(POP_TYPE_TEA_REBREW_SET,mWindPageBox,mHorSelectPos,
                        [this](int selectPos){
                            mExtractStep = MAKE_TEA_REBREW;
                            mExtTeaData = g_objConf->getExtractTeaData();
                            mHorSelectPos = selectPos;
                            mHorModePicker->setValue(selectPos);
                            updateCoffeeGroupData(); // 已包含 setStepStatus 的操作
                            // mOneCupImg->performClick();
                            mTeaRebrewImg->setSelected(true);
                            startWork();
                        });
                }
            }
        }
    }
}

void HomeTabModeTea::stopWork(){
    if(mPageEditType != HOME_PAGE_NORMAL)   mFormStepAddImg->setVisibility(View::GONE);
    else                                    mFormStepAddImg->setVisibility(View::VISIBLE);
    mFavBackImg->setVisibility(View::VISIBLE);
    if(mOneCupImg->isSelected()){
        mOneCupImg->setSelected(false);
        mTeaRebrewImg->setActivated(true);
    }else{
        mTeaRebrewImg->setSelected(false);
        mOneCupImg->setActivated(true);
    }

    mHorModePicker->setEnabled(true);
    mHorModePicker->setAlpha(1.f);
    // mFormGroup->setAlpha(v.isSelected()?0.8f:1.f);
    mFormTitleGroup->setAlpha(1.f);

    int extTeaTotal = g_objConf->getTeaTotal()+1;
    // 涂鸦状态上报 
    if(mExtractStep == MAKE_TEA_WASH){
        g_appData.eqStatus = !(g_appData.machineState & MC_STATE_EXT)?ES_EASH_DONE:ES_STANDBY;
    }else{
        g_appData.eqStatus = !(g_appData.machineState & MC_STATE_EXT)?ES_EXTRACT_DONE:ES_STANDBY;
    }
#ifndef TUYA_OS_DISABLE
    // 工作状态 ------------
    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
    // 萃茶萃取次数
    if((mExtractStep != MAKE_TEA_WASH) && !(g_appData.machineState & MC_STATE_EXT)) 
        g_tuyaOsMgr->reportDpData(TYCMD_NUM_EXTRACT_TEA,PROP_VALUE, &extTeaTotal); 
#endif
    // 若已经是非工作状态，则不需要下发萃取指令,即工作完成
    if(!(g_appData.machineState & MC_STATE_EXT)){
        if(mExtractStep == MAKE_TEA_WASH){
            g_windMgr->showPopPage(POP_TYPE_TEA_EXT_TIPS,mWindPageBox,
                [this](){ mExtractStep = MAKE_TEA_EXT; mOneCupImg->performClick(); setStepStatus();},
                [this](){
                    mExtractStep = MAKE_TEA_WASH;
                    setStepStatus();
                    g_objConnMgr->setExtTea(false,0,false,{}); 
                    bool isStart = false;
                    g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
                    g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
                    // 工作状态 ------------
                    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
#endif
                });
        }else if((mExtractStep == MAKE_TEA_EXT) || (mExtractStep == MAKE_TEA_REBREW)){
            g_objConf->setTeaTotal(extTeaTotal);
            g_windMgr->showPopPage(POP_TYPE_TEA_REBREW_TIPS,mWindPageBox,
                [this](){ mExtractStep = MAKE_TEA_REBREW;mOneCupImg->performClick(); setStepStatus();},
                [this](){
                    mExtractStep = MAKE_TEA_WASH; 
                    setStepStatus();
                    bool isStart = false;
                    g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
                    g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
                    // 工作状态 ------------
                    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
#endif
                });
        }
    }else{
        // 中断萃茶
        g_appData.machineState &= ~MC_STATE_EXT;
        g_objConnMgr->setExtTea(false,0,false,{}); 
#ifndef TUYA_OS_DISABLE
        bool isStart = false;
        g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart);  
#endif
    }

    mRightInfoReduceImg->setVisibility(View::VISIBLE);
    mRightInfoAddImg->setVisibility(View::VISIBLE);
    mArcRight->setVisibility(View::VISIBLE);

    mRightAnim->setRepeatCount(1);
    mRightAnim->restart(0);
    mRightAnim->stop();
    
    updateRightInfoGroup();
    if((g_appData.warnState & A02)){ 
        std::time_t nowTime = std::time(NULL);
        if(nowTime >= std::mktime(&g_appData.nextPopWarnClean)){
            g_appData.warnIsDealState &= ~A02;
            g_windMgr->showPopPage(POP_WARN,mWindPageBox);
        }
    }
    if(mRightInfoTv->getLayoutParams()->width != 90){
        mRightInfoTv->getLayoutParams()->width = 90;
        mRightInfoTv->requestLayout();
    }

    mExtTeaData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = mCacheData.extractTempDef;
}

void HomeTabModeTea::startWork(){    
    if(!mTeaRebrewImg->isSelected()){
        mOneCupImg->setSelected(true);
        mTeaRebrewImg->setActivated(false);
    }else{
        mOneCupImg->setActivated(false);
    }
    
    if(mExtractStep == MAKE_TEA_WASH)       mRightInfoTitleTv->setText("洗茶中");
    else                                    mRightInfoTitleTv->setText("萃茶中");
    mFavBackImg->setVisibility(View::GONE);
    mRightInfoUnitsTv->setText("ml");
    mFormStepAddImg->setVisibility(View::GONE);
    
    mHorModePicker->setEnabled(false);
    mHorModePicker->setAlpha(0.4f);
    // mFormGroup->setAlpha(v.isSelected()?0.8f:1.f);
    mFormTitleGroup->setAlpha(0.4f);

    mRightInfoReduceImg->setVisibility(View::GONE);
    mRightInfoAddImg->setVisibility(View::GONE);
    mArcRight->setVisibility(View::GONE);
    g_appData.extractWater = 0;
    g_appData.extractPressure = 0;

    mRightInfoTv->setText("0");
    mRightAnim->setRepeatCount(-1);
    if(!mRightAnim->isRunning()){
        mRightAnim->start();
    }
    
    int extTeaTotal = g_objConf->getTeaTotal()+1;
    
    // 涂鸦状态上报 
    g_appData.eqStatus = mExtractStep == MAKE_TEA_WASH?ES_EASHING:ES_EXTRACTING;
    int extType = MAKE_TYPE_TEA;
    int coffeeMode = mExtTeaData.sndModeList.at(mHorModePicker->getValue()).sndModeType;
    bool isStart = true;
#ifndef TUYA_OS_DISABLE
    // 启动/暂停 -----------
    g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); 
    // 制作大类型 -----------
    g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); 
    g_tuyaOsMgr->reportDpData(TYCMD_TEA_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
    // 咖啡DIY ----------
    g_tuyaOsMgr->reportTeaDiyRawData(EXTRACTING_TEA, mExtTeaData.sndModeList.at(mHorModePicker->getValue()).sndModeType, 
                                        mCacheData.extractTempDef,
                                        mExtTeaData.sndModeList.at(mHorModePicker->getValue())); 
    LOGE("sndModeType = %d",mExtTeaData.sndModeList.at(mHorModePicker->getValue()).sndModeType);

    // 工作状态 ------------
    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); 
#endif
    g_appData.machineState |= MC_STATE_EXT;
    mExtTeaData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = mCacheData.extractTempDef;
    g_objConnMgr->setExtTea(true,mCacheData.extractTempDef,mExtractStep == MAKE_TEA_REBREW,mExtTeaData.sndModeList.at(mHorModePicker->getValue()));
}

void HomeTabModeTea::dealTuyaStartWork(){
    int Mode = g_appData.tuyaDiyData[1];
    int extMode = g_appData.tuyaDiyData[0];
    
    for(int i=0; i<mExtTeaData.sndModeList.size(); i++){
        if(mExtTeaData.sndModeList.at(i).sndModeType == Mode){
            mHorSelectPos = i;
            mHorModePicker->setValue(mHorSelectPos);
            break;
        }
    }
    mExtTeaData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = g_appData.tuyaDiyData[2];
    ExtractTeaSndModeDataStr &stepData = mExtTeaData.sndModeList.at(mHorModePicker->getValue());

    if(extMode == EXTRACTING_TEA){
        stepData.washTeaWater = g_appData.tuyaDiyData[3];
        stepData.washTeaFlowRate = g_appData.tuyaDiyData[4];
        stepData.washTeaInterTime = g_appData.tuyaDiyData[5];

        stepData.soakTeaWater = g_appData.tuyaDiyData[6];
        stepData.soakTeaFlowRate = g_appData.tuyaDiyData[7];
        stepData.soakTeaInterTime = g_appData.tuyaDiyData[8];
    }
    
    stepData.makeTeaWater = g_appData.tuyaDiyData[9];
    stepData.makeTeaFlowRate = g_appData.tuyaDiyData[10];
    stepData.makeTeaInterTime = g_appData.tuyaDiyData[11];

    g_objConf->setExtractTeaData(mExtTeaData);
}

void HomeTabModeTea::dealAiData(){
    int Mode = getJsonInt(g_appData.aiJsonText["params"],"snd_mode_type");
    for(int i=0; i<mExtTeaData.sndModeList.size(); i++){
        if(mExtTeaData.sndModeList.at(i).sndModeType == Mode){
            mHorSelectPos = i;
            mHorModePicker->setValue(mHorSelectPos);
            break;
        }
    }
    mExtTeaData.sndModeList.at(mHorModePicker->getValue()).extractTempDef = getJsonInt(g_appData.aiJsonText["params"],"extraction_temp");
    ExtractTeaSndModeDataStr &stepData = mExtTeaData.sndModeList.at(mHorModePicker->getValue());

    if(g_appData.aiJsonText["params"]["steps"].isArray()){
        Json::Value jsonStepList = g_appData.aiJsonText["params"]["steps"];
        if(jsonStepList.size() == 3){
            stepData.washTeaWater = getJsonInt(jsonStepList[0],"water");
            stepData.washTeaFlowRate = getJsonInt(jsonStepList[0],"flow_rate");
            stepData.washTeaInterTime = getJsonInt(jsonStepList[0],"break_time");

            stepData.soakTeaWater = getJsonInt(jsonStepList[1],"water");
            stepData.soakTeaFlowRate = getJsonInt(jsonStepList[1],"flow_rate");
            stepData.soakTeaInterTime = getJsonInt(jsonStepList[1],"break_time");
            
            stepData.makeTeaWater = getJsonInt(jsonStepList[2],"water");
            stepData.makeTeaFlowRate = getJsonInt(jsonStepList[2],"flow_rate");
            stepData.makeTeaInterTime = getJsonInt(jsonStepList[2],"break_time");
        }
    }
}

void HomeTabModeTea::onArcValueChangeListener(View &v, int progress, bool fromUser){
    if(!fromUser) return;
    switch(v.getId()){
        case t5_ai_coffee::R::id::arc_right:{
            mCacheData.extractTempDef = progress;
            mRightInfoTv->setText(std::to_string(progress));
            break;
        }
    }
    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
}

void HomeTabModeTea::onHorPickerChange(){
    mCacheData = mExtTeaData.sndModeList.at(mHorModePicker->getValue());
    mHorSelectPos = mHorModePicker->getValue();
    mPowderInfoTv->setText(std::to_string(mCacheData.powderDef));
    mArcRight->setProgress(mCacheData.extractTempDef);
    mRightInfoTv->setText(std::to_string(mCacheData.extractTempDef));
    mHomeFavGroup->setActivated(g_objConf->checkFavModeData(getCacheDataToJson()));
    setFormData();  
}

//设置 当前步骤
void HomeTabModeTea::setStepStatus(){
    if(mPageEditType != HOME_PAGE_FAV_EDIT){
        mFormStepGroup_0->setAlpha(mExtractStep==MAKE_TEA_WASH?1.f:0.4);
        mFormStepGroup_1->setAlpha(mExtractStep==MAKE_TEA_EXT?1.f:0.4);
        mFormStepGroup_2->setAlpha(mExtractStep!=MAKE_TEA_WASH?1.f:0.4);
    }else{
        mFormStepGroup_0->setAlpha(1.f);
        mFormStepGroup_1->setAlpha(1.f);
        mFormStepGroup_2->setAlpha(1.f);
    }
}

// 设置表格数据
void HomeTabModeTea::setFormData(){
    TextView *stepViewWater,*stepViewSpeed,*stepViewTime;
    std::string flowRateTxt;
    mFormStepGroup_0->setVisibility(View::VISIBLE);
    mFormStepGroup_1->setVisibility(View::VISIBLE);
    mFormStepGroup_2->setVisibility(View::VISIBLE);
    mFormStepGroup_3->setVisibility(View::GONE);
    std::vector<FormStepDataStr> stepData;
    ExtractTeaSndModeDataStr teaSndModeData = mCacheData;
    stepData = {{teaSndModeData.washTeaWater,teaSndModeData.washTeaFlowRate,teaSndModeData.washTeaInterTime},
                {teaSndModeData.soakTeaWater,teaSndModeData.soakTeaFlowRate,teaSndModeData.soakTeaInterTime},
                {teaSndModeData.makeTeaWater,teaSndModeData.makeTeaFlowRate,teaSndModeData.makeTeaInterTime}};
    mFormStepTitle_0->setText("洗茶");
    mFormStepTitle_1->setText("泡茶");
    mFormStepTitle_2->setText("冲茶");
    mFormStepTitle_0->setCompoundDrawablesWithIntrinsicBounds("nullptr","nullptr","nullptr","nullptr");
    mFormStepTitle_1->setCompoundDrawablesWithIntrinsicBounds("nullptr","nullptr","nullptr","nullptr");
    mFormStepTitle_2->setCompoundDrawablesWithIntrinsicBounds("nullptr","nullptr","nullptr","nullptr");
    mFormStepTitle_0->setPadding(0,0,0,0);
    mFormStepTitle_1->setPadding(0,0,0,0);
    mFormStepTitle_2->setPadding(0,0,0,0);

    mFormStepTitle_0->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepTitle_1->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    mFormStepTitle_2->setActivated(mPageEditType == HOME_PAGE_NORMAL || 1);
    
    for(int i=0; i<stepData.size(); i++){
        switch(i){
            case 0:{
                stepViewWater = mFormStepWater_0;
                stepViewSpeed = mFormStepSpeed_0;
                stepViewTime  = mFormStepTime_0;
                break;
            }case 1:{
                stepViewWater = mFormStepWater_1;
                stepViewSpeed = mFormStepSpeed_1;
                stepViewTime  = mFormStepTime_1;
                break;
            }case 2:{
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
 
        LOGV("water = %d  flowRateTxt = %s interTime = %d %s",stepData.at(i).water,flowRateTxt.c_str(),stepData.at(i).interTime,(i==stepData.size()-1)?"\n":"");
    }
    mFormStepAddImg->setActivated(false);
    if(mPageEditType != HOME_PAGE_NORMAL)   mFormStepAddImg->setVisibility(View::GONE);
    else                                    mFormStepAddImg->setVisibility(View::VISIBLE);
    setStepStatus();
}

Json::Value HomeTabModeTea::getCacheDataToJson(){
    Json::Value favItemData;
    Json::Value favSndItemData;

    favSndItemData["sndModename"] = mCacheData.sndModename;
    favSndItemData["pic"] = mCacheData.pic;
    favSndItemData["pressure"] = mCacheData.pressure;
    favSndItemData["powderDef"] = mCacheData.powderDef;
    favSndItemData["washTeaNumMin"] = mCacheData.washTeaNumMin;
    favSndItemData["washTeaNumMax"] = mCacheData.washTeaNumMax;
    favSndItemData["extractTempDef"] = mCacheData.extractTempDef;

    favSndItemData["washTeaWater"] = mCacheData.washTeaWater;
    favSndItemData["washTeaFlowRate"] = mCacheData.washTeaFlowRate;
    favSndItemData["washTeaInterTime"] = mCacheData.washTeaInterTime;

    favSndItemData["soakTeaWater"] = mCacheData.soakTeaWater;
    favSndItemData["soakTeaFlowRate"] = mCacheData.soakTeaFlowRate;
    favSndItemData["soakTeaInterTime"] = mCacheData.soakTeaInterTime;

    favSndItemData["makeTeaWater"] = mCacheData.makeTeaWater;
    favSndItemData["makeTeaFlowRate"] = mCacheData.makeTeaFlowRate;
    favSndItemData["makeTeaInterTime"] = mCacheData.makeTeaInterTime;

    favItemData["coffeeMode"] = HOME_MT_EXTRACTING_TEA;
    favItemData["name"] = "萃茶";
    favItemData["beanGrindMode"] = g_objConf->getBeanGrindMode();
    favItemData["sndModeData"] = favSndItemData;


    return favItemData;
}

/////////////////////////////////////////////////////////////////////////////////////
HomeTabModeTea::TeaRecycAdapter::TeaRecycAdapter(ViewGroup *wind_page_box):mWindPageBox(wind_page_box){
    teaModeList = g_objConf->getExtractTeaData().sndModeList;
    mCardClickListener = nullptr;
}


RecyclerView::ViewHolder* HomeTabModeTea::TeaRecycAdapter::onCreateViewHolder(ViewGroup* parent, int viewType){
    ViewGroup *convertView;
    convertView = (ViewGroup *)LayoutInflater::from(parent->getContext())->inflate("@layout/tea_card_layout", nullptr);
    return new TeaRecycAdapter::ViewHolder(convertView);
}

void HomeTabModeTea::TeaRecycAdapter::onBindViewHolder(RecyclerView::ViewHolder& holder, int position){
    
    ViewGroup *viewGroup = ((TeaRecycAdapter::ViewHolder&)holder).viewGroup;
    TextView *nameTv = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::tea_card_name_tv);
    // ImageView *rebrewImg = (ImageView *)viewGroup->findViewById(t5_ai_coffee::R::id::tea_card_rebrew_img);
    nameTv->setText(teaModeList.at(position).sndModename);
    viewGroup->setBackgroundResource(teaModeList.at(position).pic);
    // rebrewImg->setOnClickListener([this,&holder](View &v){
    //     LOGE("rebrewImg click!");
    //     if(mCardClickListener) mCardClickListener(v,holder.getLayoutPosition());
    // });

    viewGroup->setOnClickListener([this,&holder](View &v){
        LOGE("card click!");
        if(mCardClickListener) mCardClickListener(v,holder.getLayoutPosition());
    });
    
}

int HomeTabModeTea::TeaRecycAdapter::getItemCount(){
    return teaModeList.size();
}

void HomeTabModeTea::TeaRecycAdapter::notifyItemMove(int FromPos, int ToPos){
    if(teaModeList.size() <= FromPos || teaModeList.size() <= ToPos){
        LOGE("notifyItemReMove FromPos|ToPos >= mFavListJson.size()");
        return;
    }
    ExtractTeaDataStr extTeaData = g_objConf->getExtractTeaData();
    ExtractTeaSndModeDataStr data = teaModeList[FromPos];
    teaModeList.erase(teaModeList.begin()+FromPos);
    teaModeList.insert(teaModeList.begin()+ToPos,data);

    extTeaData.sndModeList = teaModeList;
    g_objConf->setExtractTeaData(extTeaData);
    notifyItemMoved(FromPos,ToPos);
}

void HomeTabModeTea::TeaRecycAdapter::setOnCardClickListener(OnCardClickListener l){
    mCardClickListener = l;
}

HomeTabModeTea::TeaItemDecoration::TeaItemDecoration(int firItemLeft, int betweenItemRight, int lastItemRight)
    :firstItemLeftSpacing(firItemLeft),betweenItemRightSpacing(betweenItemRight),lastItemRightSpacing(lastItemRight){

}

void HomeTabModeTea::TeaItemDecoration::getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state){
    int position = parent.getChildAdapterPosition(&view);
    int itemCOunt = parent.getAdapter()->getItemCount();

    if(position == 0){
        outRect.left = firstItemLeftSpacing;
        outRect.width = betweenItemRightSpacing;
    }else if(position == itemCOunt-1){
        outRect.width = lastItemRightSpacing;
    }else{
        outRect.width = betweenItemRightSpacing;
    }
}


HomeTabModeTea::TeaSimpleCallback::TeaSimpleCallback(TeaRecycAdapter*adapter):ItemTouchHelper::SimpleCallback(
    ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT| ItemTouchHelper::UP | ItemTouchHelper::DOWN,
    0){
    mAdapter = adapter;
}

bool HomeTabModeTea::TeaSimpleCallback::onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target){
    const int fromPosition=viewHolder.getLayoutPosition();
    const int toPosition=target.getLayoutPosition();
    mAdapter->notifyItemMove(fromPosition, toPosition);
    return true;
}

void HomeTabModeTea::TeaSimpleCallback::onSwiped(RecyclerView::ViewHolder& viewHolder, int direction){
    const int swipedPosition = viewHolder.getLayoutPosition();
    LOGE("direction=%d swipedPosition=%d",direction,swipedPosition);
    // mAdapter->notifyItemReMove(swipedPosition);
}

void HomeTabModeTea::TeaSimpleCallback::onChildDraw(Canvas& c, RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive){

    if(isCurrentlyActive){
       ViewGroup *viewGroup = ((TeaRecycAdapter::ViewHolder&)viewHolder).viewGroup; 
       viewGroup->setScaleX(0.9);
       viewGroup->setScaleY(0.9);
    }else{
        ViewGroup *viewGroup = ((TeaRecycAdapter::ViewHolder&)viewHolder).viewGroup; 
       viewGroup->setScaleX(1.0);
       viewGroup->setScaleY(1.0);
    }
    ItemTouchHelper::Callback::onChildDraw(c,recyclerView,viewHolder,dX,dY,actionState,isCurrentlyActive);
}

int HomeTabModeTea::TeaSimpleCallback::interpolateOutOfBoundsScroll(RecyclerView& recyclerView, int viewSize, int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll) {
        int maxScroll = recyclerView.getHeight() / 2;
        float decay = 0.1f;
        int delta = (int) (viewSizeOutOfBounds * decay);
        if (delta < 0) {
            return std::max(delta, -maxScroll);
        } else {
            return std::min(delta, maxScroll);
        }
}

