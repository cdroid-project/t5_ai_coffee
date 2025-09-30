#include "page_clean.h"
#include "wind_mgr.h"
#include "data.h"
#include "conf_mgr.h"
#include "pop_normal.h"
#include "conn_mgr.h"
#include "tuya_os_mgr.h"
#include <core/inputeventsource.h>

PageClean::PageClean(ViewGroup *wind_page_box,int pageType)
    :PageBase(wind_page_box,"@layout/page_clean",PAGE_CLEAN){

    if(pageType == PAGE_CLEAN){
        mIsSelectWater = false;
        mPageStep = 0;
    }else if(pageType == PAGE_CLEAN_CALCIFY){
        mIsSelectWater = false;
        mPageStep = 1;
    }
    initPageView();
    initPageData();
    changePageStep();
}

PageClean::~PageClean(){
    mCleanAnim->cancel();
    delete mCleanAnim;
}


void PageClean::initPageView(){
    
    mSelectGroup = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::clean_select);
    mStepGroup = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_tips_group);
    mRunningGroup = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::clean_running_group);
    // select group
    mSelectWaterTv = (TextView *)mSelectGroup->findViewById(kaidu_ms7_lqy::R::id::clean_select_water_enter_tv);
    mSelectCalcifyTv = (TextView *)mSelectGroup->findViewById(kaidu_ms7_lqy::R::id::clean_select_calcify_enter_tv);
    mCalCleanWarnImg = mSelectGroup->findViewById(kaidu_ms7_lqy::R::id::cal_clean_warn_img);
    mCalInfoTv = (TextView *)mSelectGroup->findViewById(kaidu_ms7_lqy::R::id::clean_select_calcify_time_tv);
    mCalInfoTv2 = (TextView *)mSelectGroup->findViewById(kaidu_ms7_lqy::R::id::clean_select_calcify_time_tv2);
    
    // step group
    mStepGroup_1 = (ViewGroup *)mStepGroup->findViewById(kaidu_ms7_lqy::R::id::step_1_group);
    mStepGroup_2 = (ViewGroup *)mStepGroup->findViewById(kaidu_ms7_lqy::R::id::step_2_group);

    mSteptitleTv_1 = (TextView *)mStepGroup_1->findViewById(kaidu_ms7_lqy::R::id::clean_step_1_title_tv);
    mStepInfoTv_1 = (TextView *)mStepGroup_1->findViewById(kaidu_ms7_lqy::R::id::clean_step_1_info_tv);
    mSteptitleTv_2 = (TextView *)mStepGroup_2->findViewById(kaidu_ms7_lqy::R::id::clean_step_2_title_tv);
    mStepInfoTv_2 = (TextView *)mStepGroup_2->findViewById(kaidu_ms7_lqy::R::id::clean_step_2_info_tv);
    
    mStepEnterTv = (TextView *)mStepGroup->findViewById(kaidu_ms7_lqy::R::id::clean_enter);
    mStepcancelTv = (TextView *)mStepGroup->findViewById(kaidu_ms7_lqy::R::id::clean_cancel);

    // running group
    mRunningTitleTv = (TextView *)mRunningGroup->findViewById(kaidu_ms7_lqy::R::id::clean_running_title_tv);
    mRunningInfoTv = (TextView *)mRunningGroup->findViewById(kaidu_ms7_lqy::R::id::clean_running_info_tv);
    mRunningView = mRunningGroup->findViewById(kaidu_ms7_lqy::R::id::clean_running_rolling);


    auto btn_click_func = std::bind(&PageClean::btnClickListener, this, std::placeholders::_1);
    mPageLayout->setOnClickListener(btn_click_func);
    mSelectWaterTv->setOnClickListener(btn_click_func);
    mSelectCalcifyTv->setOnClickListener(btn_click_func);
    mStepEnterTv->setOnClickListener(btn_click_func);
    mStepcancelTv->setOnClickListener(btn_click_func);
    
    mPageLayout->findViewById(kaidu_ms7_lqy::R::id::title_back)->setOnClickListener(btn_click_func);

    mPageLayout->setSoundEffectsEnabled(false);
    
    mCleanAnim = ValueAnimator::ofInt({0,380});
    mCleanAnim->setInterpolator(AccelerateDecelerateInterpolator::gAccelerateDecelerateInterpolator.get()); // 设置动画的插值器
    mCleanAnim->setRepeatCount(-1);       // 设置动画重复次数
    mCleanAnim->setRepeatMode(ValueAnimator::REVERSE);
    mCleanAnim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        int t = GET_VARIANT(anim.getAnimatedValue(),int);
        mRunningView->setTranslationX(t);
    }));
}

