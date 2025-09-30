#include "page_factory.h"
#include <ghc/filesystem.hpp>
#include "R.h"
#include "conn_mgr.h"
#include "wind_mgr.h"
#include "wifi_adapter.h"
#include "conf_mgr.h"
#include <sys/ioctl.h>
#include <core/build.h>
#include "btn_mgr.h"

#define USBPATH "/vendor/udisk_sda1"

#define SCREEN_WIDTH    1920    // 屏幕的宽，用于添加触摸点
#define SCREEN_HEIGHT   440     // 屏幕的高，用于添加触摸点
#define FACTORY_PADDING 20      // 产测布局的padding，用于添加触摸点
#define TOUCH_SIZE      30      // 触摸点的大小

PageFactory::PageFactory(ViewGroup *wind_page_box)
    :PageBase(wind_page_box,"@layout/page_factory",PAGE_FACTORY){
 
    g_appData.mIsFactory = true;
    mSerialDown = false;
    mShutDownFlag = false;
    initPageView();
    initPageData();

}

PageFactory::~PageFactory() {
    g_appData.mIsFactory = true; // 退出产测模式
}

void PageFactory::initPageView(){
    mMainFlipper = (ViewFlipper*)mPageLayout;
    mMenuLayout     = (ViewGroup*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::menu);
    mTouchLayout    = (ViewGroup*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::touch);
    mColorFlipper   = (ViewFlipper*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::color);
    mWifiLayout     = (ViewGroup*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::wifi);
    mInfoLayout     = (ViewGroup*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::info);
    mShutDownLayout = (ViewGroup*)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::shutdown_btn);

    mTouchView      = (TouchView*)mTouchLayout->findViewById(kaidu_ms7_lqy::R::id::touchview);
    mwifiSsidText      = (TextView *)mWifiLayout->findViewById(kaidu_ms7_lqy::R::id::ssid_text);
    mwifiQualityText   = (TextView *)mWifiLayout->findViewById(kaidu_ms7_lqy::R::id::signal_level_text);

    mInfoModelView  = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_model);
    mInfoUuidView   = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_uuid);
    mInfoTpView     = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_tp);
    mInfoMCUView    = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_mcu);
    mInfoCdroidView = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_cdroid);
    mInfoMacView    = (TextView *)mInfoLayout->findViewById(kaidu_ms7_lqy::R::id::info_mac);
    mShutDownText   = (TextView *)mShutDownLayout->findViewById(kaidu_ms7_lqy::R::id::shutdown_text);

    mShutDownEnterText = (TextView *)mShutDownLayout->findViewById(kaidu_ms7_lqy::R::id::enter_btn);
    mShutDownCancelText = (TextView *)mShutDownLayout->findViewById(kaidu_ms7_lqy::R::id::cancel_btn);

    mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::exit)->setOnClickListener( [this](View&) {g_windMgr->showPrevPage();} );

    auto modeClick  = std::bind(&PageFactory::switchMode, this, std::placeholders::_1);
    auto btnclick   = std::bind(&PageFactory::btnClick, this, std::placeholders::_1);

    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_serial)->setOnClickListener( modeClick);
    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_touch)->setOnClickListener( modeClick);
    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_color)->setOnClickListener( modeClick);
    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_wifi)->setOnClickListener( modeClick);
    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_info)->setOnClickListener( modeClick);
    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_shutdown)->setOnClickListener( modeClick);
    
    mWifiLayout->findViewById(kaidu_ms7_lqy::R::id::shuaxin_wifi_btn)->setOnClickListener( btnclick );
    mWifiLayout->findViewById(kaidu_ms7_lqy::R::id::pass_btn)->setOnClickListener( btnclick);
    mShutDownEnterText->setOnClickListener( btnclick );
    mShutDownCancelText->setOnClickListener( btnclick);
    mInfoLayout->setOnClickListener( btnclick);
    
    
    mTouchView->setOnTestDownCallback(std::bind(&PageFactory::touchTestDown,this));

    mSerialRunner = [this](){
        if(mSerialDown){
            mNowShow = FACTORY_MENU;
            mMainFlipper->setDisplayedChild(mNowShow);
        }else{
            ((TextView *)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::serial_text))->setText(std::string("测试串口通讯异常，请检查通讯线。"));
            mSerialDown = true;
            mMainFlipper->post([this](){ g_objConnMgr->setFactoryCallback(nullptr);});
            mMainFlipper->removeCallbacks(mSerialRunner);
            mMainFlipper->postDelayed(mSerialRunner,3*1000);
            mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_serial)->setActivated(false);
        }
        
    };

    mShutDownRunner = [this](){
        if(mShutDownFlag){
            if(g_appData.isShutDown){
                g_objBtnMgr->shutDown();
            }else{
                mNowShow = FACTORY_MENU;
                mMainFlipper->setDisplayedChild(mNowShow);
            }
        }else {
            mShutDownText->setText(std::string("按键测试异常，请检查通讯线。"));
            mShutDownFlag = true;
            mMainFlipper->removeCallbacks(mShutDownRunner);
            mMainFlipper->postDelayed(mShutDownRunner,3*1000);
        }
        g_objConnMgr->setFactoryCallback(nullptr);
    };

    mInfoModelView->setText("型号：cdroid");
    mInfoUuidView->setText("UUID："+g_objConf->getTuyaAuthUUID());
