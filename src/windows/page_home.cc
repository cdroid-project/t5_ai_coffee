#include "page_home.h"
#include "conf_mgr.h"

#include "wind_mgr.h"
#include "gaussfilterdrawable.h"
#include <random>
#include <core/inputeventsource.h>
#include "conn_mgr.h"
#include "tuya_os_mgr.h"

class PRESSURE_VISUAL_PROGRESS:public Property{
public:
    PRESSURE_VISUAL_PROGRESS():Property("pressure_visual_progress"){}
    void set(void*object,const AnimateValue&value)override {
        int fv = GET_VARIANT(value,int);
        ((PageHome*)object)->mVisualProgress = fv;
        int level = 10000*(fv/200.0)*(290/360.0);
        // int pressure = fv*(360.0/300)/10000.0*20;
        ((PageHome*)object)->mRightWorkingAnimView->getBackground()->setLevel(level/10);
        ((PageHome*)object)->mRightInfoTv->setText(floatDataDecimalLength(fv/100.0,1));
        LOGV("setlevel = %d  level = %d",fv,level);
    }
    AnimateValue get(void*object) override{
        // return ((PageHome*)object)->mRightWorkingAnimView->getBackground()->getLevel();
        return ((PageHome*)object)->mVisualProgress;
    }
};

PageHome::PageHome(ViewGroup *wind_page_box)
    : PageBase(wind_page_box,"@layout/page_home",PAGE_HOME) {
    
    mPageEditType = HOME_PAGE_NORMAL;
    g_appData.machineState = MC_STATE_NONE;
    mGroupType = -1;
    mWorkingTime = 0;
    mWorkStartTime = 0;
    mLastUpdateHorPickerTime = 0;
    mModeBeanGrindMode = 0;
    mIsShowBeanWarn = false;
    g_objConf->getHomeTabList(mModeData);
    mLastProgressAnimator = nullptr;

    if(Property::fromName("pressure_visual_progress")==nullptr){
        Property::reigsterProperty("pressure_visual_progress",new PRESSURE_VISUAL_PROGRESS());
    }
    initPageView();
    initPageData();
}

PageHome::~PageHome() {
    mResetAnimator->cancel();
    delete mResetAnimator;
    if(g_appData.displayMode){
        delete mDislaySteamAnimator;
        delete mDislayHotWaterAnimator;
    }
    if(mLastProgressAnimator){
        mLastProgressAnimator->cancel();
        delete mLastProgressAnimator;
    }
    mWindPageBox->removeCallbacks(mWorkingRun);
    mRightAnim->stop();
    mPowderAnim->stop();

    delete mEspMgr;
    delete mAmericanoMgr;
    delete mMasEspMgr;
    delete mHandWashMgr;
    delete mTeaMgr;
    delete mFavMgr;
}

void PageHome::initPageView(){
    mHorScrollLayout    = (LinearLayout *)mPageLayout->findViewById(kaidu_ms7_lqy::R::id::hor_scroll_layout);

    mHomeTabAnimBg = mPageLayout->findViewById(kaidu_ms7_lqy::R::id::tab_anim_bg);
    mCoffeeGroup = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::layout_coffee);
    mTeaListGroup = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::layout_tea_list);
    mFavListGroup = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::layout_fav_list);

    mArcPowder      = (ArcSeekBar *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::arc_powder);
    mArcRight       = (ArcSeekBar *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::arc_right);

    mInfoRVPicker   = (RVNumberPicker *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::info_picker);

    mPowderInfoTitleTv = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_title_tv);
    mPowderInfoTv   = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_info_tv);
    mPowderInfoUnitsTv = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_units_tv);
    mRightInfoTv    = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_info_tv);
    mRightInfoTitleTv = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_info_title_tv);
    mRightInfoUnitsTv = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_units_tv);
    mRightInfoWorkTimeTv = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_info_work_time_tv);

    mOneBeanImg     = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::icon_one_bean);
    mDoubleBeanImg  = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::icon_double_bean);
    mOneCupImg      = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::icon_one_cup);
    mDoubleCupImg   = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::icon_double_cup);
    mTeaRebrewImg   = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::icon_tea_rebrew);
    mFavBackImg     = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::fav_back_img);
    mFavEnterImg     = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::fav_enter_img);
    mFavNameTv      = (TextView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::fav_name_tv);

    mPowderReduceImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_reduce);
    mPowderAddImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_add);
    mRightInfoReduceImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_info_reduce);
    mRightInfoAddImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_info_add);

    mHomeFavGroup = (ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::hand_wash_fav_group);
    mHomeTipsGroup = (ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::home_tips_group);
    mHomeFavImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::hand_wash_fav_img);
    mHomeTipsImg = (ImageView *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::hand_wash_tips_img);

    mPowderProgress = (ProgressBar *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_progress_anim);
    mRightProgress  = (ProgressBar *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_progress_anim);
    
    mPowderAnimView = mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_anim);
    mRightAnimView = mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_anim);
    mRightWorkingAnimView = mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_working_anim);

    mPowderProgress->setVisibility(View::GONE);
    mRightProgress->setVisibility(View::GONE);
    mRightWorkingAnimView->setVisibility(View::GONE);
#ifdef CDROID_X64
    mPowderAnimView->setBackgroundResource("../apps/kaidu_ms7_lqy/docs/home_anims.webp");
    mRightAnimView->setBackgroundResource("../apps/kaidu_ms7_lqy/docs/home_anims.webp");
#else
    mPowderAnimView->setBackgroundResource("./home_anims.webp");
    mRightAnimView->setBackgroundResource("./home_anims.webp");
