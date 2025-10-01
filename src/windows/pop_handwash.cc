#include "pop_handwash.h"
#include "R.h"
#include "wind_mgr.h"
#include "project.h"
#include "proto.h"
#include "gaussfilterdrawable.h"
#include "conf_mgr.h"

PopHandWash::PopHandWash(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb,popModeCb cencelCb)
    :PopBase(wind_pop_box,"@layout/pop_layout_handwash",POP_TYPE_HAND_WASH,enterCb,cencelCb),mPickerSelectPos(selectPos),mStepSelectPos(selectStep){

    initPopView();

    mPopLayout->setSoundEffectsEnabled(false); // 关闭点击音效
    mPopLayout->setBackground(new GaussFilterDrawable(gaussFromView,Rect::Make(0,0,-1,-1),50,3,0x11bda279));
}

PopHandWash::~PopHandWash() {
    mResetAnimator->cancel();
    delete mResetAnimator;
}

void PopHandWash::initPopView(){
    mPopPicker = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker);

    mPopStep1 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_0);
    mPopStep2 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_1);
    mPopStep3 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_2);
    mPopStep4 = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_3);

    mPopStepAdd = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_add);
    mPopStepDelete = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_step_delete);

    mPopPickerWater = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_water);
    mPopPickerFlowRate = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_flowrate);
    mPopPickerTime = (NumberPicker *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_picker_time);

    mPopResetAnim = (ImageView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_anim);
    mPopBtnConfirm   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_enter);
    mPopBtnCancel   = (TextView *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_cancel);
    // mPopGaussBox = (ViewGroup *)mPopLayout->findViewById(t5_ai_coffee::R::id::pop_group);
    
    initPopData();

    auto clickFunc = std::bind(&PopHandWash::onBtnClickListener,this,std::placeholders::_1);
    auto clickStepFunc = std::bind(&PopHandWash::onStepBtnClickListener,this,std::placeholders::_1);
    mPopStep1->setOnClickListener(clickStepFunc);
    mPopStep2->setOnClickListener(clickStepFunc);
    mPopStep3->setOnClickListener(clickStepFunc);
    mPopStep4->setOnClickListener(clickStepFunc);
    mPopStepAdd->setOnClickListener(clickFunc);
    mPopStepDelete->setOnClickListener(clickFunc);
    mPopLayout->setOnClickListener(clickFunc);  // 防止点击穿透
    mPopBtnConfirm->setOnClickListener(clickFunc);
    mPopBtnCancel->setOnClickListener(clickFunc);
    mPopLayout->findViewById(t5_ai_coffee::R::id::pop_reset_group)->setOnClickListener(clickFunc);
    // mPopPicker->setOnClickListener(clickFunc);
    mPopPicker->setOnValueChangedListener(std::bind(&PopHandWash::onPickerValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    mPopLayout->setSoundEffectsEnabled(false);
    mResetAnimator = ValueAnimator::ofInt({0,10000});
    mResetAnimator->setDuration(1000);
    mResetAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
    mResetAnimator->setRepeatCount(0);       // 设置动画重复次数
    mResetAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        const int t = GET_VARIANT(anim.getAnimatedValue(),int);
        mPopResetAnim->getDrawable()->setLevel(t);
        // LOGV("position = %f",position); 
    }));

    // 旋转时有锯齿
    ((BitmapDrawable *)((RotateDrawable *)mPopResetAnim->getDrawable())->getDrawable())->setFilterBitmap(true);
}

