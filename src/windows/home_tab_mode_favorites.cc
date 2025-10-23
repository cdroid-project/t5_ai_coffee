/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:06:51
 * @FilePath: /t5_ai_coffee/src/windows/home_tab_mode_favorites.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "home_tab_mode_favorites.h"
#include "conf_mgr.h"
#include "wind_mgr.h"
#include "gaussfilterdrawable.h"

HomeTabModeFav::HomeTabModeFav(ViewGroup *wind_page_box,OnFavModeEditListener l):mWindPageBox(wind_page_box),mFavModeEditListener(l){
    // mFavList = g_objConf->getFavModeList();
    mFavListJson = g_objConf->getFavModeList();
    initGroup();
}

HomeTabModeFav::~HomeTabModeFav(){
    delete mFavAdapter;
    delete mLinearLyManager;
    delete mSimpleCallback;
    delete mItemTouchHelper;
}

void HomeTabModeFav::initGroup(){
    // 特定控件
    mCoffeeGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::layout_coffee);
    mFavListGroup = (ViewGroup *)mWindPageBox->findViewById(t5_ai_coffee::R::id::layout_fav_list);

    mFavRecycler = (RecyclerView *)mWindPageBox->findViewById(t5_ai_coffee::R::id::fav_list);

    mFavAdapter = new FavRecycAdapter(mFavRecycler);
    mSimpleCallback = new FavSimpleCallback(mFavAdapter);
    mLinearLyManager = new LinearLayoutManager(mWindPageBox->getContext(),RecyclerView::HORIZONTAL,false);
    mFavAdapter->setOnCardClickListener(std::bind(&HomeTabModeFav::onFavCardClickListener,this,std::placeholders::_1,std::placeholders::_2));
    mFavRecycler->setLayoutManager(mLinearLyManager);
    mFavRecycler->setAdapter(mFavAdapter);
    mItemTouchHelper = new ItemTouchHelper(mSimpleCallback);
    mItemTouchHelper->attachToRecyclerView(mFavRecycler);
}

void HomeTabModeFav::updateGroupData(bool isScrollList ){
    mFavListGroup->setVisibility(View::VISIBLE);
    mCoffeeGroup->setVisibility(View::GONE);
    mFavAdapter->notifyData();
    mFavListJson = g_objConf->getFavModeList();
    if(isScrollList) mFavRecycler->scrollToPosition(0);
}

void HomeTabModeFav::onFavCardClickListener(View &v ,int Position){
    switch(v.getId()){
        case t5_ai_coffee::R::id::fav_name_tv:{
            mFavListJson = g_objConf->getFavModeList();
            g_windMgr->showKeyBoardPage(getJsonString(mFavListJson[Position], "name"),"请输入收藏名称",20,
            [this,Position](std::string inputData){
                mFavListJson[Position]["name"] = inputData;
                g_objConf->editFavModeData(mFavListJson[Position],Position);
                mFavAdapter->notifyItem(Position);
            },nullptr);
            break;
        }case t5_ai_coffee::R::id::fav_make_tv:{
            mFavModeEditListener(Position,false);
            break;
        }case t5_ai_coffee::R::id::fav_edit_img:{
            mFavModeEditListener(Position,true);
            break;
        }
    }
}

void HomeTabModeFav::onItemClickListener(View &v){
    switch(v.getId()){
        default:{
            LOGE("HomeTabModeFav is no item click listener");
            break;
        }
    }
}

HomeTabModeFav::FavRecycAdapter::FavRecycAdapter(RecyclerView *rv):mRecyclerView(rv){
    mFavListJson = g_objConf->getFavModeList();
    mCardClickListener = nullptr;
}


RecyclerView::ViewHolder* HomeTabModeFav::FavRecycAdapter::onCreateViewHolder(ViewGroup* parent, int viewType){

    ViewGroup *convertView = (ViewGroup *)LayoutInflater::from(parent->getContext())->inflate("@layout/fav_card_layout", nullptr);
    
    return new FavRecycAdapter::ViewHolder(convertView);
}

