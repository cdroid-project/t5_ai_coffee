/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:07:07
 * @FilePath: /t5_ai_coffee/src/windows/home_tab_mode_favorites.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#ifndef _HOME_TAB_MODE_FAVORITES_H_
#define _HOME_TAB_MODE_FAVORITES_H_

#include "data.h"

#include <widget/imageview.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>

class HomeTabModeFav {
public:
    DECLARE_UIEVENT(void,OnFavModeEditListener,int Position, bool isEdit);
private:  
    class FavRecycAdapter;
    class FavSimpleCallback;
public:
    HomeTabModeFav(ViewGroup *wind_page_box,OnFavModeEditListener l);
    ~HomeTabModeFav();

    void updateGroupData(bool isScrollList = true);         // 页面  更新数据
    void onItemClickListener(View &v);
private:
    void initGroup();
    void onFavCardClickListener(View &v ,int Position);
private:
    ViewGroup       *mWindPageBox;

////////////////// 各种模式的数据 ////////////////////
    Json::Value mFavListJson;

////////////////// 特定控件 //////////////////////
    ViewGroup       *mCoffeeGroup;
    ViewGroup       *mFavListGroup;

    RecyclerView    *mFavRecycler;
    LinearLayoutManager *mLinearLyManager;
    FavRecycAdapter *mFavAdapter;
    FavSimpleCallback  *mSimpleCallback;
    ItemTouchHelper    *mItemTouchHelper;
private:
    OnFavModeEditListener mFavModeEditListener;
private:


// recyclerView  的 适配器
class FavRecycAdapter : public RecyclerView::Adapter{
public:
    DECLARE_UIEVENT(void, OnCardClickListener ,View & ,int);
private:

    static constexpr int VIEW_TYPE_HEADER   = 0;
    static constexpr int VIEW_TYPE_ITEM     = 1;

    RecyclerView *mRecyclerView;
    Json::Value mFavListJson;

    OnCardClickListener mCardClickListener;
public:
    class ViewHolder:public RecyclerView::ViewHolder {
    public:
        ViewGroup* viewGroup;
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            viewGroup =(ViewGroup*)itemView;
        }
    };

    FavRecycAdapter(RecyclerView *rv);
    ~FavRecycAdapter(){LOGE("~RecycAdapter()");}

    RecyclerView::ViewHolder*onCreateViewHolder(ViewGroup* parent, int viewType)override;
    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override;
    int getItemCount()override;
    // void onViewRecycled(RecyclerView::ViewHolder& holder)override;

    void notifyData();
    void notifyItem(int Pos);

    void notifyItemMove(int FromPos, int ToPos);
    void notifyItemReMove(int Pos);
    void setOnCardClickListener(OnCardClickListener l);
};

// recyclerview 每一项之间的间隔
class FavItemDecoration : public RecyclerView::ItemDecoration{
private:
    int firstItemLeftSpacing;
    int betweenItemRightSpacing;
    int lastItemRightSpacing;

public:
    FavItemDecoration(int firItemLeft, int betweenItemRight, int lastItemRight);

    void getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state) override;
};

class FavSimpleCallback: public ItemTouchHelper::SimpleCallback{
private:
    FavRecycAdapter *mAdapter;
public:
    FavSimpleCallback(FavRecycAdapter*adapter);
    bool onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target)override;
    void onSwiped(RecyclerView::ViewHolder& viewHolder, int direction)override;
    void onChildDraw(Canvas& c, RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive)override;
    int interpolateOutOfBoundsScroll(RecyclerView& recyclerView, int viewSize, int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll)override;
};
};

#endif
