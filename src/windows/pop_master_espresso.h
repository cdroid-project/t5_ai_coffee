#ifndef _POP_MASTER_ESPRESSO_H_
#define _POP_MASTER_ESPRESSO_H_

#include <cyplotview.h>
#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopMasterEspresso:public PopBase{
public:

public:
    PopMasterEspresso(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,popModeCb enterCb,popModeCb cencelCb);
    ~PopMasterEspresso();

private:
    void initPopView()override;
    void initPopData()override;

    void setStepData();         // 设置   步骤数据
    void initPlotViewData();    // 初始化 折线图
    void setPlotViewData();     // 设置   折线图 数据
    void onBtnClickListener(View&v);
    void onMasEspStepBtnClickListener(View&v);
    void onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue);

    void setWarnText();      // 设置警告文本
protected:

    ViewGroup       *mPopGaussBox;  // 高斯模糊背景 

    NumberPicker    *mPopPicker;

    CYPlotView        *mMasEspPopPlotView;   // 测试浓缩 plot折线图
    PlotObject      *mPlotObj;
    TextView        *mMasEspPopStep1;
    TextView        *mMasEspPopStep2;
    TextView        *mMasEspPopStep3;
    TextView        *mMasEspPopStep4;
    TextView        *mMasEspPopStep5;

    ImageView       *mMasEspPopStepAdd;
    ImageView       *mMasEspPopStepDelete;
    ImageView       *mMasEspPopResetAnim;
    ValueAnimator   *mResetAnimator;

    NumberPicker    *mMasEspPickerPressure;
    NumberPicker    *mMasEspPickerWater;

    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;
protected:
    int mPickerSelectPos;
    int mStepSelectPos;
    TextView        *mStepSelectView;

    MasEspDataStr    mMasEspData;

    Runnable         mSetPlotDataRun;
};

#endif