void HomeTabModeFav::FavRecycAdapter::onBindViewHolder(RecyclerView::ViewHolder& holder, int position){
    
    ViewGroup *viewGroup = ((FavRecycAdapter::ViewHolder&)holder).viewGroup;

    viewGroup->setId(position + 10000);

    LOGE("viewGroup = %p holder = %p id = %d",viewGroup,&holder,viewGroup->getId());
    ImageView *picImg = (ImageView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_pic_img);
    ImageView *deleteImg = (ImageView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_delete_img);
    ImageView *editImg = (ImageView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_edit_img);

    TextView *nameTv = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_name_tv);

    TextView *infoTitleTv_1 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_title_tv_1);
    TextView *infoTv_1 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_tv_1);
    TextView *infoTitleTv_2 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_title_tv_2);
    TextView *infoTv_2 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_tv_2);
    TextView *infoTitleTv_3 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_title_tv_3);
    TextView *infoTv_3 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_tv_3);
    TextView *infoTitleTv_4 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_title_tv_4);
    TextView *infoTv_4 = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_info_tv_4);

    TextView *makeTv = (TextView *)viewGroup->findViewById(t5_ai_coffee::R::id::fav_make_tv);
    nameTv->setText(getJsonString(mFavListJson[position], "name"));
    Json::Value jsonSndModeData = mFavListJson[position]["sndModeData"];
    int beanGrindMode = getJsonInt(mFavListJson[position],"beanGrindMode");

    switch(getJsonInt(mFavListJson[position], "coffeeMode")){
        case HOME_MT_ESPRESSO:{
            picImg->setImageResource("@mipmap/pic_fav_esp");
            infoTitleTv_1->setText(beanGrindMode?"磨豆":"粉重");
            infoTitleTv_2->setText("萃取温度");
            infoTitleTv_3->setText("萃取水量");
            infoTitleTv_4->setText("预浸泡时间");
            
            infoTv_1->setText(std::to_string(getJsonInt(jsonSndModeData,"powderDef")) + (beanGrindMode?"s": "g"));
            infoTv_2->setText(std::to_string(getJsonInt(jsonSndModeData,"extractTempDef")) + "℃");
            infoTv_3->setText(std::to_string(getJsonInt(jsonSndModeData,"extractWaterDef")) + "ml");
            infoTv_4->setText(std::to_string(getJsonInt(jsonSndModeData,"soakingTimeDef")) + "s");
            break;
        }case HOME_MT_AMERICANO:{
            picImg->setImageResource("@mipmap/pic_fav_americano");
            infoTitleTv_1->setText(beanGrindMode?"磨豆":"粉重");
            infoTitleTv_2->setText("热水温度");
            infoTitleTv_3->setText("热水水量");
            infoTitleTv_4->setText("萃取水量");
            infoTv_1->setText(std::to_string(getJsonInt(jsonSndModeData,"powderDef")) + (beanGrindMode?"s": "g"));
            infoTv_2->setText(std::to_string(getJsonInt(jsonSndModeData,"hotWaterTempDef")) + "℃");
            infoTv_3->setText(std::to_string(getJsonInt(jsonSndModeData,"hotWaterDef")) + "ml");
            infoTv_4->setText(std::to_string(getJsonInt(jsonSndModeData,"extractWaterDef")) + "ml");
            break;
        }case HOME_MT_MASTER_ESPRESSO:{
            picImg->setImageResource("@mipmap/pic_fav_esp");
            infoTitleTv_1->setText(beanGrindMode?"磨豆":"粉重");
            infoTitleTv_2->setText("萃取温度");
            infoTitleTv_3->setText("萃取水量");
            infoTitleTv_4->setText("预浸泡时间");
            infoTv_1->setText(std::to_string(getJsonInt(jsonSndModeData,"powderDef")) + (beanGrindMode?"s": "g"));
            infoTv_2->setText(std::to_string(getJsonInt(jsonSndModeData,"extractTempDef")) + "℃");
            infoTv_4->setText(std::to_string(getJsonInt(jsonSndModeData,"soakingTimeDef")) + "s");

            int water = 0;
            Json::Value stepDataListJson = jsonSndModeData["stepDataList"];
            for (auto &sndDataItem: stepDataListJson){
                water += getJsonInt(sndDataItem,"water");
            }
            infoTv_3->setText(std::to_string(water) + "ml");
            break;
        }case HOME_MT_HAND_WASHED:{
            picImg->setImageResource("@mipmap/pic_fav_hand_wash");
            infoTitleTv_1->setText(beanGrindMode?"磨豆":"粉重");
            infoTitleTv_2->setText("萃取温度");
            infoTitleTv_3->setText("注水量");
            infoTitleTv_4->setText("中断时间");
            infoTv_1->setText(std::to_string(getJsonInt(jsonSndModeData,"powderDef")) + (beanGrindMode?"s": "g"));
            infoTv_2->setText(std::to_string(getJsonInt(jsonSndModeData,"extractTempDef")) + "℃");

            int water = 0, interTime = 0;
            Json::Value stepDataListJson = jsonSndModeData["stepDataList"];
            for (auto &sndDataItem: stepDataListJson){
                water += getJsonInt(sndDataItem,"water");
                interTime += getJsonInt(sndDataItem,"interTime");
            }
            infoTv_3->setText(std::to_string(water) + "ml");
            infoTv_4->setText(std::to_string(interTime) + "s");
            break;
        }case HOME_MT_EXTRACTING_TEA:{
            picImg->setImageResource("@mipmap/pic_fav_tea");

            infoTitleTv_1->setText("模式分类");
            infoTitleTv_2->setText("萃取温度");
            infoTitleTv_3->setText("注水量");
            infoTitleTv_4->setText("中断时间");
            infoTv_1->setText(getJsonString(jsonSndModeData,"sndModename"));
            infoTv_2->setText(std::to_string(getJsonInt(jsonSndModeData,"extractTempDef")) + "℃");
            infoTv_3->setText(std::to_string(getJsonInt(jsonSndModeData,"washTeaWater") + getJsonInt(jsonSndModeData,"soakTeaWater") + getJsonInt(jsonSndModeData,"makeTeaWater")) + "ml");
            infoTv_4->setText(std::to_string(getJsonInt(jsonSndModeData,"washTeaInterTime") + getJsonInt(jsonSndModeData,"soakTeaInterTime") + getJsonInt(jsonSndModeData,"makeTeaInterTime")) + "s");
            break;
        }
    }
    
    deleteImg->setOnClickListener([this,&holder](View &v){
        if(holder.isRecyclable()){
            LOGE("deleteImg click!  holder.getLayoutPosition() = %d",holder.getLayoutPosition());
            int Pos = holder.getLayoutPosition();
            notifyItemReMove(Pos);
        }
    });

    editImg->setOnClickListener([this,&holder](View &v){
        if(holder.isRecyclable()){
            LOGE("editImg click!");
            int Pos = holder.getLayoutPosition();
            if(mCardClickListener) mCardClickListener(v,Pos);
        }
    });

    makeTv->setOnClickListener([this,&holder](View &v){
        if(holder.isRecyclable()){
            LOGE("makeTv click!");
            int Pos = holder.getLayoutPosition();
            if(mCardClickListener) mCardClickListener(v,Pos);
        }
    });

    nameTv->setOnClickListener([this,&holder](View &v){
        if(holder.isRecyclable()){
            LOGE("nameTv click!");
            int Pos = holder.getLayoutPosition();
            if(mCardClickListener) mCardClickListener(v,Pos);
        }
    });
    
}

