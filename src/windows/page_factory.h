/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:12:04
 * @FilePath: /t5_ai_coffee/src/windows/page_factory.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef __PAGE_FACTORY__
#define __PAGE_FACTORY__

#include <widget/viewflipper.h>
#include <widget/textview.h>
#include "page_base.h"
#include <core/rect.h>

class TouchView;
class PageFactory :public PageBase {
private:
    enum {
        FACTORY_MENU = 0,    // 主菜单
        FACTORY_SERIAL,      // 串口
        FACTORY_TOUCH,       // 触摸
        FACTORY_COLOR,       // 颜色
        FACTORY_WIFI,        // wifi
        FACTORY_INFO,        // info 项目信息
        FACTORY_SHUTDOWN,    // 关机按键
    };
private:
    ViewFlipper* mMainFlipper;
    ViewFlipper* mColorFlipper;
    ViewGroup*   mMenuLayout;
    ViewGroup*   mTouchLayout;
    ViewGroup*   mWifiLayout;
    ViewGroup*   mInfoLayout;
    ViewGroup*   mShutDownLayout;

    TouchView*   mTouchView;

    TextView*    mwifiSsidText;
    TextView*    mwifiQualityText;

    TextView*    mInfoModelView;
    TextView*    mInfoUuidView;
    TextView*    mInfoMacView;
    TextView*    mInfoTpView;
    TextView*    mInfoMCUView;
    TextView*    mInfoCdroidView;

    TextView*    mShutDownText;
    TextView*    mShutDownEnterText;
    TextView*    mShutDownCancelText;

    int mNowShow = 0;
    bool     mSerialDown;
    bool     mShutDownFlag;
    Runnable mSerialRunner;
    Runnable mShutDownRunner;
public:
    PageFactory(ViewGroup *wind_page_box);
    ~PageFactory();

    void initPageView()override;
    void initPageData()override;
    void updatePageData()override;
    void stopPageAction()override;
private:
    void switchMode(View& v);
    void btnClick(View& v);

    void touchTestDown();
    void colorTest();
    void shuaxinWifiList();

};

// 触摸测试的view
class TouchView:public View{
    public:
        DECLARE_UIEVENT(void,OnTestDownListener);
    protected:
        #define LINE_WIDTH 2    // 线条宽度
        #define START_POINT 11  // 相当于padding
        enum{
            NOT_INIT = 0,
            TOUCH_ENTER,
            TOUCH_OUTSIDE,
        };
    private:
        typedef struct{int x;int y;int pressure;}TOUCHPOINT;
        typedef struct{int flag; Rect rect;}POINTINFO;
        std::list<TOUCHPOINT> mTouchPoints;
        std::vector<POINTINFO> mLeftTestPoint;  // 左边测试点
        std::vector<POINTINFO> mTopTestPoint;   // 上边测试点
        std::vector<POINTINFO> mRightTestPoint; // 右边测试点
        std::vector<POINTINFO> mBottomTestPoint;// 底边测试点
    
        OnTestDownListener mTestDownListener;   // 测试完成时，会调用的回调，用于退出测试
        int mGridSize;                          // 背景的网格间距
        int mRectangleSize;                     // 测试方块的尺寸（宽高一致，但只是大概值，计算后可能会上下偏差1）
        bool mClear;                            // 是否清除当前页面的绘制状态，回到初始状态
        bool mIsInitTestPoint;                  // 初始化测试点，需要在第一次ondraw 时调用 initTestPoint
    public:
        TouchView(int w, int h);
        TouchView(Context *ctx, const AttributeSet &attr);
        ~TouchView();
    
        bool onTouchEvent(MotionEvent&event)override;
        void onDraw(Canvas&canvas)override;
        
        void initTestPoint();
        void setOnTestDownCallback(OnTestDownListener l);
    protected:
        
        bool checkTestPoint();
        void drawFactoryTestPoint(Canvas&canvas);
    
        void drawGrid(Canvas&canvas);
    };

#endif