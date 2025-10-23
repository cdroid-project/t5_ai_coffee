/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:35:31
 * @FilePath: /t5_ai_coffee/src/viewlibs/nestedscrollgroup.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "nestedscrollgroup.h"

DECLARE_WIDGET(NestedScrollGroup)

NestedScrollGroup::NestedScrollGroup(int w, int h):RelativeLayout(w,h){
    isStartAnim = false;
}

NestedScrollGroup::NestedScrollGroup(Context *ctx, const AttributeSet &attrs):RelativeLayout(ctx,attrs){
    isStartAnim = false;
}

// 启动横向嵌套滑动
bool NestedScrollGroup::onStartNestedScroll(View* child, View* target, int nestedScrollAxes){
    return (nestedScrollAxes & View::SCROLL_AXIS_HORIZONTAL) != 0;
}

// 停止嵌套Fling滑动
bool NestedScrollGroup::onNestedPreFling(View* target, float velocityX, float velocityY){
    return false;
}

// 嵌套滑动 前置处理
// 若已经存在translationX，则优先处理translationX，不处理内置控件的scrollX
void NestedScrollGroup::onNestedPreScroll(View* target, int dx, int dy,int consumed[], int type){
    if(type == TYPE_TOUCH){
        int translationX = target->getTranslationX();
        if(((target->getTranslationX() > 0) && (dx > 0))
            || (target->getTranslationX() < 0) && (dx < 0)){
            if(std::abs(dx) > std::abs(target->getTranslationX())){
                dx = target->getTranslationX();
            }
            consumed[0] = dx;
            target->setTranslationX(translationX-dx);
        }
    }
}

// 嵌套滑动 后置处理，处理内置控件是translationX
void NestedScrollGroup::onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int type,int*consumed){
    if(type == TYPE_TOUCH){
        if(isStartAnim)target->animate().cancel();
        dxUnconsumed = (dxUnconsumed>0?-1:1)*std::sqrt(std::abs(dxUnconsumed*2));
        if(dxUnconsumed != 0 ){
            target->setTranslationX(dxUnconsumed+target->getTranslationX());
        }
        isStartAnim = false;
    }
}

// 嵌套滑动 停止滑动
void NestedScrollGroup::onStopNestedScroll(View* target){
    if(!isStartAnim){
        target->animate().setDuration(600).translationX(0).start();
        isStartAnim = true;
    }
}

