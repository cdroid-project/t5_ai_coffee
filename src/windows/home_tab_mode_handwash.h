#ifndef _HOME_TAB_MODE_HANDWASH_H_
#define _HOME_TAB_MODE_HANDWASH_H_

#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <widget/numberpicker.h>
#include "arc_seekbar.h"

class HomeTabModeHandWash {
public:
    HomeTabModeHandWash(ViewGroup *wind_page_box);
    ~HomeTabModeHandWash();

    void updateGroupData();                     // 页面  更新数据
    void updateRightInfoGroup();     // 右边数据（出水量、萃取水量等等） 更新
    void updatePageData();
    void updateBeanCupImgStatus();
    void onItemClickListener(View &v);
    void onArcValueChangeListener(View &v, int progress, bool fromUser);
    void onHorPickerChange();
    void onFavModeEditListener(int Position, bool isEdit);
    void dealTuyaStartWork();
    void dealAiData();
    Json::Value getCacheDataToJson();
private:
    void initGroup();
    void showHandWashPop(int selectStep);   // 显示 手冲咖啡 的修改弹窗页面
    void setFormData();                     // 设置   表格数据
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

    ImageView       *mOneBeanImg;       // 单豆按钮
    ImageView       *mDoubleBeanImg;    // 双豆按钮
    ImageView       *mOneCupImg;        // 单杯按钮
    ImageView       *mDoubleCupImg;     // 双杯按钮

    ViewGroup       *mHomeFavGroup; // 主页的收藏 group (手冲模式、萃茶)
    ViewGroup       *mHomeTipsGroup;// 主页的Tips group (手冲模式、萃茶)
    ViewGroup       *mHomeTipsHandWashGroup;  
    TextView        *mHomeTipsTeaTv;
    TextView        *mHomeTipsWater;// 主页的 group 累积注水量 (手冲模式、萃茶)
    TextView        *mHomeTipsTime; // 主页的 group 累积中断时间 (手冲模式、萃茶)
////////////////// 各种模式的数据 ////////////////////
    HandWashDataStr   mHandWashData;
 
    HandWashSndModeDataStr mCacheData;
////////////////// 手冲咖啡特定控件 //////////////////////
    NumberPicker    *mHorModePicker;   // 手冲咖啡 横向picker

    ViewGroup       *mFormGroup;
    ViewGroup       *mFormTitleGroup;
    ViewGroup       *mFormStepGroup_0;
    TextView        *mFormStepTitle_0;    
    TextView        *mFormStepWater_0;
    TextView        *mFormStepSpeed_0;
    TextView        *mFormStepTime_0;

    ViewGroup       *mFormStepGroup_1;
    TextView        *mFormStepTitle_1;    
    TextView        *mFormStepWater_1;
    TextView        *mFormStepSpeed_1;
    TextView        *mFormStepTime_1;

    ViewGroup       *mFormStepGroup_2;
    TextView        *mFormStepTitle_2;    
    TextView        *mFormStepWater_2;
    TextView        *mFormStepSpeed_2;
    TextView        *mFormStepTime_2;

    ViewGroup       *mFormStepGroup_3;
    TextView        *mFormStepTitle_3;    
    TextView        *mFormStepWater_3;
    TextView        *mFormStepSpeed_3;
    TextView        *mFormStepTime_3;

    ImageView       *mFormStepAddImg;
private:
    int             mHorPickerState;
    int             mHorSelectPos;

    int             mPageEditType;
    int             mFavEditPos;
};

#endif
