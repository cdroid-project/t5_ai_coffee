/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:37:02
 * @FilePath: /t5_ai_demo/src/viewlibs/paint.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/


#ifndef __paint_h__
#define __paint_h__

#include <core/canvas.h>
#include <core/typeface.h>

class Paint
{
public:
    typedef enum
    {
        FILL   = 0x01,
        STROKE = 0x02,
        FILL_AND_STROKE = FILL | STROKE,
    } Style;

    Paint(cdroid::Canvas &ctx);
    Paint();
    ~Paint();

    void setStyle(int flags);
    void setAntiAlias(bool val);
    void setColor(UINT val);
    void setFillColor(UINT val);
    void setStrokeWidth(int val);
    void setStrokeCap(Cairo::Context::LineCap strokeCap);
    void setTextSize(double val);
    void setTypeface(cdroid::Typeface *typeface);
    void getTextSpace(const std::string &txt, double *w, double *h);

protected:
    void initData();

public:
    //************************************
    // Method:    drawText
    // FullName:  Paint::drawText
    // Access:    public 
    // Returns:   void
    // Qualifier: д��
    // Parameter: int x ������ʼλ��
    // Parameter: int y ���ֵײ�λ��
    // Parameter: int w ����
    // Parameter: int h �߶�
    // Parameter: const std::string & txt ����
    //************************************
    void drawText(int x, int y, int w, int h, const std::string &txt, int alignment = 0, cdroid::Canvas *ctx = nullptr);
    //************************************
    // Method:    drawArc
    // FullName:  Paint::drawArc
    // Access:    public 
    // Returns:   void
    // Qualifier: ����
    // Parameter: int x Բ��x
    // Parameter: int y Բ��y
    // Parameter: int radius �뾶
    // Parameter: int sAngle ��ʼ�Ƕ� (�����3��λ��) 
    // Parameter: int eAngle ��ֹ�Ƕ�
    //************************************
    void drawArc(int x, int y, int radius, int sAngle, int eAngle, cdroid::Canvas *ctx = nullptr);

private:
    cdroid::Canvas *        mCanvas;        // ����
    bool                    mAntiAlias;     // �����
    UINT                    mColor;         // ��ɫ
    UINT                    mFillColor;     // �����ɫ 
    int                     mStrokeWidth;   // ���ʿ���
    Cairo::Context::LineCap mStrokeCap;     // �˵���ʽ
    int                     mStyle;         // �����ʽ
    double                  mTextSize;      // ���ִ�С
    cdroid::Typeface        *mTypeface;
};

#endif