#endif

    mPowderAnim = (AnimatedImageDrawable *)mPowderAnimView->getBackground();
    mRightAnim  = (AnimatedImageDrawable *)mRightAnimView->getBackground();
    mPowderAnim->setRepeatCount(1);
    mRightAnim->setRepeatCount(1);

    LayoutTransition *appearTransition =  new LayoutTransition();
    appearTransition->setDuration(600);
    mCoffeeGroup->setLayoutTransition(appearTransition);

    ((ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::right_group))->setVisibility(View::VISIBLE);
    ((ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::powder_group))->setVisibility(View::VISIBLE);
    
    mPowderWorkingListener.onAnimationEnd = [this](Animator& animation, bool isReverse){
        // mPowderWorkingAnimView->animate().alpha( mPowderWorkingAnimView->getAlpha() >=1.f?0.4:1.f).start();
    };
    mRightWorkingListener.onAnimationEnd = [this](Animator& animation, bool isReverse){
        // mRightWorkingAnimView->animate().alpha( mRightWorkingAnimView->getAlpha() >=1.f?0.4:1.f).start();
    };
    // mPowderWorkingAnimView->animate().setListener(mPowderWorkingListener).setDuration(1500);
    // mRightWorkingAnimView->animate().setListener(mRightWorkingListener).setDuration(1500);
    
    TextView *child;
    for(int i=0; i<mHorScrollLayout->getChildCount(); i++){
        child = (TextView *)mHorScrollLayout->getChildAt(i);
        child->setText(mModeData.at(i).modeName);
        child->setActivated(false);
        child->jumpDrawablesToCurrentState();
        child->setOnClickListener([this,i,child](View &v){
            if(g_appData.machineState != MC_STATE_NONE){
                LOGE("is working ! dont't change tab");
                showToast("工作中不能切换页面",-55);
                return;
            }else if(g_appData.homeTabSelectMode == i){
                return;
            }
            mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(false);
            g_appData.homeTabSelectMode = i;
            // child->setActivated(true);
            int duration = std::abs(mHomeTabAnimBg->getTranslationX() - 255*i);
            mHomeTabAnimBgListener.onAnimationEnd = [this,i](Animator& animation, bool isReverse){
                if(i == g_appData.homeTabSelectMode) mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(true);
            };
            mHomeTabAnimBg->animate().cancel();
            mHomeTabAnimBg->animate().translationX(255*i).setDuration(duration).setListener(mHomeTabAnimBgListener).start();
            mPageEditType = HOME_PAGE_NORMAL;
            setGroupType(mModeData.at(i).modeType);
        });
    }

    child = (TextView *)mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode);
    child->setActivated(true);
    child->setTypeface(Typeface::create("@font/HarmonyOS_Sans_SC_Medium", Typeface::BOLD));   // 需完善

    mInfoRVPicker->setConvertList({
        {0.f,        0, RVNumberPicker::TRANSFER_ABS,   0, RVNumberPicker::TRANSFER_RELATIVE},
        {0.15f,     -5, RVNumberPicker::TRANSFER_ABS,   5, RVNumberPicker::TRANSFER_RELATIVE},
        {0.50000f,  -30, RVNumberPicker::TRANSFER_ABS,   0, RVNumberPicker::TRANSFER_RELATIVE},
        {0.6f,      -50, RVNumberPicker::TRANSFER_ABS,   0, RVNumberPicker::TRANSFER_RELATIVE}
    });
    mInfoRVPicker->setOnCenterViewChangeListener([this](RVNumberPicker&picker,int oldValue,int newValue){
        if(g_appData.machineState != MC_STATE_EXT){
            LOGI("oldValue = %d  newValue = %d",oldValue,newValue);
            updateRightInfoGroup(newValue);
        }
    });

    mInfoRVPicker->setItemAnimator(nullptr);
    mInfoRVPicker->setOnItemClickListener([this](RVNumberPicker&rv, View&v, int Pos){
        int pickerValue = mInfoRVPicker->getValue();
        if((Pos < pickerValue) && (pickerValue != mInfoRVPicker->getMinValue())){
            mInfoRVPicker->setValue(pickerValue-1,true);
            playSound(0);
        }else if((Pos > pickerValue) && (pickerValue != mInfoRVPicker->getMaxValue())){
            mInfoRVPicker->setValue(pickerValue+1,true);
            playSound(0);
        }
    });
    mInfoRVPicker->setSoundEffectsEnabled(false);
    mInfoRVPicker->setOnTouchListener([this](View &v ,MotionEvent &event){
        return  g_appData.machineState & (MC_STATE_EXT | MC_STATE_HOT_WATER);
    });

    auto childDealClickFunc = std::bind(&PageHome::onChildDealClickListener,this,std::placeholders::_1);

    mArcPowder->setOnChangeListener2(std::bind(&PageHome::onArcValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    mArcRight->setOnChangeListener2(std::bind(&PageHome::onArcValueChangeListener,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    mOneBeanImg->setOnClickListener(childDealClickFunc);
    mDoubleBeanImg->setOnClickListener(childDealClickFunc);
    mOneCupImg->setOnClickListener(childDealClickFunc);
    mDoubleCupImg->setOnClickListener(childDealClickFunc);
    mTeaRebrewImg->setOnClickListener(childDealClickFunc);
    mFavBackImg->setOnClickListener(childDealClickFunc);
    mFavEnterImg->setOnClickListener(childDealClickFunc);

    mPowderReduceImg->setOnClickListener(childDealClickFunc);
    mPowderAddImg->setOnClickListener(childDealClickFunc);
    
    mHomeFavImg->setOnClickListener(childDealClickFunc);
    mHomeTipsImg->setOnClickListener(childDealClickFunc);

    mRightInfoReduceImg->setOnClickListener(childDealClickFunc);
    mRightInfoAddImg->setOnClickListener(childDealClickFunc);
    
}

void PageHome::initPageData(){
    mEspMgr = new HomeTabModeEsp(mWindPageBox);
    mAmericanoMgr = new HomeTabModeAmericano(mWindPageBox);
    mMasEspMgr = new HomeTabModeMasEsp(mWindPageBox);
    mHandWashMgr = new HomeTabModeHandWash(mWindPageBox);
    mTeaMgr = new HomeTabModeTea(mWindPageBox);
    mFavMgr = new HomeTabModeFav(mWindPageBox,std::bind(&PageHome::onFavModeEditListener,this,std::placeholders::_1,std::placeholders::_2));

    mPicInfoGroup = (ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::info_layout_cup_pic);
    mPlotInfoGroup = (ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::info_layout_plot);
    mFormInfoGroup = (ViewGroup *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::info_layout_form);
    mInfoTouchView = (View *)mCoffeeGroup->findViewById(kaidu_ms7_lqy::R::id::info_layout_touch_view);

    mCupResetGroup = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::cup_info_reset_group);
    mCupInfoFavImg = (ImageView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::cup_info_favorites);
    mCupShadowImg  = (ImageView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::cup_shadow_img);
    mPlotInfoFavImg= (ImageView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::info_plot_favorites);
    mInfoImg = (ImageView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::cup_info_img);
    mResetTv = (TextView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::btn_reset);
    mHotWaterTv = (TextView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::btn_hot_water);
    mSteamTv = (TextView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::btn_steam);
    mFormInfoHorPicker = (NumberPicker *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::info_form_hor_picker);
    mFormGroup      = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_layout);
    mFormStepGroup_0 = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_info_step_heating);
    mFormStepGroup_1 = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_info_step_1);
    mFormStepGroup_2 = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_info_step_2);
    mFormStepGroup_3 = (ViewGroup *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_info_step_3);
    mFormStepAddImg = (ImageView *)mWindPageBox->findViewById(kaidu_ms7_lqy::R::id::form_step_add);

    auto childDealClickFunc = std::bind(&PageHome::onChildDealClickListener,this,std::placeholders::_1);
    mCupInfoFavImg->setOnClickListener(childDealClickFunc);    
    mPlotInfoFavImg->setOnClickListener(childDealClickFunc);
    mFormStepGroup_0->setOnClickListener(childDealClickFunc);
    mFormStepGroup_1->setOnClickListener(childDealClickFunc);
    mFormStepGroup_2->setOnClickListener(childDealClickFunc);
    mFormStepGroup_3->setOnClickListener(childDealClickFunc);
    mFormStepAddImg->setOnClickListener(childDealClickFunc);
    mResetTv->setOnClickListener(childDealClickFunc);
    mHotWaterTv->setOnClickListener(childDealClickFunc);
    mSteamTv->setOnClickListener(childDealClickFunc);

    mHotWaterTv->setSoundEffectsEnabled(false);
    mSteamTv->setSoundEffectsEnabled(false);
    
    mResetAnimator = ValueAnimator::ofInt({0,10000});
    mResetAnimator->setDuration(1000);
    mResetAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
    mResetAnimator->setRepeatCount(0);       // 设置动画重复次数
    mResetAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        const int t = GET_VARIANT(anim.getAnimatedValue(),int);
        mResetTv->getCompoundDrawables().at(TextView::Drawables::LEFT)->setLevel(t);
        // LOGV("position = %f",position); 
    }));

    if(g_appData.displayMode){
        mDislaySteamAnimator = ValueAnimator::ofInt({0,10000});
        mDislaySteamAnimator->setDuration(5000);
        mDislaySteamAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
        mDislaySteamAnimator->setRepeatCount(-1);       // 设置动画重复次数
        mDislaySteamAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
            if(mGroupType == HOME_MT_ESPRESSO || mGroupType ==  HOME_MT_AMERICANO)
                mSteamTv->getBackground()->setLevel(GET_VARIANT(anim.getAnimatedValue(),int));
        }));
        mDislayHotWaterAnimator = ValueAnimator::ofInt({0,10000});
        mDislayHotWaterAnimator->setDuration(2000);
        mDislayHotWaterAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
        mDislayHotWaterAnimator->setRepeatCount(-1);       // 设置动画重复次数
        mDislayHotWaterAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
            if(mGroupType == HOME_MT_ESPRESSO || mGroupType ==  HOME_MT_AMERICANO)
                mHotWaterTv->getBackground()->setLevel(GET_VARIANT(anim.getAnimatedValue(),int));
        }));
        mDislaySteamAnimator->start();
        mDislayHotWaterAnimator->start();
    }

    mFormHorPickerState = NumberPicker::OnScrollListener::SCROLL_STATE_IDLE;
    mFormInfoHorPicker->setOnValueChangedListener([this](NumberPicker&picker,int oldValue,int newValue){
        int64_t nowTime = SystemClock::uptimeMillis();
        if(((nowTime - mLastUpdateHorPickerTime) > 150) || (mFormHorPickerState == NumberPicker::OnScrollListener::SCROLL_STATE_IDLE)){
            mLastUpdateHorPickerTime = nowTime;
            mHorPickerUpdateRun();
        }
    });

    // mInfoTouchView->setOnTouchListener([this](View&v, MotionEvent&event){
    //     if(g_appData.machineState != MC_STATE_NONE){
    //         LOGI("is working ! dont't change tab");
    //         // if(event.getAction() == MotionEvent::ACTION_UP)showToast("工作中不能切换页面");
    //         return true;
    //     }else if(mPageEditType != HOME_PAGE_NORMAL){
    //         LOGI("收藏页面，信息栏不可操作");
    //         return true;
    //     }
    //     LOGE("is working ! dont't change tab");
    //     return false;
    // });
    mFormGroup->setOnTouchListener([this](View&v, MotionEvent&event){
        if(g_appData.machineState != MC_STATE_NONE){
            LOGI("is working ! dont't change tab");
            if(event.getAction() == MotionEvent::ACTION_UP) showToast("工作中，请稍候",-55);
            return true;
        }else if(mPageEditType != HOME_PAGE_NORMAL){
            LOGI("收藏页面，信息栏不可操作");
            if(event.getAction() == MotionEvent::ACTION_UP) showToast("不可编辑",-55);
            return true;
        }
        LOGE("is working ! dont't change tab");
        return false;
    });
    
    NumberPicker::OnScrollListener scrollListener;
    scrollListener.onScrollStateChange = [this](View &v,int state){
        if(state == NumberPicker::OnScrollListener::SCROLL_STATE_IDLE){
            mWindPageBox->removeCallbacks(mHorPickerUpdateRun);
            mWindPageBox->postDelayed(mHorPickerUpdateRun,300);
        }
        mFormHorPickerState = state;
        LOGI("mFormHorPickerState = %d",mFormHorPickerState);
    };
    mFormInfoHorPicker->setOnScrollListener(scrollListener);

    mHorPickerUpdateRun = [this](){
        switch(mGroupType){
            case HOME_MT_HAND_WASHED:{
                mHandWashMgr->onHorPickerChange();
                break;
            }case HOME_MT_EXTRACTING_TEA:{
                mTeaMgr->onHorPickerChange();
                break;
            }default:{
                LOGE("mGroupType is error !!!!!!");
                break;
            }
        }
    };
    mWorkingRun = [this](){
        if(g_appData.machineState & MC_STATE_POWDER){

        }else if(g_appData.machineState & MC_STATE_EXT){
            if(g_appData.displayMode){
                // x86 运行时、商展版本，增加压力表的模拟数据
                std::random_device rd;
                std::mt19937 gen(rd());
                // 定义生成随机数的范围(修改范围，从而实现不同的杂色效果，（-2，2）效果也还可以，目前颗粒感较重，水波纹基本去掉)
                std::uniform_int_distribution<int> distribution(0, 200);
                int pressure = distribution(gen);
                g_appData.extractPressure = pressure;
                updatePressureGauge();
                g_appData.extractWater++;
                mPowderInfoTv->setText(std::to_string(g_appData.extractWater));
                if(mGroupType == HOME_MT_MASTER_ESPRESSO){
                    mMasEspMgr->updatePoltViewData();
                }
            }
            if(!mIsShowBeanWarn || ((g_appData.warnState & ~g_appData.warnIsDealState) == WARN_NONE)){
                mWorkingTime++;
                mRightInfoWorkTimeTv->setText(std::to_string(mWorkingTime)+"s");
            }
        }
        int diffTime = SystemClock::uptimeMillis() - (mWorkStartTime+1000);
        mWorkStartTime += 1000;
        mWindPageBox->postDelayed(mWorkingRun,1000-diffTime);
    };


    setGroupType(mModeData.at(g_appData.homeTabSelectMode).modeType);
}

