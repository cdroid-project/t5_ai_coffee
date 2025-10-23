/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:49:46
 * @FilePath: /t5_ai_demo/src/common/comm_class.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "comm_class.h"
#include <core/app.h>
////////////////////////////////////////////////////////////////////////////////////////
ThreadPool *ThreadPool::ins() {
    static ThreadPool sIns;
    return &sIns;
}

ThreadPool::ThreadPool() {
    mIsInit = false;
    mTaskId = 0;
    App::getInstance().addEventHandler(this);
}

ThreadPool::~ThreadPool() {
    App::getInstance().removeEventHandler(this);

    for (ThreadData *td : mThreads) { td->status = TS_OVER; }
    mThreads.clear();
}

int ThreadPool::init(int count) {
    if (mIsInit) return 0;
    mIsInit = true;
    if (count <= 0) { count = 1; }
    for (int i = 0; i < count; i++) {
        ThreadData *thData = new ThreadData();
        thData->status     = TS_NULL;
        thData->itime      = 0;
        thData->tdata      = 0;
        thData->th         = new std::thread(std::bind(&ThreadPool::onThread, this, std::placeholders::_1), thData);
        thData->th->detach();
        mThreads.push_back(thData);
    }
    return 0;
}

int ThreadPool::add(ThreadTask *sink, void *data,bool isRecycle) {
    if (!sink) return -1;

    TaskData *td = new TaskData();
    td->id       = ++mTaskId;
    td->isDel    = 0;
    td->atime    = SystemClock::uptimeMillis();
    td->sink     = sink;
    td->data     = data;
    td->isRecycle  = isRecycle;
    mWaitTasks.push_back(td);

    LOGV("task add. id=%d", td->id);

    return td->id;
}

int ThreadPool::del(int taskId) {
    for (ThreadData *td : mThreads) {
        if (td->tdata->id == taskId) {
            td->tdata->isDel = 1;
            return 0;
        }
    }
    for (TaskData *td : mWaitTasks) {
        if (td->id == taskId) {
            td->isDel = 1;
            return 0;
        }
    }

    return -1;
}

int ThreadPool::checkEvents() {
    return mWaitTasks.size() + mMainTasks.size();
}

int ThreadPool::handleEvents() {
    // 处理执行完成的任务

    while (mWaitTasks.size()) {
        ThreadData *th = getIdle();
        if (!th) break;

        TaskData *td = mWaitTasks.front();
        th->tdata    = td;
        th->status   = TS_BUSY;
        LOG(VERBOSE) << "add thread. id=" << td->id << " tid=" << th->tid;

        mWaitTasks.pop_front();
    }

    int64_t startTime, diffTime;
    startTime = SystemClock::uptimeMillis();
    mMutex.lock();
    for (int i = 0; i < 3 && mMainTasks.size(); i++) {
        TaskData *td = mMainTasks.front();
        LOGV("task end. id=%d del=%d time=%dms", td->id, (int)td->isDel, int(startTime - td->atime));
        if (td->isDel == 0) td->sink->onMain(td->data);
        mMainTasks.pop_front();
        if(td->isRecycle) delete td->sink;
        delete td;

        diffTime = SystemClock::uptimeMillis() - startTime;
        if (diffTime > 500) {
            LOGW("Task on main more time. %dms", (int)diffTime);
            break;
        }
    }
    mMutex.unlock();

    return 1;
}

void ThreadPool::onThread(ThreadData *thData) {
    int ret,idleCount;
    thData->tid    = std::this_thread::get_id();
    thData->status = TS_IDLE;
    thData->itime  = SystemClock::uptimeMillis();

    LOG(VERBOSE) << "new thread. id=" << thData->tid;

    idleCount = 0;
    while (thData->status != TS_OVER) {
        if (thData->status == TS_BUSY) {
            LOGV("on task beg. id=%d", thData->tdata->id);
            if ((thData->tdata->isDel && (ret = 10000)) || (ret = thData->tdata->sink->onTask(thData->tdata->data)) == 0) {
                mMutex.lock();
                mMainTasks.push_back(thData->tdata);
                mMutex.unlock();
                thData->itime  = SystemClock::uptimeMillis();
                thData->status = TS_IDLE;
            }
            LOGV("on task end. id=%d ret=%d", thData->tdata->id, ret);
        } else {
            idleCount++;
            if (idleCount >= 10) {
                idleCount = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }            
        }
    }

    delete thData;
}

ThreadPool::ThreadData *ThreadPool::getIdle() {
    for (ThreadData *td : mThreads) {
        if (td->status == TS_IDLE) { return td; }
    }
    return 0;
}