#ifndef CDROID_X64
    int fd = open("/dev/techwin_ioctl", O_RDONLY);
    mInfoTpView->setText("TP版本号："+std::to_string(ioctl(fd,0xff)));
#else   
    mInfoTpView->setText("TP版本号：123123123");
#endif
    mInfoMCUView->setText("MCU版本：" + getMCUVersion(g_appData.McuVersion));
    mInfoCdroidView->setText("框架版本号：V"+BUILD::VERSION::Release);
    mInfoMacView->setText("MAC地址："+g_objConf->getDeviceMac());
}

void PageFactory::initPageData(){
    colorTest();
}

void PageFactory::updatePageData(){
    if(g_appData.statusChangeFlag & CS_WIFI_ADAPTER_NOTIFI){
        shuaxinWifiList();
    }
}

void PageFactory::stopPageAction(){
    mMainFlipper->removeCallbacks(mSerialRunner);
    mMainFlipper->removeCallbacks(mShutDownRunner);
}

void PageFactory::switchMode(View& v) {
    mNowShow = FACTORY_MENU;
    switch (v.getId()) {
        case kaidu_ms7_lqy::R::id::to_serial:{
            mNowShow = FACTORY_SERIAL;
            ((TextView *)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::serial_text))->setText(std::string("请将RX、TX接口对接或连接整机"));
            mMainFlipper->removeCallbacks(mSerialRunner);
            mMainFlipper->postDelayed(mSerialRunner,10*1000);
            mSerialDown = false;
            g_objConnMgr->setFactoryCallback([this](IAck *ack){
                bool isSerialSuccess = false;
                if((ack->getData(BUF_SND_D0) == 0x5A) && (ack->getData(BUF_SND_D1) == 0x04) &&
                        (ack->getData(BUF_SND_D2) == 0x09) && (ack->getData(BUF_SND_D3) == 0x01) &&
                        (ack->getData(BUF_SND_D4) == 0x01) && (ack->getData(BUF_SND_D5) == 0x01)){
                    isSerialSuccess = true;
                }else if((ack->getData(BUF_SND_D0) == 0x5B) && (ack->getData(BUF_SND_D1) == 0x02) &&
                        (ack->getData(BUF_SND_D2) == 0x09) && (ack->getData(BUF_SND_D3) == 0x5A) &&
                        (ack->getData(BUF_SND_D4) == 0xC0) && (ack->getData(BUF_SND_D5) == 0x2A)){
                    isSerialSuccess = true;
                }
                
                if(isSerialSuccess && (mNowShow == FACTORY_SERIAL)){
                    ((TextView *)mMainFlipper->findViewById(kaidu_ms7_lqy::R::id::serial_text))->setText(std::string("测试串口通讯正常"));
                    mSerialDown = true;
                    mMainFlipper->post([this](){ g_objConnMgr->setFactoryCallback(nullptr);});
                    mMainFlipper->removeCallbacks(mSerialRunner);
                    mMainFlipper->postDelayed(mSerialRunner,3*1000);
                    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_serial)->setActivated(true);
                }
                return true;
            });
            
            break;
        }   
        case kaidu_ms7_lqy::R::id::to_touch:{
            mNowShow = FACTORY_TOUCH;
            mTouchView->initTestPoint(); // 内部也有处理，也可省略
            break;
        }
        case kaidu_ms7_lqy::R::id::to_color:{
            mNowShow = FACTORY_COLOR;
            break;
        }
        case kaidu_ms7_lqy::R::id::to_wifi:{
            mNowShow = FACTORY_WIFI;
            WIFIMgr::ins()->scanWifi();
            break;
        }case kaidu_ms7_lqy::R::id::to_info:{
            mNowShow = FACTORY_INFO;
            break;
        }case kaidu_ms7_lqy::R::id::to_shutdown:{
            mNowShow = FACTORY_SHUTDOWN;
            mShutDownText->setText(std::string("该测试完成会自动重启，是否继续？"));
            mShutDownEnterText->setVisibility(View::VISIBLE);
            mShutDownCancelText->setVisibility(View::VISIBLE);
            break;
        } 
        default:
            break;
    }
    mMainFlipper->setDisplayedChild(mNowShow);
}

