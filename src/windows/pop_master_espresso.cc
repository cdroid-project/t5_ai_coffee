/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:29:33
 * @FilePath: /t5_ai_demo/src/windows/pop_master_espresso.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "pop_master_espresso.h"

#include <plotaxis.h>
#include <cyplotobject.h>

#include "R.h"
#include "wind_mgr.h"
#include "project.h"
#include "proto.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"

PopMasterEspresso::PopMasterEspresso(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,popModeCb enterCb,popModeCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_master_espresso",POP_TYPE_MASTER_ESPRESSO,enterCb,cencelCb),mPickerSelectPos(selectPos){

    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));

    initPopView();
    initPlotViewData();

    mPopLayout->setSoundEffectsEnabled(false); // 关闭点击音效
}

PopMasterEspresso::~PopMasterEspresso() {
    mPopLayout->removeCallbacks(mSetPlotDataRun);
    mResetAnimator->cancel();
    delete mResetAnimator;
}

void PopMasterEspresso::initPopView(){
    mPopPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker);

    mMasEspPopPlotView = (CYPlotView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_plotview);
    mMasEspPopStep1 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_extraction_step1);
    mMasEspPopStep2 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_extraction_step2);
    mMasEspPopStep3 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_extraction_step3);
    mMasEspPopStep4 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_extraction_step4);
    mMasEspPopStep5 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_extraction_step5);

    mMasEspPopStepAdd = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_add);
    mMasEspPopStepDelete = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_delete);

    mMasEspPickerPressure = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_pressure);
    mMasEspPickerWater = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_water);

    mMasEspPopResetAnim = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_anim);

    mPopBtnConfirm   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel);

    initPopData();
    
    // mPopGaussBox = (ViewGroup *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_group);
    auto clickFunc = std::bind(&PopMasterEspresso::onBtnClickListener,this,std::placeholders::_1);
    auto clickStepFunc = std::bind(&PopMasterEspresso::onMasEspStepBtnClickListener,this,std::placeholders::_1);
    auto pickerValueChangedFunc = std::bind(&PopMasterEspresso::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
    mMasEspPopStep1->setOnClickListener(clickStepFunc);
    mMasEspPopStep2->setOnClickListener(clickStepFunc);
    mMasEspPopStep3->setOnClickListener(clickStepFunc);
    mMasEspPopStep4->setOnClickListener(clickStepFunc);
    mMasEspPopStep5->setOnClickListener(clickStepFunc);
    mMasEspPopStepAdd->setOnClickListener(clickFunc);
    mMasEspPopStepDelete->setOnClickListener(clickFunc);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_group)->setOnClickListener(clickFunc);
    // mPopPicker->setOnClickListener(clickFunc);
    mPopPicker->setOnValueChangedListener(pickerValueChangedFunc);
    mMasEspPickerPressure->setOnValueChangedListener(pickerValueChangedFunc);
    mMasEspPickerWater->setOnValueChangedListener(pickerValueChangedFunc);

    mPopLayout->setSoundEffectsEnabled(false);

    mResetAnimator = ValueAnimator::ofInt({0,10000});
    mResetAnimator->setDuration(1000);
    mResetAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
    mResetAnimator->setRepeatCount(0);       // 设置动画重复次数
    mResetAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        const int t = GET_VARIANT(anim.getAnimatedValue(),int);
        mMasEspPopResetAnim->getDrawable()->setLevel(t);
        // LOGV("position = %f",position); 
    }));

    // 旋转时有锯齿
    ((BitmapDrawable *)((RotateDrawable *)mMasEspPopResetAnim->getDrawable())->getDrawable())->setFilterBitmap(true);
}
void PopMasterEspresso::initPopData(){
    mMasEspData = g_objConf->getMasEspData();
    mMasEspPickerWater->setMinValue(5/5);
    mMasEspPickerWater->setMaxValue(200/5);
    mMasEspPickerWater->setFormatter([this](int value){ return std::to_string(value*5); });
    mPopPicker->setMaxValue(mMasEspData.sndModeList.size()-1);
    mPopPicker->setFormatter([this](int value){ return mMasEspData.sndModeList.at(value).sndModename; });
    mPopPicker->setValue(mPickerSelectPos);
    setStepData();

    mSetPlotDataRun = [this](){
        MasEspStepDataStr &stepData = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
        stepData.pressure = mMasEspPickerPressure->getValue();
        stepData.water = mMasEspPickerWater->getValue()*5;
        setPlotViewData();
    };
}   