void PageHome::updatePageData(){
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{ mEspMgr->updatePageData(); break;}
        case HOME_MT_AMERICANO:{ mAmericanoMgr->updatePageData(); break;}
        case HOME_MT_MASTER_ESPRESSO:{ mMasEspMgr->updatePageData(); break;}
        case HOME_MT_HAND_WASHED:{ mHandWashMgr->updatePageData(); break;}
        case HOME_MT_EXTRACTING_TEA:{ mTeaMgr->updatePageData(); break;}
        // case HOME_MT_FAVORITES:{ mFavMgr->updatePageData(); break;}
    }
    
    if(g_appData.statusChangeFlag & CS_EXTRACT_DATA_CHANGE){
        if(g_appData.machineState & MC_STATE_EXT){
            if(mGroupType != HOME_MT_EXTRACTING_TEA){
                // mRightInfoTv->setText(std::to_string(g_appData.extractPressure));
                mPowderInfoTv->setText(std::to_string(g_appData.extractWater));
                updatePressureGauge();
                if(mGroupType == HOME_MT_MASTER_ESPRESSO){
                    mMasEspMgr->updatePoltViewData();
                }
            }else{
                mRightInfoTv->setText(std::to_string(g_appData.extractWater));
            }
        }else if(g_appData.machineState & MC_STATE_HOT_WATER){
            mRightInfoTv->setText(std::to_string(g_appData.extractWater));
        }
    }

    if(g_appData.statusChangeFlag & CS_HOT_WATER_DOWN){
        mHotWaterTv->performClick();
        InputEventSource::getInstance().closeScreenSaver();
    }

    if(g_appData.statusChangeFlag & CS_STEAM_DOWN){
        mSteamTv->performClick();
        InputEventSource::getInstance().closeScreenSaver();
    }

    if(g_appData.statusChangeFlag & CS_EXTRACT_DOWN){
        if(!(g_appData.machineState & MC_STATE_EXT)){
            InputEventSource::getInstance().closeScreenSaver();
            if(mOneCupImg->isSelected())            mOneCupImg->performClick();
            else if(mTeaRebrewImg->isSelected())    mTeaRebrewImg->performClick();
            else                                    mDoubleCupImg->performClick();
        } 
    }

    if(g_appData.statusChangeFlag & CS_GRIND_BEAN_DATA_CHANGE){
        int maxPowder = g_appData.tuyaSetPowder<0?mArcPowder->getProgress():g_appData.tuyaSetPowder;
        if(g_appData.grindBean >= maxPowder){
            InputEventSource::getInstance().closeScreenSaver();
            if(mOneBeanImg->isSelected())   mOneBeanImg->performClick();
            else                            mDoubleBeanImg->performClick();
        }else{
            if(mModeBeanGrindMode==BEAN_GRIND_WEI){
                mPowderInfoTv->setText(std::to_string(g_appData.grindBean));
            }else{
                mPowderInfoTv->setText(std::to_string(maxPowder - g_appData.grindBean));
            }
        }
    }

    if((g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE)){
        if((g_appData.machineState != MC_STATE_NONE) && (g_appData.warnState & ~g_appData.warnIsDealState)){
            dealWarnStatus();
        }else{
            mIsShowBeanWarn = false;
        }
    }
    if(g_appData.statusChangeFlag & CS_WARN_CLOSE){
        InputEventSource::getInstance().closeScreenSaver();
        if(mIsShowBeanWarn){
            if(g_appData.machineState & MC_STATE_POWDER){
                if(mOneBeanImg->isSelected())   mOneBeanImg->performClick();
                else                            mDoubleBeanImg->performClick();
            }else if(g_appData.machineState & MC_STATE_EXT){
                if(mGroupType == HOME_MT_EXTRACTING_TEA){
                    mTeaMgr->stopWork();
                }else{
                    if(mOneCupImg->isSelected())            mOneCupImg->performClick();
                    else                                    mDoubleCupImg->performClick();
                }
            }else if(g_appData.machineState & MC_STATE_HOT_WATER){
                mHotWaterTv->performClick();
            }
        }
        mIsShowBeanWarn = false;
    }

    if(g_appData.statusChangeFlag & CS_TUYA_START_GRIND){
        if(g_appData.tuyaStartGrind){
            if(!(g_appData.machineState & (MC_STATE_POWDER | MC_STATE_EXT | MC_STATE_HOT_WATER))){
                InputEventSource::getInstance().closeScreenSaver();
                if((mGroupType == HOME_MT_FAVORITES) || (mGroupType == HOME_MT_EXTRACTING_TEA)){
                    mPageEditType = HOME_PAGE_NORMAL;
                    setGroupType(HOME_MT_ESPRESSO);
                    mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(false);
                    g_appData.homeTabSelectMode = 0;
                    // mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(true);
                    int selectPos = g_appData.homeTabSelectMode;
                    int duration = std::abs(mHomeTabAnimBg->getTranslationX() - 255*selectPos);
                    mHomeTabAnimBgListener.onAnimationEnd = [this,selectPos](Animator& animation, bool isReverse){
                        if(selectPos == g_appData.homeTabSelectMode) mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(true);
                    };
                    mHomeTabAnimBg->animate().cancel();
                    mHomeTabAnimBg->animate().translationX(255*selectPos).setDuration(duration).setListener(mHomeTabAnimBgListener).start();
                    
                }
                if(mOneBeanImg->isActivated())  mOneBeanImg->performClick();
                else                            mDoubleBeanImg->performClick();
            }
        }else{
            if(g_appData.machineState & MC_STATE_POWDER){
                InputEventSource::getInstance().closeScreenSaver();
                if(mOneBeanImg->isSelected())   mOneBeanImg->performClick();
                else                            mDoubleBeanImg->performClick();
            }
        }
    }

    if(g_appData.statusChangeFlag & CS_TUYA_START_EXT){
        dealTuyaExt();
    }

    // 涂鸦下发 测茶下一步
    // if(g_appData.statusChangeFlag & CS_TUYA_TEA_NEXT_STEP){
    //     InputEventSource::getInstance().closeScreenSaver();
    //     if(mOneCupImg->isSelected())            mOneCupImg->performClick();
    //     else if(mTeaRebrewImg->isSelected())    mTeaRebrewImg->performClick();
    //     else if(mDoubleCupImg->isSelected())    mDoubleCupImg->performClick();
    //     else                                    LOGE("*************************** 状态错误 !!!!!!!!!!!!!!!!!!!");
    // }
}

