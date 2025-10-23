/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:05
 * @FilePath: /t5_ai_demo/src/protocol/btn_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "btn_mgr.h"

#include <core/app.h>

#include "conf_mgr.h"
#include "data.h"
#include "wind_mgr.h"

#include "conn_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define GPIO_INPUT_PIN "0"
#define GPIO_OUTPUT_PIN "55"
#define GPIO_BASE_PATH "/sys/class/gpio"
#define TIME_THRESHOLD 20
#define TIME_ERROR 1
#define INTERRUPT_COUNT 5

/////////////////////////////////////////////////////////////////////////////

#define TICK_TIME 200 // tick触发时间（毫秒）

//////////////////////////////////////////////////////////////////

BtnMgr::BtnMgr() {

    mIsFactory          = false;
    mIsRunning          = false;
    mIsShutDown         = false;
}

BtnMgr::~BtnMgr() {
    mIsRunning = false;
}

int BtnMgr::init() {
    mIsRunning = true;
#ifndef CDROID_X64
    std::thread th = std::thread(&BtnMgr::threadFunction,this);
    th.detach();
    App::getInstance().addEventHandler(this);
#endif
    return 0;
}

int BtnMgr::checkEvents() {
    int64_t curr_tick = SystemClock::uptimeMillis();
    if (curr_tick >= mNextEventTime) {
        mNextEventTime = curr_tick + TICK_TIME;
        return 1;
    }
    return 0;
}

int BtnMgr::handleEvents() {
    int64_t now_tick = SystemClock::uptimeMillis();

    if (mIsShutDown) { 
        if(g_appData.mIsFactory)    shutDown();
        else                        g_objConnMgr->setShutDown();
    }

    return 1;
}

// 关机
void BtnMgr::shutDown(){
    char path[64];
    snprintf(path, sizeof(path), "%s/gpio%s/value", GPIO_BASE_PATH, GPIO_OUTPUT_PIN);

    int output_io_fd = open(path, O_WRONLY);
    if (output_io_fd < 0)
    {
        perror("Failed to open output GPIO value file");
        return;
    }

    while (1)
    {
        write(output_io_fd, "1", 1);
        usleep(16*1000);
        write(output_io_fd, "0", 1);
        usleep(16*1000);
    }

    close(output_io_fd);
}


void BtnMgr::setFactoryCallback(factoryCb cb){
    BuffData *mCurrRecv;
    if(cb){
        mIsFactory = true;
        
    }else{
        mIsFactory = false;

    }
    
}

// 产测指令
void BtnMgr::sendFactoryData(){
    
    // mSetData = 0;
    // send2MCU();
    // setBuzzer_SetBit(1,1,1);
}

void BtnMgr::threadFunction(){
    char path[64] = {0};
    char buf[8] = {0};
    int input_io_fd;

    snprintf(path, sizeof(path), "%s/gpio%s/value", GPIO_BASE_PATH, GPIO_INPUT_PIN);
    input_io_fd = open(path, O_RDONLY);
    if (input_io_fd < 0)
    {
        perror("Failed to open input GPIO value file");
        return;
    }

    struct pollfd pfd;
    pfd.fd = input_io_fd;
    pfd.events = POLLPRI;

    int count = 0;

    uint64_t LastTime = 0, nowTime = 0;
    while (mIsRunning)
    {
        lseek(input_io_fd, 0, SEEK_SET);
        read(input_io_fd, buf, sizeof(buf));
        int ret = poll(&pfd, 1, 1000);

        if (ret > 0 && (pfd.revents & POLLPRI))
        {
            nowTime = SystemClock::uptimeMillis();
            if (nowTime - LastTime > 0)
            {
                uint64_t diff = nowTime - LastTime;
                if (diff >= TIME_THRESHOLD - TIME_ERROR && diff <= TIME_THRESHOLD + TIME_ERROR)
                    count++;
                else
                    count = 0;
            }
            LastTime = nowTime;

            if (count >= INTERRUPT_COUNT){
                LOGE("PWM ON!!!");
                mIsShutDown = true;
                g_appData.isShutDown = true;
                mNextEventTime = nowTime -1;
                break;
            }
        }
        else if (ret == 0){
            count = 0;
            LOGI("no signal");
        }else{
            count = 0;
            LOGE("poll error !!!!!!!!!!!!");
            break;
        }
        // usleep(1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    close(input_io_fd);
}