void PageClean::initPageData(){
    mCalCleanWarnImg->setVisibility((g_appData.warnState & A02)?View::VISIBLE:View::GONE);
    if(g_appData.warnState & A02){
        mCalInfoTv->setText("萃取已超过1500次/使用6个月\n以上，需进行钙化清洁");
        mCalInfoTv2->setText("预计需要20分钟，中途需加2次水");
        ((MarginLayoutParams*)mCalInfoTv2->getLayoutParams())->topMargin = 15;
    }else{
        mCalInfoTv->setText("预计需要  20分钟");
        mCalInfoTv2->setText("中途需加2次水");
        ((MarginLayoutParams*)mCalInfoTv2->getLayoutParams())->topMargin = 0;
    }
}

void PageClean::updatePageData(){
    // 清洁完成
    if(g_appData.statusChangeFlag & CS_CLEAN_DOWN){
        InputEventSource::getInstance().closeScreenSaver();
        if(mRunningGroup->getVisibility() == View::VISIBLE){
            mCleanAnim->cancel();
            mRunningView->setTranslationX(0);
            mPageStep++;
            changePageStep();
        }
    }

    if((g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && (g_appData.machineState != MC_STATE_NONE) && (g_appData.warnState != WARN_NONE)){
        if((g_appData.warnState & A01)){
            mCleanAnim->pause();
            g_objConnMgr->setCleanSelf(false,mIsSelectWater?CLEAN_TYPE_WATER:CLEAN_TYPE_CALCIFY);
            bool isStart = false;
            int  cleanMode = mIsSelectWater?CLEAN_MODE_PIPE:(mPageStep == 5?CLEAN_MODE_CAL_SECOND:CLEAN_MODE_CAL);
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_MODE,PROP_ENUM, &cleanMode);// 清洁工作模式
            g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); // 清洁 false
#endif
            return;
        }else if(!((g_appData.warnState & A02) || (g_appData.warnState & A03))){
            if(mPageStep > 0) mPageStep--;
            changePageStep();
        }
    }

    if(g_appData.statusChangeFlag & CS_WARN_CLOSE){
        InputEventSource::getInstance().closeScreenSaver();
        changePageStep();
    }

    if(g_appData.statusChangeFlag & CS_TUYA_START_CLEAN){
        InputEventSource::getInstance().closeScreenSaver();
        if(g_appData.tuyaStartCleaning){
            if((g_appData.tuyaCleanMode == CLEAN_MODE_CAL_SECOND) && (!mIsSelectWater) && (mPageStep == 4)){
                mCleanAnim->cancel();
                mRunningView->setTranslationX(0);
                mPageStep++;
                changePageStep();
            }else if(g_appData.tuyaCleanMode == CLEAN_MODE_PIPE){
                mIsSelectWater = true;
                mPageStep = 2;
                changePageStep();
            }else if(g_appData.tuyaCleanMode == CLEAN_MODE_CAL){
                mIsSelectWater = false;
                mPageStep = 3;
                changePageStep();
            }
        }else{
            if(mIsSelectWater){
                g_appData.machineState &= ~MC_STATE_CLEAN_WATER;
                g_objConnMgr->setCleanSelf(false,CLEAN_TYPE_WATER);
            }else{
                g_appData.machineState &= ~MC_STATE_CLEAN_CAL;
                g_objConnMgr->setCleanSelf(false,CLEAN_TYPE_CALCIFY);
            }
            bool isStart = false;
            g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); // 清洁 false
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
            g_windMgr->showPrevPage();
        }
    }
}

