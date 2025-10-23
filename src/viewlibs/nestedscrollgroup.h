/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:35:26
 * @FilePath: /t5_ai_coffee/src/viewlibs/nestedscrollgroup.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


/*
 * 嵌套滚动的group，与recyclerview搭配使用
 */
#ifndef __NESTED_SCROLL_GROUP_H
#define __NESTED_SCROLL_GROUP_H

#include <widget/relativelayout.h>


class NestedScrollGroup : public RelativeLayout {
public:
    NestedScrollGroup(int w, int h);
    NestedScrollGroup(Context *ctx, const AttributeSet &attrs);

    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    void onNestedPreScroll(View* target, int dx, int dy,int consumed[], int type)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int type,int*consumed)override;
    void onStopNestedScroll(View* target)override;
    bool onNestedPreFling(View* target, float velocityX, float velocityY)override;
protected:
    bool isStartAnim;
};

#endif
