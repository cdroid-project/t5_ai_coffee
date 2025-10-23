/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:16:00
 * @FilePath: /t5_ai_coffee/src/windows/pop_warn.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef _POP_WARN_H_
#define _POP_WARN_H_

#include <widget/textview.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopWarn:public PopBase{
public:

public:
    PopWarn(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb);
    ~PopWarn();

private:
    void initPopView()override;
    void initPopData()override;

    void updatePageData()override;
    void onBtnClickListener(View&v);

    void initWarnText();
protected:
    ViewGroup       *mPopGroup;

    ViewGroup       *mWarnInfoGroup;
    ViewGroup       *mWarnCleanGroup;

    ImageView       *mWarnImg;

    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;

    // info Group
    TextView        *mWarnTitleTv;
    TextView        *mWarnInfoTv;
    
    // clean Group
    TextView        *mWarnCleanInfoTv;
    TextView        *mWarnCleanSwitchTv;

private:
    std::string     mWarnTitleText; // 警告标题文本
    std::string     mWarnInfoText;  // 警告文本
    int             mWarnData;      // 所有警告
    int             mWarnShowType;  // 警告显示类型
};

#endif