waveAnim::waveAnim(View *view) {
    mView       = view;
    mWaveY      = 0;
    mOldWaveY   = 0;
    mWidth      = 0;
    mHight      = 0;
    mOffsetWaveY= 0;
    mIsRunAnim  = false;
    mIsStopAnim = false;
    mBubAlpha   = 1.0f;
    mClipTypes  = CLIP_TYPE_NONE;
    runAnim = [this](){
        if(mIsRunAnim){
            drawWave();
            mView->postDelayed(runAnim,40);
        }
        LOGV("running Anim mView = %p",mView);
    };
    stopAnim = [this](){
        if(mIsStopAnim){
            drawWave(false);
            mView->postDelayed(stopAnim,40);
        }
    };
    endBubble = [this](){

    };
    endAnimator = ValueAnimator::ofFloat({1.f,0.f});
    endAnimator->setInterpolator(LinearInterpolator::gLinearInterpolator.get()); // 设置动画的插值器
    endAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        // 获取当前动画的值
        const float t = GET_VARIANT(anim.getAnimatedValue(),float);
        mBubAlpha  = t;
        if(mOffsetWaveY != 0){
            mOldWaveY = mWaveY + mOffsetWaveY*(t);
        }
        if(t == 0.0f) mIsStopAnim = false;
        LOGV("t = %f,mOffsetWaveY = %d mWaveY = %d mOldWaveY = %d",t,mOffsetWaveY,mWaveY,mOldWaveY); 
    }));
}

waveAnim::~waveAnim(){
    endAnimator->cancel();
    delete endAnimator;
}

void waveAnim::addWave(int waveH,float frequency,float speed,linGradient patternColor,float offsetX,float offsetY,int translationZ){
    WaveDataStr data;
    data.waveHight      = waveH;
    data.frequency      = frequency;
    data.speed          = speed;
    data.patternColor   = patternColor;
    data.offsetX        = offsetX;
    data.offsetY        = offsetY;
    data.translationZ   = translationZ;
    data.phase          = 0;
    mWaves.push_back(data);
    // 按照 translationZ 的值对 mWaves 进行排序
    std::sort(mWaves.begin(), mWaves.end(), [](const WaveDataStr& a, const WaveDataStr& b) {
        return a.translationZ < b.translationZ;
    });
}

// 启动动画
void waveAnim::startAnim(){
    if(!mIsRunAnim){
        mIsRunAnim = true;
        mBubAlpha  = 1.f; 
        mView->post([this](){mWidth = mView->getWidth(); mHight = mView->getHeight();});
        endAnimator->pause();
        mView->removeCallbacks(stopAnim);
        mView->postDelayed(runAnim,40);
        LOGV("start anim *** mView = %p",mView);
    }
}

// 结束动画。可在结束之前再执行一次changeWaveY，在结束前会缓慢变换到目前值
void waveAnim::endAnim(int endTime){
    mIsRunAnim = false;
    mIsStopAnim = true;
    mOffsetWaveY = mOldWaveY - mWaveY;
    int ret = mView->removeCallbacks(runAnim);
    LOGE("removeCallbacks end anim *** mView = %p ret = %d",mView,ret);
    endAnimator->setDuration(endTime * 1000);
    mView->postDelayed(stopAnim,40);
    endAnimator->start();
}

// 马上结束动画，并刷新到指定位置
void waveAnim::endToHight(int hight){
    mIsRunAnim = false;
    mIsStopAnim = false;
    mOldWaveY = hight;
    mWaveY = hight;
    drawWave();
}

// 更改波浪的Y轴位置，马上变化到目前Y值
void waveAnim::setWaveY(int waveY){
    mWaveY = waveY;
    mOldWaveY = waveY;
}

// 更改波浪的Y轴位置，会根据每次刷新缓慢更改到目标Y值
void waveAnim::changeWaveY(int waveY){
    mWaveY = waveY;
}