void PageFactory::touchTestDown() {
    mNowShow = FACTORY_MENU;
    mMainFlipper->setDisplayedChild(mNowShow);

    mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_touch)->setActivated(true);
}

void PageFactory::btnClick(View& v){
    switch (v.getId()) {
        case kaidu_ms7_lqy::R::id::shuaxin_wifi_btn:{
            WIFIMgr::ins()->scanWifi();
            break;
        }   
        case kaidu_ms7_lqy::R::id::pass_btn:{
            mNowShow = FACTORY_MENU;
            mMainFlipper->setDisplayedChild(mNowShow);
            mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_wifi)->setActivated(true);
            break;
        }case kaidu_ms7_lqy::R::id::info:{
            mNowShow = FACTORY_MENU;
            mMainFlipper->setDisplayedChild(mNowShow);
            break;
        }case kaidu_ms7_lqy::R::id::enter_btn:{
            mShutDownText->setText(std::string("1、请长按关机按键3s\n2、接收到关机信号后，将会自动重启"));

            mMainFlipper->removeCallbacks(mShutDownRunner);
            mMainFlipper->postDelayed(mShutDownRunner,15*1000);
            mShutDownFlag = false;
            // g_objConnMgr->setFactoryCallback([this](IAck *ack){
            //     bool isShutDownSuccess = false;
            //     if(mShutDownFlag) return true;
            //     else if((ack->getData(BUF_SND_D0) == 0x5A) && (ack->getData(BUF_SND_D1) == 0x02) &&
            //             (ack->getData(BUF_SND_D2) == 0x01) && (ack->getData(BUF_SND_D3) == 0x00) &&
            //             (ack->getData(BUF_SND_D4) == 0x5D) && (ack->getData(BUF_SND_D5) == 0x2A)){
            //         mShutDownFlag = true;
            //         mShutDownText->setText(std::string("接收按键通讯正常、下发关机指令到电源板正常\n正在自动重启"));
            //         mMainFlipper->removeCallbacks(mShutDownRunner);
            //         mMainFlipper->postDelayed(mShutDownRunner,3*1000);
            //         mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_shutdown)->setActivated(true);
            //     }else if((ack->getData(BUF_SND_D0) == 0x5B) && (ack->getData(BUF_SND_D1) == 0x02) &&
            //             (ack->getData(BUF_SND_D2) == 0x01) && (ack->getData(BUF_SND_D3) == 0x5A) &&
            //             (ack->getData(BUF_SND_D4) == 0xB8) && (ack->getData(BUF_SND_D5) == 0x2A)){
            //         mShutDownFlag = true;
            //         mShutDownText->setText(std::string("接收按键通讯正常、下发关机指令到电源板正常\n正在自动重启"));
            //         mMainFlipper->removeCallbacks(mShutDownRunner);
            //         mMainFlipper->postDelayed(mShutDownRunner,3*1000);
            //         mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_shutdown)->setActivated(true);
            //         return true;
            //     }
            //     return false;
            // });
            mShutDownEnterText->setVisibility(View::INVISIBLE);
            mShutDownCancelText->setVisibility(View::INVISIBLE);
            break;
        }case kaidu_ms7_lqy::R::id::cancel_btn:{
            mNowShow = FACTORY_MENU;
            mMainFlipper->setDisplayedChild(mNowShow);
            break;
        }
        default:
            break;
    }
}

