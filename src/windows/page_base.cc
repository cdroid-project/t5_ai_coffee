#include "page_base.h"
#include "R.h"
#include "comm_func.h"
#include "wind_mgr.h"
#include "project.h"

PageBase::PageBase(ViewGroup *wind_page_box,const std::string resource,int page_type)
    :mWindPageBox(wind_page_box),mPageType(page_type){

    mPageLayout             = nullptr;
    mTitleBarWeather        = nullptr;
    mTitleBarWeekText       = nullptr;
    mTitleBarTimeText       = nullptr;
    mTitleBarWifi           = nullptr;
    mTitleWash              = nullptr;

    initPageBase(resource);
    changeTitleBarStatus(0xFF);

    mInitTime               = SystemClock::uptimeMillis();
}

PageBase::~PageBase() {
    
}

ViewGroup *PageBase::getPageLayout(){
    return mPageLayout;
}   

void PageBase::initPageBase(const std::string resource){
    mPageLayout = (ViewGroup *)LayoutInflater::from(mWindPageBox->getContext())->inflate(resource, nullptr);
    addPage(); //默认显示

    mToastView = (TextView *)mWindPageBox->getParent()->findViewById(t5_ai_coffee::R::id::toast_tv);
    if(mPageType == PAGE_SCREENSAVER){
        mTitleBarWeather    = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::weather_img);
        mTitleBarWeekText   = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::date_text);
        mTitleBarTimeText   = (TimeTextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::time_text);
        mTitleBarTimeText->start();
        
        mTickerTime = 1000; //每1秒刷新一次
        mTicker = std::bind(&PageBase::onBaseTicker,this);
        onBaseTicker();
    }else if((mPageType != PAGE_KEYBOARD) && (mPageType != PAGE_FACTORY)){
    
        mTitleBarWifi       = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::icon_wifi);
        mTitleWash          = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::icon_wash);
        mTitleSetup         = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::icon_setup);

        mTitleBarWeather    = (ImageView *)mPageLayout->findViewById(t5_ai_coffee::R::id::weather);
        mTitleBarWeekText   = (TextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::week_text);
        mTitleBarTimeText   = (TimeTextView *)mPageLayout->findViewById(t5_ai_coffee::R::id::time_text);
        mTitleBarTimeText->start();

        auto btn_click_func = std::bind(&PageBase::btnClickListener, this, std::placeholders::_1);

        if(mPageType != PAGE_SETTING)   mTitleBarWifi->setOnClickListener(btn_click_func);
        if(mPageType != PAGE_CLEAN)     mTitleWash->setOnClickListener(btn_click_func);

        if(mPageType != PAGE_SETTING)   mTitleSetup->setOnClickListener(btn_click_func);
        else                            mTitleSetup->setOnClickListener(std::bind(&PageBase::enterTestPage, this, std::placeholders::_1));

        mPageLayout->findViewById(t5_ai_coffee::R::id::time_layout)->setOnClickListener([this](View &){
            static int clickCount = 0;
            if(SystemClock::uptimeMillis() <= 15*1000){
                clickCount++;
                if(clickCount >= 5){
                    clickCount = 0;
                    // g_windMgr->showPage(PAGE_SCALE_CALIBRATION,false);
                }
            }
        });
        mPageLayout->findViewById(t5_ai_coffee::R::id::time_layout)->setSoundEffectsEnabled(false);

        mTickerTime = 1000; //每1秒刷新一次
        mTicker = std::bind(&PageBase::onBaseTicker,this);

        setWifiStatus();
        onBaseTicker();
    }

    // mToastView->setOnClickListener(std::bind(&PageBase::btnClickListener, this, std::placeholders::_1));
    mToastView->setOnClickListener([](View &){});
    mToastView->setSoundEffectsEnabled(false);

    mViewAnimListener.onAnimationEnd = [this](Animator& animation, bool isReverse){
        mToastView->setVisibility(View::GONE);
    };
    mViewAnimListener.onAnimationCancel = [this](Animator& animation){
        mToastView->setVisibility(View::GONE);
    };
}

void PageBase::removePage(){
    if((mPageType != PAGE_KEYBOARD) && (mPageType != PAGE_FACTORY)){
        mPageLayout->removeCallbacks(mTicker);
        mTitleBarTimeText->stop();
    }
    mWindPageBox->removeView(mPageLayout);
    mToastView->animate().cancel();
}

void PageBase::addPage(){
    mWindPageBox->addView(mPageLayout);
}

