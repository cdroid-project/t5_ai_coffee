/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:30:00
 * @FilePath: /t5_ai_coffee/src/windows/page_setting.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "page_keyboard.h"
#include "wind_mgr.h"

PageKeyBoard::PageKeyBoard(ViewGroup *wind_page_box,std::string iText, std::string description,int maxCount,input_callback enterCallback,exit_callback exitCallback)
    :PageBase(wind_page_box,"@layout/page_keyboard",PAGE_KEYBOARD),
    mPopEnterCallback(enterCallback),mPopExitCallback(exitCallback){

    initPageView();
    initPageData(iText,description,maxCount,enterCallback,exitCallback);
}

void PageKeyBoard::btnClickListener(View& view) {
    switch(view.getId()){
        case t5_ai_coffee::R::id::keyboard_enter:{
            if(mPopEnterCallback) mPopEnterCallback(mKeyBoard->getEnterText());
            g_windMgr->dealCloseKeyBoardPage();
            break;
        }
        case t5_ai_coffee::R::id::keyboard_cancel:{
            if(mPopExitCallback) mPopExitCallback();
            g_windMgr->dealCloseKeyBoardPage();
            break;
        }
    }
}

PageKeyBoard::~PageKeyBoard() {
    removePage();
}

void PageKeyBoard::initPageView(){
    auto btn_click_func = std::bind(&PageKeyBoard::btnClickListener, this, std::placeholders::_1);
    mPageLayout->findViewById(t5_ai_coffee::R::id::keyboard_enter)->setOnClickListener(btn_click_func);
    mPageLayout->findViewById(t5_ai_coffee::R::id::keyboard_cancel)->setOnClickListener(btn_click_func);
    mKeyBoard = (CdroidKeyBoard *)mPageLayout->findViewById(t5_ai_coffee::R::id::cdroid_keyboard);

    mKeyBoard->setOnCompleteListener([this](const std::string& txt) {
        LOG(DEBUG) << "editText :" << txt;
        if(mPopEnterCallback) mPopEnterCallback(mKeyBoard->getEnterText());
    });
    
    mKeyBoard->setEditType(EditText::TYPE_ANY);

    mKeyBoard->showWindow();
    
}
void PageKeyBoard::initPageData(){

}
void PageKeyBoard::initPageData(std::string iText,std::string description,int maxCount,input_callback enterCallback,exit_callback exitCallback){
    if(maxCount < 0) maxCount = 999;
    mKeyBoard->setWordCount(maxCount);
    mKeyBoard->setDescription(std::string(" ") + description);
    mKeyBoard->setEditText(iText);

    mPopEnterCallback = enterCallback;
    mPopExitCallback = exitCallback;
}

void PageKeyBoard::updatePageData(){

}

void PageKeyBoard::stopPageAction(){

}