void PageFactory::colorTest() {
    for (int i = 0, count = mColorFlipper->getChildCount(); i < count; i++) {
        if (i == count - 1)
            mColorFlipper->getChildAt(i)->setOnClickListener([this](View&) {

                mNowShow = FACTORY_MENU;
                mColorFlipper->setDisplayedChild(0);
                mMainFlipper->setDisplayedChild(mNowShow);

                mMenuLayout->findViewById(kaidu_ms7_lqy::R::id::to_color)->setActivated(true);
            });
        else
            mColorFlipper->getChildAt(i)->setOnClickListener([this](View&) {mColorFlipper->showNext();});
    }
}

void PageFactory::shuaxinWifiList(){
    std::sort(sWifiData.begin(),sWifiData.end(),[](const WIFIAdapterData &a, const WIFIAdapterData &b) -> bool { return a.signalLevel > b.signalLevel; });

    std::string ssidText,qualityText;
    for(int i=0; i<sWifiData.size();i++){
        ssidText += "ssid = " + sWifiData.at(i).name + "\n";
        qualityText += "signalLevel = " + std::to_string(sWifiData.at(i).signalLevel) + "   quality = " + std::to_string(sWifiData.at(i).quality) + "\n";
    }
    mwifiSsidText->setText(ssidText);
    mwifiQualityText->setText(qualityText);
}

/////////////////////////////// TouchView ///////////////////////////////
DECLARE_WIDGET(TouchView)
TouchView::TouchView(int w, int h):View(w,h){
    mGridSize = 10;
    mRectangleSize = 80;
    mClear = true;
    mIsInitTestPoint = false;
    mTestDownListener = nullptr;
}

TouchView::TouchView(Context *ctx, const AttributeSet &attr):View(ctx,attr){
    mGridSize = 10;
    mRectangleSize = 80;
    mClear = true;
    mIsInitTestPoint = false;
    mTestDownListener = nullptr;
}

TouchView::~TouchView(){
    
}

void TouchView::initTestPoint(){
    mTopTestPoint.clear();
    mBottomTestPoint.clear();
    mLeftTestPoint.clear();
    mRightTestPoint.clear();
    // 自适应宽度（取浮点型使得方块紧凑在一起）
    // 例如 offsetX 是 40.4，这样在递增的时候，会有从80.8 递增到 121.2,宽度递增了41
    float offsetX = (getWidth()-2*START_POINT)/(float)((getWidth()-2*START_POINT)/mRectangleSize); 
    int horCount = (getWidth()-2*START_POINT)/mRectangleSize;
    int i=0;
    for(float x=START_POINT; i<horCount; x+=offsetX,i++){
        LOGV("getWidth()-START_POINT = %d x = %f std::ceil(x) = %f offsetX = %f",getWidth()-START_POINT,x,std::ceil(x),offsetX);
        // 添加点的坐标数据
        // (int)((x+offsetX)-(int)x) 同上解释，自适应宽度
        mTopTestPoint.push_back({NOT_INIT,Rect::Make(x,START_POINT,(int)((x+offsetX)-(int)x),mRectangleSize)});
        mBottomTestPoint.push_back({NOT_INIT,Rect::Make(x,getHeight()-START_POINT-mRectangleSize,(int)((x+offsetX)-(int)x),mRectangleSize)});
    }

    float offsetY = (getHeight()-2*mRectangleSize-2*START_POINT)/(float)((getHeight()-2*mRectangleSize-2*START_POINT)/mRectangleSize); 
    int verCount = (getHeight()-2*mRectangleSize-2*START_POINT)/mRectangleSize;
    i=0;
    for(float y=mRectangleSize+START_POINT; i<verCount; y+=offsetY,i++){
        mLeftTestPoint.push_back({NOT_INIT,Rect::Make(START_POINT,y,mRectangleSize,(int)((y+offsetY)-(int)y))});
        mRightTestPoint.push_back({NOT_INIT,Rect::Make(getWidth()-START_POINT-mRectangleSize,y,mRectangleSize,(int)((y+offsetY)-(int)y))});
    }
    mIsInitTestPoint = true;
}