void PopHandWash::initPopData(){
    mHandWashData = g_objConf->getHandWashData();
    mPopPicker->setMaxValue(mHandWashData.sndModeList.size()-1);
    mPopPickerWater->setMinValue(mHandWashData.extractWaterMin/mHandWashData.extractWaterGear);
    mPopPickerWater->setMaxValue(mHandWashData.extractWaterMax/mHandWashData.extractWaterGear);
    mPopPickerFlowRate->setMinValue(0);
    mPopPickerFlowRate->setMaxValue(2);
    mPopPickerTime->setMinValue(mHandWashData.interTimeMin);
    mPopPickerTime->setMaxValue(mHandWashData.interTimeMax);

    mPopPicker->setFormatter([this](int value){ return mHandWashData.sndModeList.at(value).sndModename; });
    mPopPickerWater->setFormatter([this](int value){ return std::to_string(value*mHandWashData.extractWaterGear); });
    mPopPickerFlowRate->setFormatter([this](int value){ 
        switch(value){
            case 0:{return std::string("慢");}
            case 1:{return std::string("中");}
            case 2:{return std::string("快");}
            default:{ LOGE("mPopPickerFlowRate value[%d] is error !",value);return std::string("慢");}
        }
    });
    mPopPicker->setValue(mPickerSelectPos);

    if(mStepSelectPos == mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.size()){
        mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.push_back({40,1,10});
    }
    setStepData();
}

void PopHandWash::setStepData(){
    TextView *stepView;
    std::string stepText;
    mPopStep1->setVisibility(View::GONE);
    mPopStep2->setVisibility(View::GONE);
    mPopStep3->setVisibility(View::GONE);
    mPopStep4->setVisibility(View::GONE);
    mPopStep1->setSelected(false);
    mPopStep2->setSelected(false);
    mPopStep3->setSelected(false);
    mPopStep4->setSelected(false);
    std::vector<FormStepDataStr> stepList = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList;
    switch(stepList.size()){
        case 1:{
            mPopStep1->setVisibility(View::VISIBLE);
            break; 
        }case 2:{
            mPopStep1->setVisibility(View::VISIBLE);
            mPopStep2->setVisibility(View::VISIBLE);
            break; 
        }case 3:{
            mPopStep1->setVisibility(View::VISIBLE);
            mPopStep2->setVisibility(View::VISIBLE);
            mPopStep3->setVisibility(View::VISIBLE);
            break; 
        }case 4:{
            mPopStep1->setVisibility(View::VISIBLE);
            mPopStep2->setVisibility(View::VISIBLE);
            mPopStep3->setVisibility(View::VISIBLE);
            mPopStep4->setVisibility(View::VISIBLE);
            break; 
        }
    }
    switch(mStepSelectPos){
        case 0:{ mStepSelectView = mPopStep1; break; }
        case 1:{ mStepSelectView = mPopStep2; break; }
        case 2:{ mStepSelectView = mPopStep3; break; }
        case 3:{ mStepSelectView = mPopStep4; break; }
    }
    mStepSelectView->setSelected(true);
    
    mPopPickerWater->setValue(stepList.at(mStepSelectPos).water/mHandWashData.extractWaterGear);
    mPopPickerFlowRate->setValue(stepList.at(mStepSelectPos).flowRate);
    mPopPickerTime->setValue(stepList.at(mStepSelectPos).interTime);
    
    if(stepList.size() >= 4) mPopStepAdd->setVisibility(View::GONE);
    else                     mPopStepAdd->setVisibility(View::VISIBLE);
    
    if(mStepSelectPos > 0 ){
        mPopStepDelete->setVisibility(View::VISIBLE);
        mPopStepDelete->setTranslationY(88+mStepSelectPos*49);
    }else{
        mPopStepDelete->setVisibility(View::INVISIBLE);
    }
}