int HomeTabModeFav::FavRecycAdapter::getItemCount(){
    return mFavListJson.size();
}

void HomeTabModeFav::FavRecycAdapter::notifyData(){
    mFavListJson = g_objConf->getFavModeList();
    notifyDataSetChanged();
}

void HomeTabModeFav::FavRecycAdapter::notifyItem(int Pos){
    mFavListJson = g_objConf->getFavModeList();
    notifyItemChanged(Pos);
}

void HomeTabModeFav::FavRecycAdapter::notifyItemMove(int FromPos, int ToPos){
    if(mFavListJson.size() <= FromPos || mFavListJson.size() <= ToPos){
        LOGE("notifyItemReMove FromPos|ToPos >= mFavListJson.size()");
        return;
    }
    Json::Value temp;
    mFavListJson.removeIndex(FromPos,&temp);
    mFavListJson.insert(ToPos,temp);

    g_objConf->setFavModeList(mFavListJson);
    notifyItemMoved(FromPos,ToPos);
}

void HomeTabModeFav::FavRecycAdapter::notifyItemReMove(int Pos){
    if(Pos >= mFavListJson.size() || Pos < 0){
        LOGE("notifyItemReMove Position >= mFavListJson.size() ||  Pos < 0");
        return;
    }
    Json::Value deleteData;
    mFavListJson.removeIndex(Pos,&deleteData);
    g_objConf->setFavModeList(mFavListJson);
    if(mRecyclerView->getRecycledViewPool().getRecycledViewCount(0) >= 5){
        notifyDataSetChanged();
    }else{
        notifyItemRemoved(Pos);
    }
}

