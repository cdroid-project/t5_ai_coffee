#ifndef _POP_HANDWASH_H_
#define _POP_HANDWASH_H_

#include <widget/textview.h>
#include <widget/numberpicker.h>

#include "page_base.h"
#include "data.h"
#include "pop_base.h"

class PopHandWash:public PopBase{
public:

public:
    PopHandWash(ViewGroup *wind_pop_box,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb,popModeCb cencelCb);
    ~PopHandWash();

private:
    void initPopView()override;
    void initPopData()override;

    void setStepData();// 初始化 手冲咖啡 步骤数据

    void onBtnClickListener(View&v);
    void onStepBtnClickListener(View&v);
    void onPickerValueChangeListener(NumberPicker&picker,int oldValue,int newValue);

protected:

    ViewGroup       *mPopGaussBox;  // 高斯模糊背景 

    NumberPicker    *mPopPicker;

    TextView        *mPopStep1;
    TextView        *mPopStep2;
    TextView        *mPopStep3;
    TextView        *mPopStep4;

    ImageView       *mPopStepAdd;
    ImageView       *mPopStepDelete;
    ImageView       *mPopResetAnim;
    ValueAnimator   *mResetAnimator;

    NumberPicker    *mPopPickerWater;
    NumberPicker    *mPopPickerFlowRate;
    NumberPicker    *mPopPickerTime;

    TextView        *mPopBtnConfirm;
    TextView        *mPopBtnCancel;
protected:
    int mPickerSelectPos;
    int mStepSelectPos;
    TextView        *mStepSelectView;

    HandWashDataStr  mHandWashData;

};

#endif
