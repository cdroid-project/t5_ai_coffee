/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:31:03
 * @FilePath: /t5_ai_coffee/src/windows/page_keyboard.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#ifndef __PAGE_KEYBOARD_H__
#define __PAGE_KEYBOARD_H__

#include <widget/cdwindow.h>
#include <widget/textview.h>

#include <R.h>
#include <cdlog.h>
#include "cdroid_keyboard.h"
#include "page_base.h"
// typedef struct{
//     std::string type;
// }keyboardMap;

typedef std::function<void(std::string inputData)> input_callback;
typedef std::function<void()> exit_callback;
class PageKeyBoard: public PageBase {

    input_callback mPopEnterCallback;
    exit_callback mPopExitCallback;

    CdroidKeyBoard* mKeyBoard;

private:
    
    void btnClickListener(View& view);
protected:
public:
    PageKeyBoard(ViewGroup *wind_page_box,std::string iText,std::string description = "描述",int maxCount = 5,input_callback enterCallback=nullptr,exit_callback exitCallback=nullptr);
    ~PageKeyBoard();

    void initPageView()override;
    void initPageData()override;
    void updatePageData()override;
    void stopPageAction()override;
    void initPageData(std::string iText,std::string description,int maxCount,input_callback enterCallback,exit_callback exitCallback);
};

#endif