void PageHome::stopSteamWork(){
    if(g_appData.machineState & MC_STATE_STEAM){
        InputEventSource::getInstance().closeScreenSaver();
        mSteamTv->performClick();
    }
}

void PageHome::stopPageAction(){
    mPageLayout->setVisibility(View::GONE);
}

void PageHome::resetPageAction(){
    mPageLayout->setVisibility(View::VISIBLE);
    changeTitleBarStatus(0xFF);
    g_appData.statusChangeFlag |= CS_PRE_HEATING;
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{ mEspMgr->updatePageData(); break;}
        case HOME_MT_AMERICANO:{ mAmericanoMgr->updatePageData(); break;}
        case HOME_MT_MASTER_ESPRESSO:{ mMasEspMgr->updatePageData(); break;}
        case HOME_MT_HAND_WASHED:{ mHandWashMgr->updatePageData(); break;}
        case HOME_MT_EXTRACTING_TEA:{ mTeaMgr->updatePageData(); break;}
    }
    g_appData.statusChangeFlag &= ~CS_PRE_HEATING;

    if(mPageEditType == HOME_PAGE_NORMAL)   mModeBeanGrindMode = g_appData.beanGrindMode;
    
    if(mModeBeanGrindMode==BEAN_GRIND_WEI){
        mPowderInfoTitleTv->setText("粉重");
        mPowderInfoUnitsTv->setText("g");
    }else{
        mPowderInfoTitleTv->setText("磨豆");
        mPowderInfoUnitsTv->setText("s");
    }
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{ mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(mEspMgr->getCacheDataToJson())); break;}
        case HOME_MT_AMERICANO:{ mCupInfoFavImg->setActivated(g_objConf->checkFavModeData(mAmericanoMgr->getCacheDataToJson())); break;}
        case HOME_MT_MASTER_ESPRESSO:{ mPlotInfoFavImg->setActivated(g_objConf->checkFavModeData(mMasEspMgr->getCacheDataToJson())); break;}
        case HOME_MT_HAND_WASHED:{ mHomeFavImg->setActivated(g_objConf->checkFavModeData(mHandWashMgr->getCacheDataToJson())); break;}
        case HOME_MT_EXTRACTING_TEA:{ mHomeFavImg->setActivated(g_objConf->checkFavModeData(mTeaMgr->getCacheDataToJson())); break;}
    }
    // case kaidu_ms7_lqy::R::id::info_plot_favorites:
    // case kaidu_ms7_lqy::R::id::hand_wash_fav_img:
    // case kaidu_ms7_lqy::R::id::cup_info_favorites:
}

// 设置页面 Type
void PageHome::setGroupType(int type){
    mGroupType = type;
    updateGroupData();
}

// 根据页面mGroupType，更新页面类型
void PageHome::updateGroupData(){
    mPicInfoGroup->setVisibility(View::GONE);
    mPlotInfoGroup->setVisibility(View::GONE);
    mFormInfoGroup->setVisibility(View::GONE);
    mHomeTipsGroup->setVisibility(View::GONE);
    mHomeFavGroup->setVisibility(View::GONE);
    mInfoRVPicker->setVisibility(View::GONE);
    mCoffeeGroup->setVisibility(View::VISIBLE);
    mTeaListGroup->setVisibility(View::GONE);
    mFavListGroup->setVisibility(View::GONE);
    mPowderReduceImg->setVisibility(View::VISIBLE);
    mPowderAddImg->setVisibility(View::VISIBLE);
    mRightInfoReduceImg->setVisibility(View::VISIBLE);
    mRightInfoAddImg->setVisibility(View::VISIBLE);
    mFavBackImg->setVisibility(View::GONE);
    mFavEnterImg->setVisibility(View::GONE);
    mFavNameTv->setVisibility(View::GONE);
    mTeaRebrewImg->setVisibility(View::GONE);
    mArcRight->setShowSlider(true);
    mPowderAnim->restart(0);
    mRightAnim->restart(0);
    mPowderAnim->stop();
    mRightAnim->stop();
    mResetAnimator->end();
    mModeBeanGrindMode = g_appData.beanGrindMode;
    if((mGroupType != HOME_MT_EXTRACTING_TEA) && (mGroupType != HOME_MT_FAVORITES)){
        if(mModeBeanGrindMode==BEAN_GRIND_WEI){
            mPowderInfoTitleTv->setText("粉重");
            mPowderInfoUnitsTv->setText("g");
        }else{
            mPowderInfoTitleTv->setText("磨豆");
            mPowderInfoUnitsTv->setText("s");
        }
    }
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{
            ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,0);
            ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,120);
            ((RelativeLayout::LayoutParams *)mCupResetGroup->getLayoutParams())->setMargins(0,0,0,15);
            mPicInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            mEspMgr->updateGroupData();
            break;
        }case HOME_MT_AMERICANO:{
            ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,0);
            ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,120);
            ((RelativeLayout::LayoutParams *)mCupResetGroup->getLayoutParams())->setMargins(0,0,0,15);
            mPicInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            mAmericanoMgr->updateGroupData();
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            mPlotInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            mMasEspMgr->updateGroupData();
            break;
        }case HOME_MT_HAND_WASHED:{
            mFormInfoGroup->setVisibility(View::VISIBLE);
            mHomeFavGroup->setVisibility(View::VISIBLE);
            mHandWashMgr->updateGroupData();
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            mFormInfoGroup->setVisibility(View::VISIBLE);
            mHomeFavGroup->setVisibility(View::VISIBLE);
            mPowderInfoTitleTv->setText("建议克重");
            mPowderReduceImg->setVisibility(View::GONE);
            mPowderAddImg->setVisibility(View::GONE);
            mTeaMgr->updateGroupData();
            break;
        }case HOME_MT_FAVORITES:{
            mFavMgr->updateGroupData(mPageEditType != HOME_PAGE_FAV_EDIT);
            break;
        }default:{
            LOGE("mGroupType is error !!!!!!");
            break;
        }
    }

    mOneBeanImg->getBackground()->jumpToCurrentState();
    mDoubleBeanImg->getBackground()->jumpToCurrentState();
    mOneCupImg->getBackground()->jumpToCurrentState();
    mDoubleCupImg->getBackground()->jumpToCurrentState();

    mOneBeanImg->setSelected(false);
    mDoubleBeanImg->setSelected(false);
    mOneCupImg->setSelected(false);
    mDoubleCupImg->setSelected(false);
}