bool TouchView::checkTestPoint(){
    for(auto it:mTopTestPoint){ if(it.flag != TOUCH_ENTER){ return false; } }
    for(auto it:mBottomTestPoint){ if(it.flag != TOUCH_ENTER){ return false; } }
    for(auto it:mLeftTestPoint){ if(it.flag != TOUCH_ENTER){ return false; } }
    for(auto it:mRightTestPoint){ if(it.flag != TOUCH_ENTER){ return false; } }
    return true;
}

// 绘制背景网格
void TouchView::drawGrid(Canvas&canvas){
    for(int x=0,i=0;x<getWidth();x+=mGridSize,i++){
        canvas.move_to(x,0);
        canvas.line_to(x,getHeight());
        canvas.set_source_rgba(.5,.5,.5,(i%10==0)?1.f:0.4f);
        canvas.stroke();
    }
    for(int y=0,i=0;y<getHeight();y+=mGridSize,i++){
        canvas.move_to(0,y);
        canvas.line_to(getWidth(),y);
        canvas.set_source_rgba(.5,.5,.5,(i%10==0)?1.f:0.4f);
        canvas.stroke();
    }
}

// 绘制产测触摸区域
void TouchView::drawFactoryTestPoint(Canvas&canvas){
    // 左边的测试点
    for(auto it:mLeftTestPoint){
        if(it.flag == TOUCH_ENTER){
            // 触摸区域正确，则填充纯绿
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(0.f,1.f,0.f);
            canvas.fill();
        }else if(it.flag == TOUCH_OUTSIDE){
            // 触摸区域在区域外，则填充纯红
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(1.f,0.f,0.f);
            canvas.fill();
        }else{
            // 未触摸，则画个黄色边框
            canvas.rectangle(it.rect.left+LINE_WIDTH/2,it.rect.top+LINE_WIDTH/2,it.rect.width-LINE_WIDTH,it.rect.height-LINE_WIDTH);
            canvas.set_line_width(LINE_WIDTH);
            canvas.set_source_rgb(1.f,1.f,0.f);
            canvas.stroke();
        }
    }
    // 上边的测试点
    for(auto it:mTopTestPoint){
        if(it.flag == TOUCH_ENTER){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(0.f,1.f,0.f);
            canvas.fill();
        }else if(it.flag == TOUCH_OUTSIDE){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(1.f,0.f,0.f);
            canvas.fill();
        }else{
            canvas.rectangle(it.rect.left+LINE_WIDTH/2,it.rect.top+LINE_WIDTH/2,it.rect.width-LINE_WIDTH,it.rect.height-LINE_WIDTH);
            canvas.set_line_width(LINE_WIDTH);
            canvas.set_source_rgb(1.f,1.f,0.f);
            canvas.stroke();
        }
    }
    // 右边的测试点
    for(auto it:mRightTestPoint){
        if(it.flag == TOUCH_ENTER){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(0.f,1.f,0.f);
            canvas.fill();
        }else if(it.flag == TOUCH_OUTSIDE){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(1.f,0.f,0.f);
            canvas.fill();
        }else{
            canvas.rectangle(it.rect.left+LINE_WIDTH/2,it.rect.top+LINE_WIDTH/2,it.rect.width-LINE_WIDTH,it.rect.height-LINE_WIDTH);
            canvas.set_line_width(LINE_WIDTH);
            canvas.set_source_rgb(1.f,1.f,0.f);
            canvas.stroke();
        }
    }
    // 底边的测试点
    for(auto it:mBottomTestPoint){
        if(it.flag == TOUCH_ENTER){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(0.f,1.f,0.f);
            canvas.fill();
        }else if(it.flag == TOUCH_OUTSIDE){
            canvas.rectangle(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
            canvas.set_source_rgb(1.f,0.f,0.f);
            canvas.fill();
        }else{
            canvas.rectangle(it.rect.left+LINE_WIDTH/2,it.rect.top+LINE_WIDTH/2,it.rect.width-LINE_WIDTH,it.rect.height-LINE_WIDTH);
            canvas.set_line_width(LINE_WIDTH);
            canvas.set_source_rgb(1.f,1.f,0.f);
            canvas.stroke();
        }
    }
}

#define R 4  // 历史点的圆的半径
bool TouchView::onTouchEvent(MotionEvent&event){
    const int pointer = event.getActionIndex();
    int minX=INT_MAX,minY=INT_MAX;
    int maxX=INT_MIN,maxY=INT_MIN;
    switch(event.getActionMasked()){
    case MotionEvent::ACTION_UP:
        mClear = event.getActionMasked()==MotionEvent::ACTION_UP&&(event.getX()<20)&&(event.getY()<20);
        if(mClear){
            invalidate();
        }
        if(mTestDownListener && checkTestPoint()){
            mTestDownListener();
            initTestPoint();
            mTouchPoints.clear();
            mClear=true;
        }
        break;
    case MotionEvent::ACTION_DOWN:
    case MotionEvent::ACTION_POINTER_DOWN:
    case MotionEvent::ACTION_MOVE:{
        int x = event.getX();
        int y = event.getY();
        int pressure =event.getPressure(0);
        minX = std::min(x,minX);
        minY = std::min(y,minY);
        maxX = std::max(x,maxX);
        maxY = std::max(y,maxY);

        for(int i=80; i<mTouchPoints.size();i++){
            mTouchPoints.pop_front();
        }
        mTouchPoints.push_back({x,y,pressure});

        // 判断是否在触摸点区域内
        if((y<=mRectangleSize+START_POINT) && (y>=0)){
            for(auto &it:mTopTestPoint){
                if(it.rect.contains(x,START_POINT+mRectangleSize/2)){
                    invalidate(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
                    it.flag = y>=START_POINT?TOUCH_ENTER:TOUCH_OUTSIDE;
                    break;
                }
            }
        }else if((y>=getHeight() - mRectangleSize-START_POINT) && (y<=getHeight())){
            for(auto &it:mBottomTestPoint){
                if(it.rect.contains(x,getHeight()-START_POINT-mRectangleSize/2)){
                    invalidate(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
                    it.flag = y<=getHeight()-START_POINT?TOUCH_ENTER:TOUCH_OUTSIDE;
                    break;
                }
            }
        }else if((x<=mRectangleSize+START_POINT) && (x>=0)){
            for(auto &it:mLeftTestPoint){
                if(it.rect.contains(START_POINT+mRectangleSize/2,y)){
                    invalidate(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
                    it.flag = x>=START_POINT?TOUCH_ENTER:TOUCH_OUTSIDE;
                    break;
                }
            }
        }else if((x>=getWidth() - mRectangleSize-START_POINT) && (x<=getWidth())){
            for(auto &it:mRightTestPoint){
                if(it.rect.contains(getWidth()-START_POINT-mRectangleSize/2,y)){
                    invalidate(it.rect.left,it.rect.top,it.rect.width,it.rect.height);
                    it.flag = x<=getWidth()-START_POINT?TOUCH_ENTER:TOUCH_OUTSIDE;
                    break;
                }
            }
        }
        invalidate(minX-R,minY-R,maxX-minX+R+R,maxY-minY+R+R);
        break;
    }
    default:break;
    }
    return true;
}
void TouchView::onDraw(Canvas&canvas){
    if(mClear){
        canvas.set_source_rgb(0,0,0);
        canvas.paint();
        mTouchPoints.clear();
        mClear = false;
    }
    if(!mIsInitTestPoint){
        initTestPoint();
    }
    drawGrid(canvas);
    drawFactoryTestPoint(canvas);
    for(auto it=mTouchPoints.begin();it!=mTouchPoints.end();it++){
        TOUCHPOINT pts = *it;
        canvas.set_source_rgb(0.25,0.66,1);

        canvas.arc(pts.x,pts.y,R+pts.pressure,0,M_PI*2.f);
        canvas.fill();

    }
}

void TouchView::setOnTestDownCallback(OnTestDownListener l){
    mTestDownListener = l;
}


