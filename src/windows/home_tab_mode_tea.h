/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 17:42:15
 * @LastEditTime: 2025-10-23 14:07:58
 * @FilePath: /t5_ai_coffee/src/windows/home_tab_mode_tea.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#ifndef _HOME_TAB_MODE_TEA_H_
#define _HOME_TAB_MODE_TEA_H_

#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <widget/numberpicker.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>
#include "arc_seekbar.h"

class HomeTabModeTea {
class TeaSimpleCallback;
private:  
    class TeaRecycAdapter;
    // 萃茶模式 洗茶 冲泡  泡茶（再萃）
    enum{ 
        MAKE_TEA_WASH,      // 洗茶
        MAKE_TEA_EXT,       // 冲泡
        MAKE_TEA_REBREW,    // 再萃
    };
public:
    HomeTabModeTea(ViewGroup *wind_page_box);
    ~HomeTabModeTea();

    void updateGroupData();         // 页面  更新数据
    void updateRightInfoGroup();    // 右边数据（出水量、萃取水量等等） 更新
    void updateCoffeeGroupData();   // 更新咖啡页面数据
    void updatePageData();
    void updateBeanCupImgStatus();
    void onItemClickListener(View &v);
    void onArcValueChangeListener(View &v, int progress, bool fromUser);
    void onHorPickerChange();
    void onFavModeEditListener(int Position, bool isEdit);
    void dealTuyaStartWork();
    void dealAiData();
    void stopWork();
    void startWork();
    Json::Value getCacheDataToJson();
private:
    void initGroup();
    void showHandWashPop(int selectStep);   // 显示 手冲咖啡 的修改弹窗页面
    void setFormData();                     // 设置   表格数据
    void setStepStatus();                   // 设置 当前步骤
    void dealRightInfoAddReductClick(bool isAdd);
    void onTeaCardClickListener(View &v ,int Position);
    void dealPowderItemClick(View &v);
private:
    ViewGroup       *mWindPageBox;

////////////////// 公用控件 ////////////////////

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
    ImageView       *mTeaRebrewImg;     // 萃茶的再萃按钮

    ViewGroup       *mHomeFavGroup; // 主页的收藏 group (手冲模式、萃茶)
    ViewGroup       *mHomeTipsGroup;// 主页的Tips group (手冲模式、萃茶)
    TextView        *mHomeTipsTeaTv;
    ViewGroup       *mHomeTipsHandWashGroup;  
    
    ImageView       *mRightInfoReduceImg;  // 右边信息 减少按钮
    ImageView       *mRightInfoAddImg;     // 右边信息 增加按钮

    AnimatedImageDrawable *mRightAnim;
////////////////// 各种模式的数据 ////////////////////
    ExtractTeaDataStr mExtTeaData;

    ExtractTeaSndModeDataStr mCacheData;
////////////////// 特定控件 //////////////////////
    ViewGroup       *mCoffeeGroup;      // 咖啡的容器
    ViewGroup       *mTeaListGroup;     // 萃茶列表的容器
    RecyclerView    *mTeaListRecycler;
    LinearLayoutManager *mLinearLyManager;
    TeaRecycAdapter *mTeaAdapter;
    TeaSimpleCallback  *mSimpleCallback;
    ItemTouchHelper    *mItemTouchHelper;

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
    ImageView       *mFavBackImg;       // 收藏功能页面的返回按钮（也适用于萃茶步骤的返回）
private:
    int             mHorSelectPos;
    int             mBeanItemState;
    int             mCupItemState;
    int             mExtractStep;

    int             mPageEditType;
    int             mFavEditPos;
private:
// recyclerView  的 适配器
class TeaRecycAdapter : public RecyclerView::Adapter{
public:
    DECLARE_UIEVENT(void, OnCardClickListener ,View & ,int);
private:

    ViewGroup   *mWindPageBox;
    std::vector<ExtractTeaSndModeDataStr> teaModeList;

    OnCardClickListener mCardClickListener;
public:
    class ViewHolder:public RecyclerView::ViewHolder {
    public:
        ViewGroup* viewGroup;
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            viewGroup =(ViewGroup*)itemView;
        }
    };

    TeaRecycAdapter(ViewGroup *wind_page_box);
    ~TeaRecycAdapter(){LOGE("~RecycAdapter()");}

    RecyclerView::ViewHolder*onCreateViewHolder(ViewGroup* parent, int viewType)override;
    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override;
    int getItemCount()override;
    // void onViewRecycled(RecyclerView::ViewHolder& holder)override;
    void notifyItemMove(int FromPos, int ToPos);
    void setOnCardClickListener(OnCardClickListener l);
};

// recyclerview 每一项之间的间隔
class TeaItemDecoration : public RecyclerView::ItemDecoration{
private:
    int firstItemLeftSpacing;
    int betweenItemRightSpacing;
    int lastItemRightSpacing;

public:
    TeaItemDecoration(int firItemLeft, int betweenItemRight, int lastItemRight);

    void getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state) override;
};

class TeaSimpleCallback: public ItemTouchHelper::SimpleCallback{
private:
    TeaRecycAdapter *mAdapter;
public:
    TeaSimpleCallback(TeaRecycAdapter*adapter);
    bool onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target)override;
    void onSwiped(RecyclerView::ViewHolder& viewHolder, int direction)override;
    void onChildDraw(Canvas& c, RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive)override;
    int interpolateOutOfBoundsScroll(RecyclerView& recyclerView, int viewSize, int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll)override;
};
};

#endif
