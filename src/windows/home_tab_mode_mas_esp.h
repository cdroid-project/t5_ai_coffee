#ifndef _HOME_TAB_MODE_MAS_ESP_H_
#define _HOME_TAB_MODE_MAS_ESP_H_

#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <cyplotview.h>
#include <widget/numberpicker.h>

#include "arc_seekbar.h"
#include "rvNumberPicker.h"

class HomeTabModeMasEsp {
private:
    typedef struct {
        int extractWater;       // 萃取水量
        double extractPressure;    // 萃取压力
    } PlotHistoryDataStr;
public:
    HomeTabModeMasEsp(ViewGroup *wind_page_box);
    ~HomeTabModeMasEsp();

    void updateGroupData();                     // 页面  更新数据
    void updateRightInfoGroup(int pickPos);     // 右边数据（出水量、萃取水量等等） 更新
    void updatePageData();
    void updateBeanCupImgStatus();
    void updatePoltViewData();
    void onItemClickListener(View &v);
    void onArcValueChangeListener(View &v, int progress, bool fromUser);
    void onHorPickerChange();
    void onFavModeEditListener(int Position, bool isEdit);
    void dealTuyaStartWork();
    Json::Value getCacheDataToJson();
private:
    void initGroup();

    void setPlotViewData();     // 设置   测试浓缩 折线图 数据
    void initPlotViewData();    // 初始化 测试浓缩 折线图
    void dealRightInfoAddReductClick(bool isAdd);
    void dealPowderItemClick(View &v);
private:
    ViewGroup       *mWindPageBox;

////////////////// 共用控件 ////////////////////
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
    MasEspDataStr   mMasEspData;

    MasEspSndModeDataStr mCacheData;
////////////////// 测试浓缩特定控件 //////////////////////
    NumberPicker    *mPlotInfoModePicker;   // 测试浓缩 横向picker
    CYPlotView      *mPlotInfoPlotView;     // 测试浓缩 plot折线图
    CYPlotObject      *mPlotObj;              // 测试浓缩 plot 折线图 数据
    CYPlotObject      *mPlotRealObj;          // 测试浓缩 plot 折线图 实时数据

    ImageView       *mPlotInfoFavImg;       // 测试浓缩 收藏按钮
    ImageView       *mPlotInfoEditImg;      // 测试浓缩 的修改按钮图

#if 0
    Runnable        mHorPickerUpdateRun;
    int             mMasEspHorPickerState;
#endif
private:
    
    int             mPageEditType;
    int             mFavEditPos;

    std::vector<RightInfoStr> masEspRightInfoList;
    std::vector<PlotHistoryDataStr> masEspPlotHistoryDataList;
};

#endif