void PopMasterEspresso::setStepData(){
    TextView *stepView;
    std::string stepText;
    mMasEspPopStep1->setVisibility(View::GONE);
    mMasEspPopStep2->setVisibility(View::GONE);
    mMasEspPopStep3->setVisibility(View::GONE);
    mMasEspPopStep4->setVisibility(View::GONE);
    mMasEspPopStep5->setVisibility(View::GONE);
    std::vector<MasEspStepDataStr> stepList = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList;
    for(int i=0; i<stepList.size(); i++){
        switch(i){
            case 0:{
                stepView = mMasEspPopStep1;
                stepText = "①";
                break;
            }case 1:{
                stepView = mMasEspPopStep2;
                stepText = "②";
                break;
            }case 2:{
                stepView = mMasEspPopStep3;
                stepText = "③";
                break;
            }case 3:{
                stepView = mMasEspPopStep4;
                stepText = "④";
                break;
            }case 4:{
                stepView = mMasEspPopStep5;
                stepText = "⑤";
                break;
            }
        }

        stepText += fillLength(stepList.at(i).pressure,2," ")+" bar-"+fillLength(stepList.at(i).water,2," ")+" ml";
        LOGE("stepText = [%s]",stepText.c_str());
        stepView->setVisibility(View::VISIBLE);
        stepView->getLayoutParams()->height = 45;
        stepView->setSelected(false);
        stepView->setText(stepText);
    }
    mStepSelectView = stepView;
    mStepSelectPos = stepList.size()-1;
    if(stepList.size() > 0){
        stepView->getLayoutParams()->height = 54;
        stepView->setSelected(true);
        if(stepList.size() >= 5) mMasEspPopStepAdd->setVisibility(View::GONE);
        else                     mMasEspPopStepAdd->setVisibility(View::VISIBLE);
        if(mStepSelectPos > 0){
            mMasEspPopStepDelete->setTranslationY(75+45*mStepSelectPos);
            mMasEspPopStepDelete->setVisibility(View::VISIBLE);
        }else{
            mMasEspPopStepDelete->setVisibility(View::GONE);
        } 
    }
    mMasEspPickerPressure->setValue(stepList.back().pressure);
    mMasEspPickerWater->setValue(stepList.back().water/5);
}

void PopMasterEspresso::initPlotViewData(){
    // 是否显示辅助线
    mMasEspPopPlotView->setShowGrid(true);
    // 设置 坐标轴颜色（包括文本）
    mMasEspPopPlotView->setForegroundColor(0x33FFFFFF);
    // 设置 辅助线颜色
    mMasEspPopPlotView->setGridColor(0x33FFFFFF);
    
    // 辅助线
    mMasEspPopPlotView->axis(CYPlotView::LeftAxis)->setTickmarkVisible(true);

    mMasEspPopPlotView->axis(CYPlotView::RightAxis)->setTickmarkVisible(false);
    mMasEspPopPlotView->axis(CYPlotView::TopAxis)->setTickmarkVisible(false);
    mMasEspPopPlotView->axis(CYPlotView::BottomAxis)->setTickmarkVisible(false);

    // 坐标轴
    mMasEspPopPlotView->axis(CYPlotView::LeftAxis)->setVisible(false);
    mMasEspPopPlotView->axis(CYPlotView::RightAxis)->setVisible(false);
    mMasEspPopPlotView->axis(CYPlotView::TopAxis)->setVisible(false);
    mMasEspPopPlotView->axis(CYPlotView::BottomAxis)->setVisible(false);
    
    // 坐标轴 信息
    mMasEspPopPlotView->axis(CYPlotView::LeftAxis)->setTickLabelsShown(true);
    mMasEspPopPlotView->axis(CYPlotView::BottomAxis)->setTickLabelsShown(true);

    mMasEspPopPlotView->axis(CYPlotView::TopAxis)->setTickLabelsShown(false);
    mMasEspPopPlotView->axis(CYPlotView::RightAxis)->setTickLabelsShown(false);

    mMasEspPopPlotView->axis(CYPlotView::LeftAxis)->setLabel(std::string("bar"));  // 自己修改过源码从而达到效果
    mMasEspPopPlotView->axis(CYPlotView::BottomAxis)->setLabel(std::string("ml"));  // 自己修改过源码从而达到效果
    

    // 设置数据点，包括属性
    mPlotObj = new CYPlotObject(Color::BLUE, CYPlotObject::Lines, 10, CYPlotObject::Pentagon);

    Cairo::RefPtr<Cairo::LinearGradient> patternColor = Cairo::LinearGradient::create(0,320,0,0);
    patternColor->add_color_stop_rgb(0,0.310,0.035,0.553);
    patternColor->add_color_stop_rgb(1,0.694,0.318,0.565);
    
    Color clr(0xFFACB6E5);
    RefPtr<Pattern> labelColor = Cairo::SolidPattern::create_rgba(clr.red(),clr.green(),clr.blue(),clr.alpha());
    // 柱状图 生效的属性
    mPlotObj->setLabelPen(labelColor);   // 设置 柱状 label文本的颜色
    mPlotObj->setBarBrush(patternColor); // 设置 柱状 的内部颜色
    mPlotObj->setBarPen(patternColor);   // 设置 柱状 的边框颜色

    //线条 生效的属性
    mPlotObj->setLinePen(labelColor); // 设置 线状 颜色

    setPlotViewData();
    
    mMasEspPopPlotView->addPlotObject(mPlotObj);
}