void PageClean::stopPageAction(){
    
}

void PageClean::changePageStep(){
    mSelectGroup->setVisibility(View::GONE);
    mStepGroup->setVisibility(View::GONE);
    mRunningGroup->setVisibility(View::GONE);
    mStepGroup_1->setVisibility(View::VISIBLE);
    LOGE("mPageStep = %d mIsSelectWater = %d",mPageStep,mIsSelectWater);
    if(mPageStep == 0){
        mSelectGroup->setVisibility(View::VISIBLE);
    }else{
        bool isStart = false;
        int  cleanMode = CLEAN_MODE_PIPE;
        if(mIsSelectWater){
            cleanMode = CLEAN_MODE_PIPE;
            if(mPageStep == 1){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::VISIBLE);
                mSteptitleTv_1->setText("STEP1");
                mSteptitleTv_2->setText("STEP2");
                mStepInfoTv_1->setText("确保水箱内水超过250ml");
                mStepInfoTv_2->setText("请在冲煮头处放置装水的容器");
                mStepEnterTv->setText("下一步");
                mStepcancelTv->setVisibility(View::VISIBLE);
                return;
            }else if(mPageStep == 2){
                mRunningGroup->setVisibility(View::VISIBLE);
                mRunningTitleTv->setText("水路清洁中");
                mRunningInfoTv->setText("预计需要 1分钟");
                mRunningView->setTranslationX(0);
                if(mCleanAnim->isPaused()){
                    mCleanAnim->resume();
                }else{
                    mCleanAnim->setDuration(2*1000);
                    mCleanAnim->start();
                }
                g_appData.machineState |= MC_STATE_CLEAN_WATER;
                g_objConnMgr->setCleanSelf(true,CLEAN_TYPE_WATER);
                isStart = true;
                g_appData.eqStatus = ES_CLEAN_PIPE;
            }else if(mPageStep == 3){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::GONE);
                mSteptitleTv_1->setText("STEP3");
                mStepInfoTv_1->setText("使用卫生干燥的毛巾擦拭冲煮头及蒸汽管表面的残留水渍。");
                mStepEnterTv->setText("完成");
                mStepcancelTv->setVisibility(View::GONE);
                isStart = false;
                g_appData.eqStatus = ES_CLEAN_CAL_DONE;
            }else if(mPageStep > 3){
                g_windMgr->showPrevPage();
                return;
            }
        }else{
            cleanMode = CLEAN_MODE_CAL;
            if(mPageStep == 1){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::GONE);
                mSteptitleTv_1->setText("STEP1");
                mStepInfoTv_1->setText("往水箱注水至“最大”位置。加入除垢片，待除垢片完成溶化。确保水箱插入到位，水剂比例参考实际除垢产品。");
                mStepEnterTv->setText("下一步");
                mStepcancelTv->setVisibility(View::VISIBLE);
                return;
            }else if(mPageStep == 2){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::GONE);
                mSteptitleTv_1->setText("STEP2");
                mStepInfoTv_1->setText("放置一个1升的容器在冲煮头下面，另一个1升容器在蒸汽导管下面。");
                mStepEnterTv->setText("第一轮清洗");
                mStepcancelTv->setVisibility(View::GONE);
                return;
            }else if(mPageStep == 3){
                mRunningGroup->setVisibility(View::VISIBLE);
                mRunningTitleTv->setText("第一轮钙化清洁中");
                mRunningInfoTv->setText("预计需要 10分钟");
                mRunningView->setTranslationX(0);
                if(mCleanAnim->isPaused()){
                    mCleanAnim->resume();
                }else{
                    mCleanAnim->setDuration(2*1000);
                    mCleanAnim->start();
                }
                g_appData.machineState |= MC_STATE_CLEAN_CAL;
                g_objConnMgr->setCleanSelf(true,CLEAN_TYPE_CALCIFY);
                isStart = true;
                g_appData.eqStatus = ES_CLEAN_CAL_1;
            }else if(mPageStep == 4){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::GONE);
                mSteptitleTv_1->setText("STEP3");
                mStepInfoTv_1->setText("请拿出水箱，并清洗干净，避免除垢剂残留，水箱重新装水至“最大”位置。");
                mStepEnterTv->setText("第二轮清洗");
                mStepcancelTv->setVisibility(View::GONE);
                isStart = false;
                g_appData.eqStatus = ES_CLEAN_CAL_1_DONE;
            }else if(mPageStep == 5){
                mRunningGroup->setVisibility(View::VISIBLE);
                mRunningTitleTv->setText("第二轮钙化清洁中");
                mRunningInfoTv->setText("预计需要 10分钟");
                mRunningView->setTranslationX(0);
                if(mCleanAnim->isPaused()){
                    mCleanAnim->resume();
                }else{
                    mCleanAnim->setDuration(2*1000);
                    mCleanAnim->start();
                }
                g_appData.machineState |= MC_STATE_CLEAN_CAL;
                g_objConnMgr->setCleanSelf(true,CLEAN_TYPE_CALCIFY);
                isStart = true;
                g_appData.eqStatus = ES_CLEAN_CAL_2;
                cleanMode = CLEAN_MODE_CAL_SECOND;
            }else if(mPageStep == 6){
                mStepGroup->setVisibility(View::VISIBLE);
                mStepGroup_2->setVisibility(View::GONE);
                mSteptitleTv_1->setText("STEP4");
                mStepInfoTv_1->setText("使用卫生干燥的毛巾擦拭冲煮头及蒸汽管表面的残留水渍。");
                mStepEnterTv->setText("完成");
                mStepcancelTv->setVisibility(View::GONE);
                g_windMgr->changeTitleBar(TITLE_BAR_CLEAN);
                isStart = false;
                g_appData.eqStatus = ES_CLEAN_CAL_DONE;
            }else if(mPageStep > 6){
                g_windMgr->showPrevPage();
                return;
            }
        }
