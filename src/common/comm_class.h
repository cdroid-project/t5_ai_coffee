/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:49:53
 * @FilePath: /t5_ai_demo/src/common/comm_class.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/

#ifndef __comm_class_h__
#define __comm_class_h__

#include "comm_func.h"

#include <iostream>
#include <thread>
#include <list>

// 线程任务
class ThreadTask {
public:
    virtual int  onTask(void *data) = 0; // 执行任务 0-完成 !0-未完成
    virtual void onMain(void *data) = 0; // 任务执行完成，回到主线程
};

// 线程池
class ThreadPool : public EventHandler {    
protected:
    typedef enum {
        TS_NULL = 0,
        TS_IDLE,
        TS_BUSY,
        TS_OVER,
    }ThreadStatus;
    typedef struct {
        int         id;
        uchar       isDel;
        int64_t     atime;
        ThreadTask *sink;
        void       *data;
        bool        isRecycle;
    }TaskData;
    typedef struct {
        std::thread::id       tid;
        int       status;
        int64_t   itime;
        std::thread *th;
        TaskData *tdata;
    }ThreadData;
public:    
    static ThreadPool *ins();
    int init(int count);
    int add(ThreadTask *sink, void *data,bool isRecycle = false);
    int del(int taskId);
protected:
    ThreadPool();
    ~ThreadPool();
    virtual int checkEvents();
    virtual int handleEvents();

    void onThread(ThreadData *thData);
    ThreadData *getIdle();
private:
    bool                    mIsInit;
    int                     mTaskId;
    std::mutex              mMutex; 
    std::list<ThreadData *> mThreads;    
    std::list<TaskData *>   mWaitTasks; // 等待被执行的任务    
    std::list<TaskData *>   mMainTasks; // 执行完成的任务
};

// 使用cairo实现波浪
class waveAnim {    
public:
    typedef Cairo::RefPtr<Cairo::LinearGradient> linGradient;
    typedef enum{
        CLIP_TYPE_NONE = 0,
        CLIP_TYPE_CIRLE,    //圆形
        CLIP_TYPE_BESSEL,   //贝塞尔
        
    }WaveClipType;
protected:
    
    typedef struct{
        double x1;
        double y1;
        double x2;
        double y2;
        double x3;
        double y3;
        double x4;
        double y4;
    }BesseleData;

    typedef struct{
        int radius;
        int centerX;
        int centerY;
        double startAngle;
        double endAngle;
    }CirleData;

    typedef struct{
        int     waveHight;
        int     translationZ;
        linGradient     patternColor;
        float   speed;
        float   frequency;
        float   offsetX;
        float   offsetY;
        double  phase;
    }WaveDataStr;
    typedef struct{
        RectF rect;
        float offsetX;
        float offsetY;
    }BubbleDataStr;
protected:
    void drawWave(bool changePhase = true);
public:    
    waveAnim(View *view);
    ~waveAnim();
    void addWave(int waveH,float frequency,float speed,linGradient patternColor,float offsetX = 0,float offsetY = 0,int translationZ = 0);
    void startAnim();
    void endAnim(int endTime = 1);
    void endToHight(int hight);
    void setWaveY(int waveY);
    void changeWaveY(int waveY);
    void setClipTypes(WaveClipType type);
    void setCirleClipTypeData(int radius,int centerX,int centerY,double angle1, double angle2);
    void setBesselClipTypeData(double x1, double y1, double x2, double y2, double x3, double y3,double x4, double y4);
    void makeBubble(Canvas&canvas,float startX,float endX,float startY,float endY,float radius);
protected:
    Runnable        runAnim;
    Runnable        stopAnim;
    Runnable        endBubble;
    ValueAnimator  *endAnimator;
private:
    View*           mView;
    int             mWidth;
    int             mHight;
    int             mWaveY;
    int             mOldWaveY;
    int             mOffsetWaveY;
    bool            mIsRunAnim;
    bool            mIsStopAnim;
    float           mBubAlpha;
    WaveClipType    mClipTypes;
    BesseleData     mBesseleData;
    CirleData       mCirleData;
    std::vector<WaveDataStr>    mWaves;
    std::vector<BubbleDataStr>  mBubbleRects;
};

#endif