void waveAnim::drawWave(bool changePhase){
    if(changePhase){
        if(std::abs(mWaveY-mOldWaveY) > 3)  mOldWaveY = (mWaveY-mOldWaveY)>0?mOldWaveY+3:mOldWaveY-3;
        else mOldWaveY = mWaveY;
    }
    

    Cairo::RefPtr<Cairo::ImageSurface> bitmap = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mWidth, mHight);   
    Canvas canvas(bitmap);
    switch(mClipTypes){
        case CLIP_TYPE_CIRLE:{
            canvas.arc(mCirleData.centerX,
                        mCirleData.centerY,
                        mCirleData.radius,
                        mCirleData.startAngle,
                        mCirleData.endAngle);
            canvas.clip();
            break;
        }case CLIP_TYPE_BESSEL:{
            canvas.move_to(mBesseleData.x1,mBesseleData.y1);
            canvas.curve_to(mBesseleData.x2,mBesseleData.y2,
                            mBesseleData.x3,mBesseleData.y3,
                            mBesseleData.x4,mBesseleData.y4);
            canvas.clip();
            break;
        }
    }
    
    for(int i=0; i<mWaves.size(); i++){
        float startY = mOldWaveY+mWaves.at(i).offsetY;
        float offsetX = mWaves.at(i).offsetX;
        int waveH  = mWaves.at(i).waveHight;
        float frequency = mWaves.at(i).frequency;
        
        canvas.set_source(mWaves.at(i).patternColor);
        canvas.move_to(0,startY); // 移动到起始点

        // canvas.set_line_width(2.0);
        for (int x = 0; x < mWidth; x++) {
            double y = startY + waveH * std::sin(2 * M_PI * frequency * x  + mWaves.at(i).phase + M_PI + offsetX);
            canvas.line_to(x, y);
        }
        canvas.line_to(mWidth,mHight);
        canvas.line_to(0, mHight);

        canvas.close_path(); // 闭合路径
        canvas.fill(); // 绘制线条
        // 更新相位，实现波浪移动
        mWaves.at(i).phase += mWaves.at(i).speed;
        LOGV("startY = %f waveH = %d ,frequency = %f",startY,waveH,frequency);
    }
    makeBubble(canvas,0,mWidth,mHight,mHight,5);

    Drawable *drawable = new BitmapDrawable(bitmap);
    mView->setBackground(drawable);
    // mView->invalidate();
}

// 设置裁剪的类型
void waveAnim::setClipTypes(WaveClipType type){
    mClipTypes = type;
}

/* @param radius	半径
   * @param centerX	圆心点X
   * @param centerY	圆心点Y
   * @param angle1	圆的起始弧度
   * @param angle2	圆的结束弧度
*/
void waveAnim::setCirleClipTypeData(int radius,int centerX,int centerY,double angle1, double angle2){
    mCirleData.radius   = radius;
    mCirleData.centerX  = centerX;
    mCirleData.centerY  = centerY;
    mCirleData.startAngle = angle1;
    mCirleData.endAngle = angle2;
}

/* @param x1 y1	起始坐标点
   * @param x2 y2	第一个控制点
   * @param x3 y3	第二个控制点
   * @param x4 y4	结束点
*/
void waveAnim::setBesselClipTypeData(double x1, double y1, double x2, double y2, double x3, double y3,double x4, double y4){
    mBesseleData.x1 = x1;
    mBesseleData.y1 = y1;
    mBesseleData.x2 = x2;
    mBesseleData.y2 = y2;
    mBesseleData.x3 = x3;
    mBesseleData.y3 = y3;
    mBesseleData.x4 = x4;
    mBesseleData.y4 = y4;
}

void waveAnim::makeBubble(Canvas&canvas,float startX,float endX,float startY,float endY,float radius){
    BubbleDataStr bubData;
    bubData.rect.left = startX + rand()%(int)(endX-startX-2*radius);
    bubData.rect.top    = startY + rand()%(int)(endY-startY-2*radius);
    bubData.rect.height = (radius-rand()%3)*2;
    bubData.rect.width  = bubData.rect.height;
    bubData.offsetX = 0;
    bubData.offsetY = 0;

    if(rand()%4 >= 3 ) mBubbleRects.push_back(bubData);
    if(mBubAlpha == 0.f) mBubbleRects.clear();

    double centerX,centerY;
    for(int i=0; i< mBubbleRects.size(); i++){
        if(rand()%4 >= 3 ){
            mBubbleRects.at(i).offsetX = (rand()%3 - 1)*5;
        }
        mBubbleRects.at(i).rect.left -= mBubbleRects.at(i).offsetX;
        mBubbleRects.at(i).rect.top -= rand()%5 + 10;
        centerX = mBubbleRects.at(i).rect.centerX();
        centerY = mBubbleRects.at(i).rect.centerY();
        canvas.arc(centerX,centerY,mBubbleRects.at(i).rect.height/2,0,2*M_PI); 
        canvas.set_source_rgba(1.0, 1.0, 1.0,mBubAlpha);
        canvas.set_line_width(1.0);
        canvas.stroke();
    }
}