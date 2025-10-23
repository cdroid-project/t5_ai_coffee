/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:30:44
 * @FilePath: /t5_ai_demo/src/windows/page_screensaver.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef _PAGE_SCREENSAVER_H_
#define _PAGE_SCREENSAVER_H_

#include <widget/imageview.h>
#include <widget/textview.h>

#include "page_base.h"

class PageScreenSaver :public PageBase {
private:
    
protected:
public:
    PageScreenSaver(ViewGroup *wind_page_box,int page_type);
    ~PageScreenSaver();

public:
    void initPageView()override;
    void initPageData();
    void updatePageData()override;
    void stopPageAction()override;
private:
    void btnClickListener(View&view); // 按钮点击事件
private:
    ViewGroup   *mScreenSaverGroup;
    ImageView   *mWeatherImage;

protected:
};

#endif