void PopMasterEspresso::setPlotViewData(){
    // 数据点
    mPlotObj->clearPoints();    // 先清空所有数据

    std::vector<MasEspStepDataStr> stepList = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList;
    mPlotObj->addPoint(0,0);
    // 自定义规则，每秒增长2bar
    float slope = 2.0;  // 斜率
    PointF PrePoint({0,0}); // 上一个点
    for(int i=0; i<stepList.size(); i++){
        
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
    if(stepList.size() > 0)mMasEspPopPlotView->setLimits(0,PrePoint.x, 0, 20);

    mMasEspPopPlotView->invalidate();
}

void PopMasterEspresso::onBtnClickListener(View&v){
    LOGE("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_enter:{
            MasEspStepDataStr &stepData = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
            stepData.pressure = mMasEspPickerPressure->getValue();
            stepData.water = mMasEspPickerWater->getValue()*5;
            g_objConf->setMasEspData(mMasEspData);
            if(mModeEnterCallback) mModeEnterCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_cancel:{
            if(mModeCancelCallback) mModeCancelCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_step_add:{
            mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.push_back({9,30});
            setStepData();
            setPlotViewData();
            break;
        }case t5_ai_coffee::R::id::pop_step_delete:{
            std::vector<MasEspStepDataStr> &stepList = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList;
            stepList.erase(stepList.begin()+mStepSelectPos);
            if(mStepSelectPos == stepList.size()) mStepSelectPos = stepList.size()-1;
            setStepData();
            setPlotViewData();
            break;
        }case t5_ai_coffee::R::id::pop_picker:{
            // g_windMgr->showKeyBoardPage(mMasEspData.sndModeList.at(mPickerSelectPos).sndModename,"请输入自定义名称",20,
            // [this](std::string inputData){
            //     mMasEspData.sndModeList.at(mPickerSelectPos).sndModename = inputData;
            //     mPopPicker->setFormatter([this](int value){ return mMasEspData.sndModeList.at(value).sndModename; });
            // },nullptr);
            break;
        }case t5_ai_coffee::R::id::pop_reset_group:{
            mMasEspData.sndModeList.at(mPickerSelectPos) = g_objConf->getMasEspData(true).sndModeList.at(mPickerSelectPos);
            mPopPicker->setFormatter([this](int value){ return mMasEspData.sndModeList.at(value).sndModename; });
            // mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList = g_objConf->getMasEspData().sndModeList.at(mPickerSelectPos).stepDataList;
            setStepData();
            setPlotViewData();
            mResetAnimator->start();
            break;
        }
    }
}

void PopMasterEspresso::onMasEspStepBtnClickListener(View&v){
    MasEspStepDataStr &stepData = mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
    stepData.pressure = mMasEspPickerPressure->getValue();
    stepData.water = mMasEspPickerWater->getValue()*5;

    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_extraction_step1:{ mStepSelectPos = 0; break; }
        case t5_ai_coffee::R::id::pop_extraction_step2:{ mStepSelectPos = 1; break; }
        case t5_ai_coffee::R::id::pop_extraction_step3:{ mStepSelectPos = 2; break; }
        case t5_ai_coffee::R::id::pop_extraction_step4:{ mStepSelectPos = 3; break; }
        case t5_ai_coffee::R::id::pop_extraction_step5:{ mStepSelectPos = 4; break; }
    }
    mMasEspPopStepDelete->setTranslationY(75+45*mStepSelectPos);
    mStepSelectView->getLayoutParams()->height=45;
    mStepSelectView->setSelected(false);
    
    v.getLayoutParams()->height=54;
    v.setSelected(true);
    mStepSelectView->requestLayout();
    v.requestLayout();
    mStepSelectView = (TextView *)&v;

    mMasEspPickerPressure->setValue(mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos).pressure);
    mMasEspPickerWater->setValue(mMasEspData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos).water/5);
}

