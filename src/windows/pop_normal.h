/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:29:16
 * @FilePath: /t5_ai_coffee/src/windows/pop_normal.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef _POP_NORMAL_H_
#define _POP_NORMAL_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>
#include <widget/progressbar.h>
#include <widget/scrollview.h>
#include <widgetEx/qrcodeview.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopNormal:public PopBase{
public:

public:
    PopNormal(ViewGroup *wind_pop_box,View *gaussFromView,int popType,appCb enterCb,appCb cancelCb);
    ~PopNormal();

private:
    void initPopView()override;
    void initPopData()override;

    void updatePageData()override;
    void onBtnClickListener(View&v);

protected:
    ViewGroup       *mPopGroup;

    ViewGroup       *mNormalGroup;
    ViewGroup       *mOTAGroup;
    ViewGroup       *mBindAppGroup;

    /// OTA Group
    TextView        *mPopOtaInfoTv;
    TextView        *mPopOtaProgressTv;
    ProgressBar     *mPopOtaProgress;

    TextView        *mPopOtaTitleTv;
    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;

    // normal Group
    ViewGroup       *mPopInfoGroup;
    ViewGroup       *mOTAInfoScrollGroup;
    

    TextView        *mPopTitleTv;
    TextView        *mPopInfoTv;
    ImageView       *mPopImg;
    TextView        *mPopOtaScrollInfoTv;

    // bind app Group
    ViewGroup       *mBindAppImgGroup;
    QRCodeView      *mBindAppImg;
    TextView        *mBindAppCilckTipsTv;

    TextView        *mBindAppInfoTv;
 
    int             mUpDateFlag;
    int             mOtaProgress;

    Runnable        mCloseRun;
    Runnable        mConnectRun;
};

#endif
