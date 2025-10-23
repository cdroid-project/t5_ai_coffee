/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:31:11
 * @FilePath: /t5_ai_demo/src/windows/page_home.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef _PAGE_HOME_H_
#define _PAGE_HOME_H_

#include "page_base.h"
#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <widget/horizontalscrollview.h>
#include <widget/progressbar.h>

#include "data.h"
#include "arc_seekbar.h"
#include "rvNumberPicker.h"

#include "home_tab_mode_esp.h"
#include "home_tab_mode_americano.h"
#include "home_tab_mode_mas_esp.h"
#include "home_tab_mode_handwash.h"
#include "home_tab_mode_tea.h"
#include "home_tab_mode_favorites.h"

class PageHome :public PageBase {
private:
    friend class PRESSURE_VISUAL_PROGRESS;
public:
    PageHome(ViewGroup *wind_page_box);
    ~PageHome();

public:
    void initPageView()override;
    void initPageData();
    void updatePageData()override;
    void stopPageAction()override;
    void resetPageAction()override;

    void onFavModeEditListener(int Position, bool isEdit);
    void stopSteamWork();
public:

private:
    void updateRightInfoGroup(int pickPos);     // 右边数据（出水量、萃取水量等等） 更新

    void onChildDealClickListener(View &v);
    void onArcValueChangeListener(View &v, int progress, bool fromUser);

    void setGroupType(int type);
    void updateGroupData();

    void dealPowderInfoGroup(View &v); // 处理粉重圆环的控件显示
    void dealRightInfoGroup(View &v);  // 处理右边圆环的控件显示
    void updatePressureGauge();         // 更新压力表数据
    void dealWarnStatus();
    void dealTuyaExt();

    void dealAiExt();
private:
    HomeTabModeEsp          *mEspMgr;
    HomeTabModeAmericano    *mAmericanoMgr;
    HomeTabModeMasEsp       *mMasEspMgr;
    HomeTabModeHandWash     *mHandWashMgr;
    HomeTabModeTea          *mTeaMgr;
    HomeTabModeFav          *mFavMgr;

    ViewGroup       *mCoffeeGroup;      // 咖啡的容器
    ViewGroup       *mTeaListGroup;     // 萃茶列表的容器
    ViewGroup       *mFavListGroup;     // 收藏列表的容器

    ArcSeekBar      *mArcPowder;        // 粉重调节 的圆形进度条
    ArcSeekBar      *mArcRight;         // 右边     的圆形进度条

    TextView        *mPowderInfoTitleTv;// 左边圆环中间数据的标题（粉重、建议克重）
    TextView        *mPowderInfoTv;     // 左边圆环中间数据 Tv （粉重、建议克重）
    TextView        *mPowderInfoUnitsTv;// 左边圆环中间数据的单位

    TextView        *mRightInfoTv;      // 右边圆环中间数据 Tv （各种数据）
    TextView        *mRightInfoTitleTv; // 右边圆环中间数据的标题 Tv （各种数据）
    TextView        *mRightInfoUnitsTv; // 右边圆环中间数据的单位 Tv （各种数据）
    TextView        *mRightInfoWorkTimeTv;   // 右边圆环中间数据的“Bar” Tv

    RVNumberPicker  *mInfoRVPicker;     // 右边调节参数的 RV picker

    ImageView       *mOneBeanImg;       // 单豆按钮
    ImageView       *mDoubleBeanImg;    // 双豆按钮
    ImageView       *mOneCupImg;        // 单杯按钮
    ImageView       *mDoubleCupImg;     // 双杯按钮
    ImageView       *mFavBackImg;       // 收藏功能页面的返回按钮
    ImageView       *mFavEnterImg;      // 收藏功能页面的确认按钮
    ImageView       *mTeaRebrewImg;     // 萃茶的再萃按钮
    TextView        *mFavNameTv;        // 收藏功能页面的名字

    ImageView       *mPowderReduceImg;  // 粉重 减少按钮
    ImageView       *mPowderAddImg;     // 粉重 增加按钮
    ImageView       *mRightInfoReduceImg;  // 右边信息 减少按钮
    ImageView       *mRightInfoAddImg;     // 右边信息 增加按钮

    ViewGroup       *mHomeFavGroup; // 主页的收藏 group (手冲模式、萃茶)
    ViewGroup       *mHomeTipsGroup;// 主页的Tips group (手冲模式、萃茶)
    ImageView       *mHomeFavImg;   // 主页的收藏 Img    (手冲模式、萃茶)
    ImageView       *mHomeTipsImg;  // 主页的Tips Img   (手冲模式、萃茶)

    ProgressBar     *mPowderProgress;
    ProgressBar     *mRightProgress;

    View            *mPowderAnimView;
    View            *mRightAnimView;

    View            *mRightWorkingAnimView;

    AnimatedImageDrawable *mPowderAnim;
    AnimatedImageDrawable *mRightAnim;
    AnimatedImageDrawable *mRightWorkingAnim;

    Animator::AnimatorListener mPowderWorkingListener;
    Animator::AnimatorListener mRightWorkingListener;

    ValueAnimator   *mResetAnimator;
    ValueAnimator   *mDislaySteamAnimator;
    ValueAnimator   *mDislayHotWaterAnimator;
    ObjectAnimator   *mLastProgressAnimator;
////////////////// 三种模式 的group ////////////////////
    ViewGroup       *mPicInfoGroup;   // 中间显示group：图片   (意式咖啡、美式咖啡)
    ViewGroup       *mPlotInfoGroup;  // 中间显示group：折线图 (大师浓缩)
    ViewGroup       *mFormInfoGroup;  // 中间显示group：表格   (手冲咖啡)

    View            *mInfoTouchView;  // 中间显示group,用于拦截touch的view 
    

////////////////// 美式、意式咖啡 //////////////////////
    ImageView       *mCupInfoFavImg;       // 美式、意式 收藏按钮

    ViewGroup       *mCupResetGroup;
    ImageView       *mInfoImg;          // 模式的咖啡图
    ImageView       *mCupShadowImg;
    TextView        *mResetTv;
    TextView        *mHotWaterTv;
    TextView        *mSteamTv;

////////////////// 大师浓缩 ///////////////////////////
    ImageView       *mPlotInfoFavImg;       // 大师浓缩 收藏按钮
    
////////////////// 手冲咖啡、萃茶 ///////////////////////////
    ViewGroup       *mFormGroup;
    ViewGroup       *mFormStepGroup_0;
    ViewGroup       *mFormStepGroup_1;
    ViewGroup       *mFormStepGroup_2;
    ViewGroup       *mFormStepGroup_3;
    ImageView       *mFormStepAddImg;

    NumberPicker    *mFormInfoHorPicker;
////////////////// 萃茶 ///////////////////////////
private:
    LinearLayout    *mHorScrollLayout;
    View            *mHomeTabAnimBg;
    Animator::AnimatorListener mHomeTabAnimBgListener;
private:
    int     mTabSelectMode;
    int     mGroupType;
    int     mPageEditType;

    int     mHandWashHorPickerState;
    int     mMasEspHorPickerState;

    int     mWorkingTime;
    int64_t mWorkStartTime;
    int     mVisualProgress;

    int      mFormHorPickerState;
    int     mModeBeanGrindMode;
    bool    mIsShowBeanWarn;
    int64_t  mLastUpdateHorPickerTime;

    Runnable    mWorkingRun;
    Runnable    mHorPickerUpdateRun;

    std::vector<HomePageTabDataStr>      mModeData;
};

#endif
