#ifndef _HOME_TAB_MODE_ESP_H_
#define _HOME_TAB_MODE_ESP_H_

#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <cyplotview.h>

#include "arc_seekbar.h"
#include "rvNumberPicker.h"

class HomeTabModeEsp {
public:
    HomeTabModeEsp(ViewGroup *wind_page_box);
    ~HomeTabModeEsp();

    void updateGroupData();                     // 页面  更新数据
    void updateRightInfoGroup(int pickPos);     // 右边数据（出水量、萃取水量等等） 更新
    void onItemClickListener(View &v);
    void onArcValueChangeListener(View &v, int progress, bool fromUser);
    void updatePageData();
    void updateBeanCupImgStatus();
    void onFavModeEditListener(int Position, bool isEdit);
    void dealTuyaStartWork();
    Json::Value getCacheDataToJson();
private:
    void initGroup();
    void dealRightInfoAddReductClick(bool isAdd);
    void dealPowderItemClick(View &v);
private:
    ViewGroup       *mWindPageBox;

    ArcSeekBar      *mArcPowder;        // 粉重调节 的圆形进度条
    ArcSeekBar      *mArcRight;         // 右边     的圆形进度条

    TextView        *mPowderInfoTv;     // 左边圆环中间数据 Tv （粉重）
    TextView        *mRightInfoTv;      // 右边圆环中间数据 Tv （各种数据）
    TextView        *mRightInfoTitleTv; // 右边圆环中间数据的标题 Tv （各种数据）
    TextView        *mRightInfoUnitsTv; // 右边圆环中间数据的单位 Tv （各种数据）

    RVNumberPicker  *mInfoRVPicker;     // 右边调节参数的 RV picker

    ImageView       *mOneBeanImg;       // 单豆按钮
    ImageView       *mDoubleBeanImg;    // 双豆按钮
    ImageView       *mOneCupImg;        // 单杯按钮
    ImageView       *mDoubleCupImg;     // 双杯按钮

////////////////// 各种模式的数据 ////////////////////
    EspDataStr       mEspData;

    EspSndDataStr   mCacheData;  // 缓存数据
////////////////// 美式、意式咖啡 //////////////////////
    ImageView       *mCupInfoFavImg;       // 收藏按钮
    ImageView       *mInfoImg;          // 模式的咖啡图

    TextView        *mResetTv;
    TextView        *mHotWaterTv;
    TextView        *mSteamTv;

    int mBeanItemState;
    int mCupItemState;
private:
    std::vector<RightInfoStr> espRightInfoList;

    int  mPageEditType;
    int  mFavEditPos;
};

#endif