void PageHome::onFavModeEditListener(int Position, bool isEdit){
    mPageEditType = HOME_PAGE_FAV_EDIT;
    mPicInfoGroup->setVisibility(View::GONE);
    mPlotInfoGroup->setVisibility(View::GONE);
    mFormInfoGroup->setVisibility(View::GONE);
    mHomeTipsGroup->setVisibility(View::GONE);
    mHomeFavGroup->setVisibility(View::GONE);
    mInfoRVPicker->setVisibility(View::GONE);
    mCoffeeGroup->setVisibility(View::VISIBLE);
    mTeaListGroup->setVisibility(View::GONE);
    mFavListGroup->setVisibility(View::GONE);
    mTeaRebrewImg->setVisibility(View::GONE);
    if(isEdit){
        mPowderReduceImg->setVisibility(View::VISIBLE);
        mPowderAddImg->setVisibility(View::VISIBLE);
        mFavEnterImg->setVisibility(View::VISIBLE);
        mRightInfoReduceImg->setVisibility(View::VISIBLE);
        mRightInfoAddImg->setVisibility(View::VISIBLE);
        mOneBeanImg->setVisibility(View::GONE);
        mOneCupImg->setVisibility(View::GONE);
    }else{
        mFavEnterImg->setVisibility(View::GONE);
        if(g_appData.coffeePreheatPtc >= 100){
            mOneCupImg->setActivated(true);
        }else{
            mOneCupImg->setActivated(false);
        }
        mOneBeanImg->setActivated(true);
        mOneBeanImg->setSelected(false);
        mOneCupImg->setSelected(false);
        
        mOneBeanImg->setVisibility(View::VISIBLE);
        mOneCupImg->setVisibility(View::VISIBLE);

        mPowderReduceImg->setVisibility(View::GONE);
        mPowderAddImg->setVisibility(View::GONE);
        mRightInfoReduceImg->setVisibility(View::GONE);
        mRightInfoAddImg->setVisibility(View::GONE);
    }
    mDoubleBeanImg->setVisibility(View::GONE);
    mDoubleCupImg->setVisibility(View::GONE);

    mFavBackImg->setVisibility(View::VISIBLE);
    mFavNameTv->setVisibility(View::VISIBLE);

    mPowderAnim->restart(0);
    mRightAnim->restart(0);
    mPowderAnim->stop();
    mRightAnim->stop();
    mResetAnimator->end();

    Json::Value favData = g_objConf->getFavModeList()[Position];
    int favModeType = getJsonInt(favData,"coffeeMode");
    std::string favName = getJsonString(favData,"name");
    std::string modeName = "none";
    auto it = std::find_if(mModeData.begin(), mModeData.end(),[favModeType](const HomePageTabDataStr &p){return p.modeType == favModeType;});
    if( it != mModeData.end()){ modeName = it->modeName; }
    if(favName != modeName){ favName = modeName + " < " + favName; }
    mFavNameTv->setText(favName);
    ((RelativeLayout::LayoutParams *)mFavNameTv->getLayoutParams())->setMargins(0,49,0,0);

    mGroupType = favModeType;
    mModeBeanGrindMode = getJsonInt(favData,"beanGrindMode");
    if((mGroupType != HOME_MT_EXTRACTING_TEA) && (mGroupType != HOME_MT_FAVORITES)){
        if(mModeBeanGrindMode==BEAN_GRIND_WEI){
            mPowderInfoTitleTv->setText("粉重");
            mPowderInfoUnitsTv->setText("g");
        }else{
            mPowderInfoTitleTv->setText("磨豆");
            mPowderInfoUnitsTv->setText("s");
        }
    }
    switch(favModeType){
        case HOME_MT_ESPRESSO:{
            mPicInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            // mPowderInfoTitleTv->setText("粉重");
            if(isEdit){
                ((RelativeLayout::LayoutParams *)mFavNameTv->getLayoutParams())->setMargins(0,35,0,0);
                ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,-35);
                ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,85);
            }else{
                ((RelativeLayout::LayoutParams *)mFavNameTv->getLayoutParams())->setMargins(0,25,0,0);
                ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,-20);
                ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,100);
                ((RelativeLayout::LayoutParams *)mCupResetGroup->getLayoutParams())->setMargins(0,0,0,0);
            }
            mEspMgr->onFavModeEditListener(Position,isEdit);
            break;
        }case HOME_MT_AMERICANO:{
            mPicInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            // mPowderInfoTitleTv->setText("粉重");
            if(isEdit){
                ((RelativeLayout::LayoutParams *)mFavNameTv->getLayoutParams())->setMargins(0,35,0,0);
                ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,-35);
                ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,85);
            }else{
                ((RelativeLayout::LayoutParams *)mFavNameTv->getLayoutParams())->setMargins(0,25,0,0);
                ((RelativeLayout::LayoutParams *)mCupShadowImg->getLayoutParams())->setMargins(0,0,0,-20);
                ((RelativeLayout::LayoutParams *)mInfoImg->getLayoutParams())->setMargins(0,0,0,100);
                ((RelativeLayout::LayoutParams *)mCupResetGroup->getLayoutParams())->setMargins(0,0,0,0);
            }
            mAmericanoMgr->onFavModeEditListener(Position,isEdit);
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            mPlotInfoGroup->setVisibility(View::VISIBLE);
            mInfoRVPicker->setVisibility(View::VISIBLE);
            // mPowderInfoTitleTv->setText("粉重");
            mMasEspMgr->onFavModeEditListener(Position,isEdit);
            break;
        }case HOME_MT_HAND_WASHED:{
            mFormInfoGroup->setVisibility(View::VISIBLE);
            mHomeFavGroup->setVisibility(View::VISIBLE);
            // mPowderInfoTitleTv->setText("粉重");
            mHandWashMgr->onFavModeEditListener(Position,isEdit);
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            mFormInfoGroup->setVisibility(View::VISIBLE);
            mHomeFavGroup->setVisibility(View::VISIBLE);
            mPowderInfoTitleTv->setText("建议克重");
            mPowderReduceImg->setVisibility(View::GONE);
            mPowderAddImg->setVisibility(View::GONE);
            mTeaMgr->onFavModeEditListener(Position,isEdit);
            break;
        }default:{
            LOGE("mGroupType is error !!!!!!");
            break;
        }
    }
    
}