void HomeTabModeFav::FavRecycAdapter::setOnCardClickListener(OnCardClickListener l){
    mCardClickListener = l;
}

HomeTabModeFav::FavItemDecoration::FavItemDecoration(int firItemLeft, int betweenItemRight, int lastItemRight)
    :firstItemLeftSpacing(firItemLeft),betweenItemRightSpacing(betweenItemRight),lastItemRightSpacing(lastItemRight){

}

void HomeTabModeFav::FavItemDecoration::getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, RecyclerView::State& state){
    int position = parent.getChildAdapterPosition(&view);
    int itemCount = parent.getAdapter()->getItemCount();
    if(position == RecyclerView::NO_POSITION){
        position = parent.getChildViewHolder(&view)->getOldPosition();
        itemCount++;
    }
    if(position == 0){
        outRect.left = firstItemLeftSpacing;
        outRect.width = betweenItemRightSpacing;
    }else if(position == itemCount-1){
        outRect.width = lastItemRightSpacing;
    }else{
        outRect.width = betweenItemRightSpacing;
    }
}

HomeTabModeFav::FavSimpleCallback::FavSimpleCallback(FavRecycAdapter*adapter):ItemTouchHelper::SimpleCallback(
    ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT| ItemTouchHelper::UP | ItemTouchHelper::DOWN,
    ItemTouchHelper::DOWN){
    mAdapter = adapter;
}

bool HomeTabModeFav::FavSimpleCallback::onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target){
    const int fromPosition=viewHolder.getLayoutPosition();
    const int toPosition=target.getLayoutPosition();
    mAdapter->notifyItemMove(fromPosition, toPosition);
    return true;
}

void HomeTabModeFav::FavSimpleCallback::onSwiped(RecyclerView::ViewHolder& viewHolder, int direction){
    const int swipedPosition = viewHolder.getLayoutPosition();
    LOGE("direction=%d swipedPosition=%d",direction,swipedPosition);
    mAdapter->notifyItemReMove(swipedPosition);
}

void HomeTabModeFav::FavSimpleCallback::onChildDraw(Canvas& c, RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive){

    if(isCurrentlyActive){
       ViewGroup *viewGroup = ((FavRecycAdapter::ViewHolder&)viewHolder).viewGroup; 
       viewGroup->setScaleX(0.9);
       viewGroup->setScaleY(0.9);
    }else{
        ViewGroup *viewGroup = ((FavRecycAdapter::ViewHolder&)viewHolder).viewGroup; 
       viewGroup->setScaleX(1.0);
       viewGroup->setScaleY(1.0);
    }
    ItemTouchHelper::Callback::onChildDraw(c,recyclerView,viewHolder,dX,dY,actionState,isCurrentlyActive);
}

int HomeTabModeFav::FavSimpleCallback::interpolateOutOfBoundsScroll(RecyclerView& recyclerView, int viewSize, int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll) {
        int maxScroll = recyclerView.getHeight() / 2;
        float decay = 0.1f;
        int delta = (int) (viewSizeOutOfBounds * decay);
        if (delta < 0) {
            return std::max(delta, -maxScroll);
        } else {
            return std::min(delta, maxScroll);
        }
}