void PageBase::stopPageAction(){
    LOGV("mPageType = %d  is not override stopPageAction",mPageType);
}

void PageBase::resetPageAction(){
    LOGV("mPageType = %d  is not override resetPageAction",mPageType);
}

void PageBase::setPageType(int type){
    mPageType = type;
}

int  PageBase::getPageType(){
    return mPageType;
}

void PageBase::changeTitleBarStatus(uchar status){
    // 更新时间、// 更新日期
    if((status & TITLE_BAR_TIME) || (status & TITLE_BAR_DATE)){
        if(mTitleBarTimeText) mTitleBarTimeText->setText(getTimeStr());
        if(mTitleBarWeekText){
            if(mPageType == PAGE_SCREENSAVER)   mTitleBarWeekText->setText(getDateTime2());
            else                                mTitleBarWeekText->setText(getWeekDay());
        }
    }
    
    // 更新天气
    if(status & TITLE_BAR_WEATHER){
        if(mTitleBarWeather){
            if(mPageType == PAGE_SCREENSAVER)   mTitleBarWeather->setImageResource(g_appData.weatherConImage+"_larger");
            else                                mTitleBarWeather->setImageResource(g_appData.weatherConImage);
        }
    }

    // 更新wifi
    if((status & TITLE_BAR_WIFI) && mTitleBarWifi){
        setWifiStatus();
    }

    if((status & TITLE_BAR_CLEAN) && mTitleWash){
        mTitleWash->setActivated(g_appData.warnState & A02);
    }
}

void PageBase::onBaseTicker(){
    if(mPageType == PAGE_SCREENSAVER)   mTitleBarWeekText->setText(getDateTime2());
    else                                mTitleBarWeekText->setText(getWeekDay());
    mTitleBarTimeText->setText(getTimeStr());

    onTicker();
    mPageLayout->postDelayed(mTicker,mTickerTime);
}

void PageBase::onTicker(){
    LOGV("mPageType = %d  is no override onTicker",mPageType);
}

void PageBase::btnClickListener(View& view){
    if(g_appData.machineState){
        showToast("工作中不能切换页面");
        return;
    }
    switch(view.getId()){
        case t5_ai_coffee::R::id::icon_wifi:{
            g_windMgr->showPage(PAGE_SETTING);
            break;
        }case t5_ai_coffee::R::id::icon_wash:{
            g_windMgr->showPage(PAGE_CLEAN);
            break;
        }case t5_ai_coffee::R::id::icon_setup:{
            g_windMgr->showPage(PAGE_SETTING);
            break;
        }
    }
}

void PageBase::enterTestPage(View&v){
    static int64_t LastClick = SystemClock::uptimeMillis();
    // static int setupClickCount = 0;
    static int  clickCount    = 0;
    int64_t now_tick = SystemClock::uptimeMillis();
    switch(v.getId()){
        case t5_ai_coffee::R::id::icon_setup:{
            if((now_tick - LastClick ) <= 2000){
                if(++clickCount >= 10){
                    g_windMgr->showPage(PAGE_FACTORY);
                    // PageFactory();
                    clickCount = 1;
                }
            }else{
                clickCount = 1;
            }
            LastClick = now_tick;
            break;
        }
    }
    LOGE("clickCount = %d",clickCount);
}


void PageBase::showToast(std::string showText,int translationX){
    mToastView->setTranslationX(translationX);
    LOGE("translationX = %d showText = %s",translationX,showText.c_str());
    mToastView->setText(showText);
    mToastView->animate().cancel();
    mToastView->setVisibility(View::VISIBLE);
    mToastView->setAlpha(1.0f);
    mToastView->animate().alpha(0.01f).setDuration(3000).setListener(mViewAnimListener).start();
}

void PageBase::setWifiStatus(){
    if(g_appData.netOk){
        switch(g_appData.netStatus){
            case PRO_STATE_NET_NONE:{
                mTitleBarWifi->setImageLevel(5);
                break;
            }case PRO_STATE_NET_ONE:{
                mTitleBarWifi->setImageLevel(1);
                break;
            }case PRO_STATE_NET_TWO:{
                mTitleBarWifi->setImageLevel(2);
                break;
            }case PRO_STATE_NET_THREE:{
                mTitleBarWifi->setImageLevel(3);
                break;
            }case PRO_STATE_NET_FULL:{
                mTitleBarWifi->setImageLevel(4);
                break;
            }
        }
    }else{
        mTitleBarWifi->setImageLevel(5);
    }
}