void PageHome::onChildDealClickListener(View &v){
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::btn_hot_water:{
            if((g_appData.coffeePreheatPtc < 100) || g_appData.displayMode){
                showToast("预热中，请稍候",-55);
                return;
            }else if((g_appData.machineState & MC_STATE_POWDER) || (g_appData.machineState & MC_STATE_EXT)){
                showToast("工作中不能出热水",-55);
                return;
            }else if((g_appData.warnState & STOP_HOT_WATER_ERROR) && !(g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && (mHotWaterTv->getBackground()->getLevel() == 0)){
                g_appData.warnIsDealState &= ~STOP_HOT_WATER_ERROR;
                g_windMgr->dealClosePopPage(false,true);
                g_windMgr->showPopPage(POP_WARN,nullptr);
                return;
            }
            break;
        }case kaidu_ms7_lqy::R::id::btn_steam:{
            if((g_appData.coffeePreheatPtc < 100) || g_appData.displayMode){
                showToast("预热中，请稍候",-55);
                return;
            }if((g_appData.warnState & STOP_STEAM_ERROR) && !(g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && (mSteamTv->getBackground()->getLevel() == 0)){
                g_appData.warnIsDealState &= ~STOP_STEAM_ERROR;
                g_windMgr->dealClosePopPage(false,true);
                g_windMgr->showPopPage(POP_WARN,nullptr);
                return;
            }
            bool isStart = mSteamTv->getBackground()->getLevel() == 0;
            if(!isStart){
                // 此次操作是完成的操作
                int steamTotal = g_objConf->getFrothTotal()+1;
                g_objConf->setFrothTotal(steamTotal);
#ifndef TUYA_OS_DISABLE
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_MILK,PROP_VALUE, &steamTotal); // 打奶泡次数
#endif
            }
            if(isStart)         g_appData.machineState |= MC_STATE_STEAM;
            else                g_appData.machineState &= ~MC_STATE_STEAM;
            mSteamTv->getBackground()->setLevel(isStart?10000:0);
            g_objConnMgr->setExtraction(isStart,EXT_MODE_STEAM,0x00,0x00,0x00,0x00,0x00,0x00);
            break;
        }case kaidu_ms7_lqy::R::id::form_info_step_heating:
        case kaidu_ms7_lqy::R::id::form_info_step_1:
        case kaidu_ms7_lqy::R::id::form_info_step_2:
        case kaidu_ms7_lqy::R::id::form_info_step_3:
        case kaidu_ms7_lqy::R::id::form_step_add:{
            if(g_appData.machineState != MC_STATE_NONE){
                showToast("工作中，请稍候",-55);
                return;
            }else if(mPageEditType != HOME_PAGE_NORMAL){
                showToast("不可编辑",-55);
                return;
            }
            break;
        }case kaidu_ms7_lqy::R::id::icon_one_cup:
        case kaidu_ms7_lqy::R::id::icon_double_cup:{
            if(g_appData.coffeePreheatPtc < 100){
                showToast("预热中，请稍候",-55);
                return;
            }else if(g_appData.machineState & MC_STATE_HOT_WATER){
                showToast("出热水中，不可操作",-55);
                return;
            }else if(g_appData.machineState & MC_STATE_POWDER){
                showToast("萃取中，不可磨豆",-55);
                return;
            }else if((g_appData.warnState & STOP_EXT_ERROR) && !(g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && !v.isSelected()){
                g_appData.warnIsDealState &= ~STOP_EXT_ERROR;
                g_windMgr->dealClosePopPage(false,true);
                g_windMgr->showPopPage(POP_WARN,nullptr);
                return;
            }else if(!v.isActivated()){
                return;
            }else{
                mResetTv->setAlpha(v.isSelected()?1.f:0.4);
                mResetTv->setEnabled(v.isSelected());
            }
            break;
        }
        case kaidu_ms7_lqy::R::id::icon_one_bean:
        case kaidu_ms7_lqy::R::id::icon_double_bean:{
            if(g_appData.machineState & MC_STATE_HOT_WATER){
                showToast("出热水中，不可操作",-55);
                return;
            }else if(g_appData.machineState & MC_STATE_EXT){
                showToast("萃取中，不可磨豆",-55);
                return;
            }if((g_appData.warnState & STOP_GRIND_BEAN_ERROR) && !(g_appData.statusChangeFlag & CS_WARN_STATUS_CHANGE) && !v.isSelected()){
                g_appData.warnIsDealState &= ~STOP_GRIND_BEAN_ERROR;
                g_windMgr->dealClosePopPage(false,true);
                g_windMgr->showPopPage(POP_WARN,nullptr);
                return;
            }else if(!v.isActivated()){
                return;
            }else{
                mResetTv->setAlpha(v.isSelected()?1.f:0.4);
                mResetTv->setEnabled(v.isSelected());
            }
            break;
        }case kaidu_ms7_lqy::R::id::powder_add:
        case kaidu_ms7_lqy::R::id::powder_reduce:{
            mArcPowder->setProgress(mArcPowder->getProgress()+(v.getId() == kaidu_ms7_lqy::R::id::powder_reduce?-1:1));
            onArcValueChangeListener(*mArcPowder,mArcPowder->getProgress(),true);
            if(!mPowderAnim->isRunning()) mPowderAnim->start();
            break;
        }case kaidu_ms7_lqy::R::id::info_plot_favorites:
        case kaidu_ms7_lqy::R::id::hand_wash_fav_img:
        case kaidu_ms7_lqy::R::id::cup_info_favorites:{
            if(!v.isActivated() && (g_objConf->getFavModeList().size() >= 10)){
                showToast("最多收藏10个",-55);
                return;
            }
            break;
        }
    }

    switch(mGroupType){
        case HOME_MT_ESPRESSO:{
            mEspMgr->onItemClickListener(v);
            break;
        }case HOME_MT_AMERICANO:{
            mAmericanoMgr->onItemClickListener(v);
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            mMasEspMgr->onItemClickListener(v);
            break;
        }case HOME_MT_HAND_WASHED:{
            mHandWashMgr->onItemClickListener(v);
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            mTeaMgr->onItemClickListener(v);
            break;
        }case HOME_MT_FAVORITES:{
            mFavMgr->onItemClickListener(v);
            break;
        }default:{
            LOGE("mGroupType is error !!!!!!");
            break;
        }
    }
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::icon_one_cup:
        case kaidu_ms7_lqy::R::id::icon_double_cup:{
            if(mGroupType != HOME_MT_EXTRACTING_TEA)dealRightInfoGroup(v);
            // if(mGroupType == HOME_MT_MASTER_ESPRESSO) mPlotInfoGroup->setAlpha(v.isSelected()?0.4:1.0);
            // else if((mGroupType == HOME_MT_HAND_WASHED) || (mGroupType == HOME_MT_EXTRACTING_TEA))
            //     mFormInfoGroup->setAlpha(v.isSelected()?0.4:1.0);
            if(mGroupType != HOME_MT_EXTRACTING_TEA)dealPowderInfoGroup(v);
            break;
        }
        case kaidu_ms7_lqy::R::id::icon_one_bean:
        case kaidu_ms7_lqy::R::id::icon_double_bean:{
            dealPowderInfoGroup(v);
            // if(mGroupType == HOME_MT_MASTER_ESPRESSO) mPlotInfoGroup->setAlpha(v.isSelected()?0.4:1.0);
            // else if((mGroupType == HOME_MT_HAND_WASHED) || (mGroupType == HOME_MT_EXTRACTING_TEA))
            //     mFormInfoGroup->setAlpha(v.isSelected()?0.4:1.0);
            break;
        }case kaidu_ms7_lqy::R::id::right_info_reduce:
        case kaidu_ms7_lqy::R::id::right_info_add:{
            if(!mRightAnim->isRunning()) mRightAnim->start();    
            break;
        }case kaidu_ms7_lqy::R::id::btn_reset:{
            if(!(g_appData.machineState & MC_STATE_POWDER || g_appData.machineState & MC_STATE_EXT)){
                mResetAnimator->start();
            }else{
                showToast("工作中,请稍候",-55);
            }
            
            break;
        }case kaidu_ms7_lqy::R::id::fav_enter_img:
        case kaidu_ms7_lqy::R::id::fav_back_img:{
            if(mPageEditType != HOME_PAGE_NORMAL){
                if(g_appData.machineState != MC_STATE_NONE){
                    LOGE("is working ! dont't change tab");
                    showToast("工作中不能切换页面",-55);
                    return;
                }
                setGroupType(HOME_MT_FAVORITES);
                mPageEditType = HOME_PAGE_NORMAL;
            }
            break;
        }case kaidu_ms7_lqy::R::id::btn_hot_water:{
            bool isStart = mHotWaterTv->getBackground()->getLevel() == 0;
            if(isStart){
                mDoubleCupImg->setActivated(false);
                mOneCupImg->setActivated(false);
                mOneBeanImg->setActivated(false);
                mDoubleBeanImg->setActivated(false);
                
                mPowderReduceImg->setVisibility(View::GONE);
                mPowderAddImg->setVisibility(View::GONE);
                mRightInfoReduceImg->setVisibility(View::GONE);
                mRightInfoAddImg->setVisibility(View::GONE);

                mArcRight->setVisibility(View::GONE);
                mArcPowder->setVisibility(View::GONE);
                mPowderProgress->setVisibility(View::VISIBLE);
                mPowderProgress->setIndeterminate(true);
                mRightProgress->setVisibility(View::VISIBLE);
                mRightProgress->setIndeterminate(true);

                mPowderAnim->setRepeatCount(-1);
                if(!mPowderAnim->isRunning()){
                    mPowderAnim->start();
                }
                mRightAnim->setRepeatCount(-1);
                if(!mRightAnim->isRunning()){
                    mRightAnim->start();
                }

                mPowderInfoTv->setText(std::to_string(g_appData.hotWaterTemp));
                mPowderInfoTitleTv->setText("温度");
                mPowderInfoUnitsTv->setText("℃");

                mRightInfoTv->setText("0");
                mRightInfoTitleTv->setText("出水量");
                mRightInfoUnitsTv->setText("ml");

                mInfoRVPicker->setSelectVisibility(View::GONE);
            }else{
                if(mPageEditType == HOME_PAGE_NORMAL){
                    mPowderReduceImg->setVisibility(View::VISIBLE);
                    mPowderAddImg->setVisibility(View::VISIBLE);
                    mRightInfoReduceImg->setVisibility(View::VISIBLE);
                    mRightInfoAddImg->setVisibility(View::VISIBLE);
                }

                mArcRight->setVisibility(View::VISIBLE);
                mArcPowder->setVisibility(View::VISIBLE);
                mPowderProgress->setVisibility(View::GONE);
                mPowderProgress->setIndeterminate(false);
                mRightProgress->setVisibility(View::GONE);
                mRightProgress->setIndeterminate(false);

                mPowderAnim->setRepeatCount(1);
                mPowderAnim->restart(0);
                mPowderAnim->stop();
                mRightAnim->setRepeatCount(1);
                mRightAnim->restart(0);
                mRightAnim->stop();

                mPowderInfoTv->setText(std::to_string(mArcPowder->getProgress()));
                mPowderInfoTitleTv->setText("粉重");
                mPowderInfoUnitsTv->setText("g");
                
                mInfoRVPicker->setSelectVisibility(View::VISIBLE);
            }
            mResetTv->setAlpha(isStart?0.4f:1.f);
            mResetTv->setEnabled(!isStart);
            
            int hotwaterTotal = g_objConf->getHotWaterTotal()+1;
            g_appData.eqStatus = isStart?ES_EXTRACTING:(g_appData.machineState & MC_STATE_HOT_WATER)?ES_STANDBY:ES_EXTRACT_DONE;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_MAKEING_START,PROP_BOOL, &isStart); // 启动/暂停
            if(isStart){
                int extType = MAKE_TYPE_COFFEE;
                int coffeeMode = CFM_HOT;
                g_tuyaOsMgr->reportDpData(TYCMD_MAKE_TYPE,PROP_ENUM, &extType); // 制作大类型
                g_tuyaOsMgr->reportDpData(TYCMD_MODE,PROP_ENUM, &coffeeMode); // 咖啡模式
                g_tuyaOsMgr->reportCoffeeDiyRawData(COFFEE_TYPE_NORMAL, CFM_HOT, 0, 0, 0, g_appData.hotWater,g_appData.hotWaterTemp); // 咖啡DIY
            }else{
                g_tuyaOsMgr->reportDpData(TYCMD_NUM_HOT,PROP_VALUE, &hotwaterTotal); // 出热水次数
            }
                
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
            if(isStart) g_appData.machineState |= MC_STATE_HOT_WATER;
            else{
                if(!(g_appData.machineState & MC_STATE_HOT_WATER))  
                    g_objConf->setHotWaterTotal(hotwaterTotal);
                g_appData.machineState &= ~MC_STATE_HOT_WATER;
            }
        
            g_objConnMgr->setExtraction(isStart,EXT_MODE_HOT_WATER,0x00,0x00,g_appData.hotWater,g_appData.hotWaterTemp,0x00,0x00);
            mHotWaterTv->getBackground()->setLevel(isStart?10000:0);
            break;
        }
    }
}