void PopHandWash::onBtnClickListener(View&v){
    LOGE("onBtnClickListener v.getId() = %d",v.getId());
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_enter:{
            FormStepDataStr &stepData = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
            stepData.water = mPopPickerWater->getValue()*mHandWashData.extractWaterGear;
            stepData.flowRate = mPopPickerFlowRate->getValue();
            stepData.interTime = mPopPickerTime->getValue();
            g_objConf->setHandWashData(mHandWashData);
            if(mModeEnterCallback) mModeEnterCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_cancel:{
            if(mModeCancelCallback) mModeCancelCallback(mPickerSelectPos);
            g_windMgr->dealClosePopPage();
            break;
        }case t5_ai_coffee::R::id::pop_step_add:{
            mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.push_back({120,1,10});
            mStepSelectPos = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.size()-1;
            setStepData();
            break;
        }case t5_ai_coffee::R::id::pop_step_delete:{
            std::vector<FormStepDataStr> &stepList = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList;
            stepList.erase(stepList.begin()+mStepSelectPos);
            if(mStepSelectPos == stepList.size()) mStepSelectPos = stepList.size()-1;
            setStepData();
            break;
        }case t5_ai_coffee::R::id::pop_picker:{
            // g_windMgr->showKeyBoardPage(mHandWashData.sndModeList.at(mPickerSelectPos).sndModename,"请输入自定义名称",20,
            // [this](std::string inputData){
            //     mHandWashData.sndModeList.at(mPickerSelectPos).sndModename = inputData;
            //     mPopPicker->setFormatter([this](int value){ return mHandWashData.sndModeList.at(value).sndModename; });
            // },nullptr);
            break;
        }case t5_ai_coffee::R::id::pop_reset_group:{
            std::vector<FormStepDataStr> &stepList = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList;
            // mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList = g_objConf->getHandWashData().sndModeList.at(mPickerSelectPos).stepDataList;
            mHandWashData.sndModeList.at(mPickerSelectPos) = g_objConf->getHandWashData(true).sndModeList.at(mPickerSelectPos);
            mPopPicker->setFormatter([this](int value){ return mHandWashData.sndModeList.at(value).sndModename; });
            if(mStepSelectPos >= stepList.size()) mStepSelectPos = stepList.size()-1;
            setStepData();
            mResetAnimator->start();
            break;
        }
    }
}

void PopHandWash::onStepBtnClickListener(View&v){
    FormStepDataStr &stepData = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
    stepData.water = mPopPickerWater->getValue()*mHandWashData.extractWaterGear;
    stepData.flowRate = mPopPickerFlowRate->getValue();
    stepData.interTime = mPopPickerTime->getValue();
    LOGE("water = %d flowRate = %d interTime = %d mStepSelectPos = %d",stepData.water,stepData.flowRate,stepData.interTime,mStepSelectPos);
    switch(v.getId()){
        case t5_ai_coffee::R::id::pop_step_0:{ mStepSelectPos = 0; break; }
        case t5_ai_coffee::R::id::pop_step_1:{ mStepSelectPos = 1; break; }
        case t5_ai_coffee::R::id::pop_step_2:{ mStepSelectPos = 2; break; }
        case t5_ai_coffee::R::id::pop_step_3:{ mStepSelectPos = 3; break; }
    }

    if(mStepSelectPos > 0 ){
        mPopStepDelete->setVisibility(View::VISIBLE);
        mPopStepDelete->setTranslationY(88+mStepSelectPos*49);
    }else{
        mPopStepDelete->setVisibility(View::INVISIBLE);
    }        
    mStepSelectView->setSelected(false);
    v.setSelected(true);
    mStepSelectView = (TextView *)&v;

    const FormStepDataStr UpdateStepData = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.at(mStepSelectPos);
    mPopPickerWater->setValue(UpdateStepData.water/mHandWashData.extractWaterGear);
    mPopPickerFlowRate->setValue(UpdateStepData.flowRate);
    mPopPickerTime->setValue(UpdateStepData.interTime);
    LOGE("water = %d flowRate = %d interTime = %d mStepSelectPos = %d",UpdateStepData.water,UpdateStepData.flowRate,UpdateStepData.interTime,mStepSelectPos);
}

void PopHandWash::onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue){
    switch(picker.getId()){
        case t5_ai_coffee::R::id::pop_picker:{
            mPickerSelectPos = newValue;
            mHandWashData.sndModeList.at(oldValue).stepDataList = g_objConf->getHandWashData().sndModeList.at(oldValue).stepDataList;
            mStepSelectPos = mHandWashData.sndModeList.at(mPickerSelectPos).stepDataList.size()-1;
            setStepData();
            break;
        }
    }
}