void PopMasterEspresso::onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue){
    switch(picker.getId()){
        case t5_ai_coffee::R::id::pop_picker:{
            mPickerSelectPos = newValue;
            mMasEspData.sndModeList.at(oldValue).stepDataList = g_objConf->getMasEspData().sndModeList.at(oldValue).stepDataList;
            setStepData();
            setPlotViewData();
            break;
        }case t5_ai_coffee::R::id::pop_picker_pressure:
        case t5_ai_coffee::R::id::pop_picker_water:{
            std::string stepText;
            switch(mStepSelectPos){
                case 0:{ stepText = "①"; break; }
                case 1:{ stepText = "②"; break; }
                case 2:{ stepText = "③"; break; }
                case 3:{ stepText = "④"; break; }
                case 4:{ stepText = "⑤"; break; }
            }

            stepText += fillLength(mMasEspPickerPressure->getValue(),2," ")+" bar-"+fillLength(mMasEspPickerWater->getValue()*5,2," ")+" ml";
            mStepSelectView->setText(stepText);
            mPopLayout->removeCallbacks(mSetPlotDataRun);
            mPopLayout->postDelayed(mSetPlotDataRun,500);
            break;
        }
    }
}

void PopMasterEspresso::setWarnText(){
    std::string warnText,warnCode;
    // if(g_appData.ErrorCode & BIT0){
    //     warnCode = "E7";
    //     warnText = "通讯故障";
    //     LOGI("通讯故障...");
    // }else if(g_appData.ErrorCode & BIT1){
    //     warnCode = "E6";
    //     warnText = "前置增压泵故障";
    //     LOGI("前置增压泵故障...");
    // }else if(g_appData.ErrorCode & BIT2){
    //     warnCode = "E5";
    //     warnText = "软水故障，请联系客服";
    //     LOGI("软水故障，请联系客服...");
    // }else if(g_appData.ErrorCode & BIT3){
    //     warnCode = "E2";
    //     warnText = "长时间制水提醒，点击确认后恢复";
    //     LOGI("长时间制水提醒，点击确认后恢复...");
    // }else if(g_appData.ErrorCode & BIT4){
    //     warnCode = "E1";
    //     warnText = "漏水故障";
    //     LOGI("漏水故障...");
    // }else if(g_appData.ErrorCode & BIT5){
    //     warnCode = "Ex";
    //     warnText = "流量计故障";
    //     LOGI("流量计故障...");
    // }else if(g_appData.ErrorCode & BIT6){
    //     warnCode = "Exx";
    //     warnText = "温度探头故障";
    //     LOGI("温度探头故障...");
    // }else if(g_appData.ErrorCode & BIT7){
    //     warnCode = "E3";
    //     warnText = "前端净水故障，请联系客服";
    //     LOGI("前端净水故障，请联系客服...");
    // }else if(g_appData.ErrorCode & BIT8){
    //     warnCode = "E4";
    //     warnText = "终端净水故障";
    //     LOGI("终端净水故障，请联系客服...");
    // }else if(g_appData.ErrorCode & BIT9){
    //     warnCode = "E8";
    //     warnText = "缺水故障";
    //     LOGI("缺水故障...");
    // }else if(g_appData.ErrorCode & BIT30){
    //     warnCode = "E9";
    //     warnText = "盐箱缺水故障";
    //     LOGI("盐箱缺水故障...");
    //     mPopBtnConfirm->setText("我知道了");
    //     mPopBtnCancel->setVisibility(View::GONE);
    // }else if(g_appData.ErrorCode & BIT31){
    //     warnCode = "盐水箱缺盐";
    //     warnText = "请加软水专用盐";
    //     mPopBtnConfirm->setText("我知道了");
    //     mPopBtnCancel->setVisibility(View::GONE);
    //     LOGI("盐箱缺盐故障...");
    // }
    
    // mPopTextTitle->setText(warnCode);
    // mPopTextContent->setText(warnText);
    // mPopTextContent->setVisibility(View::VISIBLE);
}