void PageHome::dealPowderInfoGroup(View &v){
    if(v.isSelected()){
        mArcPowder->setVisibility(View::GONE);
        mPowderProgress->setVisibility(View::VISIBLE);
        mPowderProgress->setIndeterminate(true);
        mPowderAnim->setRepeatCount(-1);
        if(!mPowderAnim->isRunning()){
            mPowderAnim->start();
        }

        g_appData.grindBean = 0;

        mPowderReduceImg->setVisibility(View::GONE);
        mPowderAddImg->setVisibility(View::GONE);
        
    }else{
        mArcPowder->setVisibility(View::VISIBLE);
        mPowderProgress->setVisibility(View::GONE);
        mPowderProgress->setIndeterminate(false);
        mPowderAnim->setRepeatCount(1);
        mPowderAnim->restart(0);
        mPowderAnim->stop();
        mPowderInfoTv->setText(std::to_string(mArcPowder->getProgress()));
        if(mModeBeanGrindMode==BEAN_GRIND_WEI){
            mPowderInfoTitleTv->setText("粉重");
            mPowderInfoUnitsTv->setText("g");
        }else{
            mPowderInfoTitleTv->setText("磨豆");
            mPowderInfoUnitsTv->setText("s");
        }

        if(mPageEditType == HOME_PAGE_NORMAL){
            mPowderReduceImg->setVisibility(View::VISIBLE);
            mPowderAddImg->setVisibility(View::VISIBLE);
        }
    }
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::icon_one_bean:
        case kaidu_ms7_lqy::R::id::icon_double_bean:{
            int grindTotal = g_objConf->getGrindTotal()+1;
            int  powder = g_appData.tuyaSetPowder<0?mArcPowder->getProgress():g_appData.tuyaSetPowder;
            g_objConnMgr->setGrindBean(v.isSelected(),powder,mModeBeanGrindMode);
            bool isStart = v.isSelected();
            g_appData.eqStatus = v.isSelected()?ES_GRINDING:(g_appData.grindBean >= powder)?ES_GRIND_DONE:ES_STANDBY;
#ifndef TUYA_OS_DISABLE
            g_tuyaOsMgr->reportDpData(TYCMD_BEAN_START,PROP_BOOL, &isStart); // 磨豆开关
            if(v.isSelected())  g_tuyaOsMgr->reportDpData(TYCMD_POWDER_SET,PROP_VALUE, &powder); // 粉重设置
            else                g_tuyaOsMgr->reportDpData(TYCMD_NUM_GRIND,PROP_VALUE, &grindTotal); // 研磨次数
            g_tuyaOsMgr->reportDpData(TYCMD_WORK_STATE,PROP_ENUM, &g_appData.eqStatus); // 工作状态
#endif
            if(v.isSelected()){
                g_appData.machineState |= MC_STATE_POWDER;
                mPowderInfoTitleTv->setText("磨豆中");
                if(mModeBeanGrindMode==BEAN_GRIND_WEI){
                    mPowderInfoUnitsTv->setText("g");
                    mPowderInfoTv->setText("0");
                }else{
                    mPowderInfoTv->setText(std::to_string(g_appData.tuyaSetPowder<0?mArcPowder->getProgress():g_appData.tuyaSetPowder));
                    mPowderInfoUnitsTv->setText("s");
                }
            }else{
                if(g_appData.grindBean >= powder) g_objConf->setGrindTotal(grindTotal);
                g_appData.machineState &= ~MC_STATE_POWDER;
                g_appData.tuyaSetPowder = -1;
            }
            break;
        }case kaidu_ms7_lqy::R::id::icon_one_cup:
        case kaidu_ms7_lqy::R::id::icon_double_cup:{
            if(v.isSelected()){
                mPowderInfoTv->setText("0");
                mPowderInfoTitleTv->setText("萃取中");
                mPowderInfoUnitsTv->setText("ml");
                mInfoRVPicker->setSelectVisibility(View::GONE);
                mInfoRVPicker->setValue(mInfoRVPicker->getMaxValue()/2);
            }else{
                mInfoRVPicker->setSelectVisibility(View::VISIBLE);
            }
            break;
        }
    }
    mHotWaterTv->setAlpha(v.isSelected()?0.4f:1.f);
    mHotWaterTv->setEnabled(!v.isSelected());
}
void PageHome::dealRightInfoGroup(View &v){
    if(v.isSelected()){
        mRightInfoReduceImg->setVisibility(View::GONE);
        mRightInfoAddImg->setVisibility(View::GONE);
        mArcRight->setVisibility(View::GONE);
        g_appData.extractWater = 0;
        g_appData.extractPressure = 0;
        mVisualProgress = 0;

        if(mGroupType != HOME_MT_EXTRACTING_TEA){
            mRightInfoTitleTv->setText("萃取中");
            mRightInfoWorkTimeTv->setVisibility(View::VISIBLE);
            mRightInfoUnitsTv->setText("bar");
            mRightInfoWorkTimeTv->setText("0s");
            mWorkingTime = 0;
            mWorkStartTime = SystemClock::uptimeMillis();
            mWindPageBox->postDelayed(mWorkingRun,1000);

            if(mRightAnim->isRunning()){
                mRightAnim->restart(0);
                mRightAnim->stop();
            }

            mRightWorkingAnimView->setVisibility(View::VISIBLE);
            mRightWorkingAnimView->getBackground()->setLevel(0);
            mRightAnimView->setVisibility(View::INVISIBLE);

            updatePressureGauge();
            mRightInfoTv->setText("0.0");
            mRightInfoTv->getLayoutParams()->width = 120;
            mRightInfoTv->requestLayout();
        }else{
            mRightInfoTv->setText("0");
            mRightAnim->setRepeatCount(-1);
            if(!mRightAnim->isRunning()){
                mRightAnim->start();
            }
        }
    }else{
        if(mPageEditType == HOME_PAGE_NORMAL){
            mRightInfoReduceImg->setVisibility(View::VISIBLE);
            mRightInfoAddImg->setVisibility(View::VISIBLE);
        }
        mArcRight->setVisibility(View::VISIBLE);

        if(mGroupType != HOME_MT_EXTRACTING_TEA){
            mRightInfoWorkTimeTv->setVisibility(View::GONE);
            mWindPageBox->removeCallbacks(mWorkingRun);
            mRightWorkingAnimView->setVisibility(View::GONE);
            mRightAnimView->setVisibility(View::VISIBLE);
            if(mLastProgressAnimator) mLastProgressAnimator->end();
        }else{
            mRightAnim->setRepeatCount(1);
            mRightAnim->restart(0);
            mRightAnim->stop();
        }
        updateRightInfoGroup(mInfoRVPicker->getValue());
        if((g_appData.warnState & A02)){ 
            std::time_t nowTime = std::time(NULL);
            if(nowTime >= std::mktime(&g_appData.nextPopWarnClean)){
                g_appData.warnIsDealState &= ~A02;
                g_windMgr->showPopPage(POP_WARN,mPageLayout);
            }
        }
        if(mRightInfoTv->getLayoutParams()->width != 90){
            mRightInfoTv->getLayoutParams()->width = 90;
            mRightInfoTv->requestLayout();
        }
    }
}

// arcseekbar 的值变化事件 回调
void PageHome::onArcValueChangeListener(View &v, int progress, bool fromUser){
    LOGI("id  = %d   progress = %d fromUser = %d",v.getId(),progress,fromUser);
    
    switch(v.getId()){
        case kaidu_ms7_lqy::R::id::arc_powder:{
            if(!mPowderAnim->isRunning() && fromUser) mPowderAnim->start();
            break;
        }
        case kaidu_ms7_lqy::R::id::arc_right:{
            if(!mRightAnim->isRunning() && fromUser) mRightAnim->start();
            break;
        }default:{
            return;
        }
    }
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{
            mEspMgr->onArcValueChangeListener(v,progress,fromUser);
            break;
        }case HOME_MT_AMERICANO:{
            mAmericanoMgr->onArcValueChangeListener(v,progress,fromUser);
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            mMasEspMgr->onArcValueChangeListener(v,progress,fromUser);
            break;
        }case HOME_MT_HAND_WASHED:{
            mHandWashMgr->onArcValueChangeListener(v,progress,fromUser);
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            mTeaMgr->onArcValueChangeListener(v,progress,fromUser);
            break;
        }case HOME_MT_FAVORITES:{
            LOGE("favorites is not onArcValueChangeListener !!!!!!");
            break;
        }default:{
            LOGE("mGroupType is error !!!!!!");
            break;
        }
    }
}