#ifndef TUYA_OS_DISABLE
        g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_MODE,PROP_ENUM, &cleanMode); // 清洁工作模式
        g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); // 清洁 false
        g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
    }
}

void PageClean::btnClickListener(View& view){
    switch(view.getId()){
        case kaidu_ms7_lqy::R::id::title_back:{
            if(mRunningGroup->getVisibility() == View::VISIBLE
                || (!mIsSelectWater && (mPageStep == 4))){
                    if(!mIsSelectWater && (mPageStep != 4)){
                        showToast("清洁中，请稍候");
                        return;
                    }
                g_windMgr->showPopPage(POP_STOP_CLEAN,mPageLayout,[this](){
                    if(mIsSelectWater){
                        g_appData.machineState &= ~MC_STATE_CLEAN_WATER;
                        g_objConnMgr->setCleanSelf(false,CLEAN_TYPE_WATER);
                    }else{
                        g_appData.machineState &= ~MC_STATE_CLEAN_CAL;
                        g_objConnMgr->setCleanSelf(false,CLEAN_TYPE_CALCIFY);
                    }
                    bool isStart = false;
                    g_appData.eqStatus = ES_STANDBY;
#ifndef TUYA_OS_DISABLE
                    g_tuyaOsMgr->reportDpData(TYCMD_CLEANING_START,PROP_BOOL, &isStart); // 清洁 false
                    g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
                    g_windMgr->showPrevPage();
                });
            }else{
                g_windMgr->showPrevPage();
            }
            break;
        }case kaidu_ms7_lqy::R::id::clean_cancel:{
            mPageStep--;
            changePageStep();
            break;
        }case kaidu_ms7_lqy::R::id::clean_enter:{
            mPageStep++;
            changePageStep();
            break;
        }case kaidu_ms7_lqy::R::id::clean_select_water_enter_tv:{
            mIsSelectWater = true;
            mPageStep = 1;
            changePageStep();
            break;
        }case kaidu_ms7_lqy::R::id::clean_select_calcify_enter_tv:{
            mIsSelectWater = false;
            mPageStep = 1;
            changePageStep();
            break;
        }
    }
}
