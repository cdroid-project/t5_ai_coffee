#include "page_scale_calibration.h"
#include "wind_mgr.h"
#include "data.h"
#include "conf_mgr.h"
#include "conn_mgr.h"

PageScaleCalibration::PageScaleCalibration(ViewGroup *wind_page_box)
    :PageBase(wind_page_box,"@layout/page_scale_calibration",PAGE_SCALE_CALIBRATION){

    initPageView();
    initPageData();
    
    g_objConnMgr->setElectronicScaleCal(true);
}

PageScaleCalibration::~PageScaleCalibration(){
    g_objConnMgr->setElectronicScaleCal(false);
}


void PageScaleCalibration::initPageView(){
    
    mStep1Group = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_1_group);
    mStep2Group = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_2_group);
    mStep3Group = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_3_group);
    mStep4Group = (ViewGroup *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_4_group);
    
    mStep4Tv    = (TextView *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::step_4_info_tv);
    mWeightTv   = (TextView *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::card2_weight_tv);
    
    mPageLayout->findViewById(kaidu_ms7_lqy::R::id::title_back)->setOnClickListener([this](View &v){
        g_windMgr->showPrevPage();
    });

    
}

void PageScaleCalibration::initPageData(){
    mSelectGroup = mStep1Group;
    mSelectGroup->setSelected(true);  
    mStep4Tv->setText("电子秤标定"); 
    mWeightTv->setText(std::to_string(g_appData.scaleCalWeight));
}

void PageScaleCalibration::updatePageData(){
    // 标定步骤变化
    if(g_appData.statusChangeFlag & CS_SCALE_CAL_CHANGE){
        switch(g_appData.scaleCalFlag){
            case SCALE_CAL_CHANGE_POINT:
            case SCALE_CAL_GET_WEIGHT:
            case SCALE_CAL_CLEAR_WEIGHT:
                break;
            case SCALE_CAL_ZERO:{
                mSelectGroup->setSelected(false);
                mSelectGroup = mStep1Group;
                mSelectGroup->setSelected(true);
                break;
            }case SCALE_CAL_500G:{
                mSelectGroup->setSelected(false);
                mSelectGroup = mStep2Group;
                mSelectGroup->setSelected(true);
                break;
            }case SCALE_CAL_1000G:{
                mSelectGroup->setSelected(false);
                mSelectGroup = mStep3Group;
                mSelectGroup->setSelected(true);
                break;
            }case SCALE_CAL_SUCCESS:{
                mSelectGroup->setSelected(false);
                mSelectGroup = mStep4Group;
                mSelectGroup->setSelected(true);
                mStep4Tv->setText("电子秤标定成功"); 
                break;
            }case SCALE_CAL_FAIL:{
                mSelectGroup->setSelected(false);
                mSelectGroup = mStep4Group;
                mSelectGroup->setSelected(true);
                mStep4Tv->setText("电子秤标定失败"); 
                break;
            }
        }
        mWeightTv->setText(std::to_string(g_appData.scaleCalWeight));
    }
}

void PageScaleCalibration::stopPageAction(){
    
    
}