// 右边圆环的数据更新（可选择萃取水量、萃取温度等等，根据页面类型、二级picker选择来更新）
void PageHome::updateRightInfoGroup(int pickPos){
    switch(mGroupType){
        case HOME_MT_ESPRESSO:{
            mEspMgr->updateRightInfoGroup(pickPos);
            break;
        }case HOME_MT_AMERICANO:{
            mAmericanoMgr->updateRightInfoGroup(pickPos);
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            mMasEspMgr->updateRightInfoGroup(pickPos);
            break;
        }case HOME_MT_HAND_WASHED:{
            mHandWashMgr->updateRightInfoGroup();
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            mTeaMgr->updateRightInfoGroup();
            break;
        }case HOME_MT_FAVORITES:{
            LOGE("favorites is not updateRightInfoGroup !!!!!!");
            break;
        }default:{
            LOGE("mGroupType is error !!!!!!");
            break;
        }
    }
}


void PageHome::updatePressureGauge(){
    // g_appData.extractPressure
    // 15度 一格
    ObjectAnimator* animator = ObjectAnimator::ofInt(this,"pressure_visual_progress",{g_appData.extractPressure*10});
    animator->setAutoCancel(true);
    animator->setDuration(2500);
    animator->setInterpolator(DecelerateInterpolator::gDecelerateInterpolator.get()); // 设置动画的插值器
    AnimatorListenerAdapter animtorListener;
    animtorListener.onAnimationEnd=[this](Animator&anim,bool){
        delete mLastProgressAnimator;
        mLastProgressAnimator = nullptr;
    };
    animator->addListener(animtorListener);
    animator->start();
    mLastProgressAnimator = animator;
}

void PageHome::dealWarnStatus(){
    int warnState = (g_appData.warnState & ~g_appData.warnIsDealState);
    if(!(warnState & NEED_STOP_WORK_ERROR)){
        if((warnState & E07) || (warnState & E09) || (warnState & A01) || (warnState & A03)){
            mIsShowBeanWarn = true;
        }else{
            mIsShowBeanWarn = false;
        }
        return;
    }
    mIsShowBeanWarn = false;
    if(g_appData.machineState & MC_STATE_POWDER){
        InputEventSource::getInstance().closeScreenSaver();
        if(mOneBeanImg->isSelected())   mOneBeanImg->performClick();
        else                            mDoubleBeanImg->performClick();
    }else if(g_appData.machineState & MC_STATE_EXT){
        InputEventSource::getInstance().closeScreenSaver();
        if(mOneCupImg->isSelected())            mOneCupImg->performClick();
        else if(mTeaRebrewImg->isSelected())    mTeaRebrewImg->performClick();
        else                                    mDoubleCupImg->performClick();
    }else if(g_appData.machineState & MC_STATE_HOT_WATER){
        InputEventSource::getInstance().closeScreenSaver();
        mHotWaterTv->performClick();
    }

    // 出蒸汽中
    if(g_appData.machineState & MC_STATE_STEAM){
        InputEventSource::getInstance().closeScreenSaver();
        mSteamTv->performClick();
    }
}

void PageHome::dealTuyaExt(){
    InputEventSource::getInstance().closeScreenSaver();
    mPageEditType = HOME_PAGE_NORMAL;
    if(g_appData.tuyaStartExt){
        int OldTabSelectMode = g_appData.homeTabSelectMode;
        if(g_appData.tuyaMakeType == MAKE_TYPE_COFFEE){
            // 制作大类是咖啡
            int Type = g_appData.tuyaDiyData[0];
            int Mode = g_appData.tuyaDiyData[1];

            switch(Type){
                case COFFEE_TYPE_NORMAL:{
                    // 常规萃取
                    if(Mode == CFM_ESP){
                        // 测试咖啡
                        g_appData.homeTabSelectMode = 0;
                        mEspMgr->dealTuyaStartWork();
                        setGroupType(HOME_MT_ESPRESSO);
                    }else if(Mode == CFM_AMERICANO){
                        // 测试1咖啡
                        g_appData.homeTabSelectMode = 1;
                        mAmericanoMgr->dealTuyaStartWork();
                        setGroupType(HOME_MT_AMERICANO);
                    }else if(Mode == CFM_HOT){
                        // 热水
                        g_appData.hotWater = g_appData.tuyaDiyData[5];
                        g_appData.hotWaterTemp = g_appData.tuyaDiyData[6];
                        LOGE(" g_appData.hotWater = %d  g_appData.hotWaterTemp = %d", g_appData.hotWater,g_appData.hotWaterTemp);
                        if((mGroupType != HOME_MT_ESPRESSO) && (mGroupType != HOME_MT_AMERICANO)){
                            g_appData.homeTabSelectMode = 0;
                            setGroupType(HOME_MT_ESPRESSO);
                        }
                        g_objConf->setHotWater(g_appData.hotWater);
                        g_objConf->setHotWaterTemp(g_appData.hotWaterTemp);
                        mHotWaterTv->performClick();
                        memset(g_appData.tuyaDiyData,0,20);
                        g_appData.tuyaMakeType = -1;
                        return;
                        // setGroupType(HOME_MT_ESPRESSO);
                    }
                    break;
                }case COFFEE_TYPE_MAS:{
                    // 测试浓缩
                    g_appData.homeTabSelectMode = 2;
                    mMasEspMgr->dealTuyaStartWork();
                    setGroupType(HOME_MT_MASTER_ESPRESSO);
                    break;
                }case COFFEE_TYPE_HANDWASH:{
                    // 测试2咖啡
                    g_appData.homeTabSelectMode = 3;
                    mHandWashMgr->dealTuyaStartWork();
                    setGroupType(HOME_MT_HAND_WASHED);
                    break;
                }
            }
            
        }else{
            // 制作大类是测茶
            g_appData.homeTabSelectMode = 4;
            mTeaMgr->dealTuyaStartWork();
            if(mGroupType != HOME_MT_EXTRACTING_TEA)setGroupType(HOME_MT_EXTRACTING_TEA);
            mTeaMgr->updateCoffeeGroupData();
            // 需要再判断是不是再萃
            // if(){
            //     mTeaRebrewImg->setSelected(true);
            // }
        }
        if(OldTabSelectMode != g_appData.homeTabSelectMode){
            mHorScrollLayout->getChildAt(OldTabSelectMode)->setActivated(false);
            // mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(true);
            int selectPos = g_appData.homeTabSelectMode;
            int duration = std::abs(mHomeTabAnimBg->getTranslationX() - 255*selectPos);
            mHomeTabAnimBgListener.onAnimationEnd = [this,selectPos](Animator& animation, bool isReverse){
                if(selectPos == g_appData.homeTabSelectMode) mHorScrollLayout->getChildAt(g_appData.homeTabSelectMode)->setActivated(true);
            };
            mHomeTabAnimBg->animate().cancel();
            mHomeTabAnimBg->animate().translationX(255*selectPos).setDuration(duration).setListener(mHomeTabAnimBgListener).start();
        }
        if(mGroupType == HOME_MT_EXTRACTING_TEA){
            if(g_appData.tuyaDiyData[0] == MAKE_TEA){
                mTeaRebrewImg->setSelected(true);
            }
            mTeaMgr->startWork();
        }else if(mOneCupImg->isActivated())      mOneCupImg->performClick();
        else if(mDoubleCupImg->isActivated())   mDoubleCupImg->performClick();
        else                                    LOGE("*************************** 状态错误 !!!!!!!!!!!!!!!!!!!!!");
        memset(g_appData.tuyaDiyData,0,20);
        g_appData.tuyaMakeType = -1;
        g_appData.tuyaStartExt = false;
    }else{
        if(g_appData.machineState == MC_STATE_NONE){
            LOGE("!!!!!!!!!!!  app 下发错误 ");
            return;
        }
        if(g_appData.machineState & MC_STATE_HOT_WATER){
            if(mHotWaterTv->getBackground()->getLevel() == 10000)
                mHotWaterTv->performClick();
        }else if(mGroupType == HOME_MT_EXTRACTING_TEA){
            mTeaMgr->stopWork();
        }else{
            if(mOneCupImg->isSelected())            mOneCupImg->performClick();
            else if(mDoubleCupImg->isSelected())    mDoubleCupImg->performClick();
            else                                    LOGE("*************************** 状态错误 !!!!!!!!!!!!!!!!!!!");
        }
    }
}
