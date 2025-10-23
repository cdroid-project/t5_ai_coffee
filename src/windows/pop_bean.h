/*
 * @Author: gezilong
 * @Email: ioremap@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:40:23
 * @FilePath: /t5_ai_coffee/src/windows/pop_bean.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by gezilong, All Rights Reserved. 
 * 
**/

#ifndef _POP_BEAN_H_
#define _POP_BEAN_H_

#include <widget/textview.h>
#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopBean:public PopBase{
public:

public:
    PopBean(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb);
    ~PopBean();

private:
    void initPopView()override;
    void initPopData()override;

    void updatePageData()override;
    void onBtnClickListener(View&v);

protected:
    TextView        *mBeanModeWeightTv;
    TextView        *mBeanModeTimeTv;

};

#endif
