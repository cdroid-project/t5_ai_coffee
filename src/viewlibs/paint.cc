/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:35:11
 * @FilePath: /t5_ai_coffee/src/viewlibs/paint.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "paint.h"
#include <cdlog.h>

Paint::Paint(cdroid::Canvas &ctx) : mCanvas(&ctx)
{
    initData();
}

Paint::Paint()
{
    initData();
}

void Paint::initData()
{
    mAntiAlias   = true;
    mColor       = cdroid::Color::WHITE;
    mStrokeWidth = 1;
    mStyle       = STROKE;
    mStrokeCap   = Cairo::Context::LineCap::ROUND;
    mTextSize    = 15;
    mTypeface    = cdroid::Typeface::create("Droid Sans Fallback", cdroid::Typeface::NORMAL);
}

Paint::~Paint()
{
}

void Paint::setStyle(int flags)
{
    mStyle = flags;
}

void Paint::setAntiAlias(bool val)
{
    mAntiAlias = val;
}

void Paint::setColor(UINT val)
{
    mColor = val;
}

void Paint::setFillColor(UINT val)
{
    mFillColor = val;
}

void Paint::setStrokeWidth(int val)
{
    mStrokeWidth = val;
}

void Paint::setStrokeCap(Cairo::Context::LineCap strokeCap)
{
    mStrokeCap = strokeCap;
}

void Paint::setTextSize(double val)
{
    mTextSize = val;
}

void Paint::setTypeface(cdroid::Typeface *typeface) {
    if (typeface){
        mTypeface = typeface;
    }
}

void Paint::getTextSpace(const std::string &txt, double *w, double *h) {
    if (!w && !h) return;

    // ����һ�� Cairo ͼ��
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);

    // ����һ�� Cairo ��ͼ������
    cairo_t *cr = cairo_create(surface);

    // ���������С����ʽ
    cairo_select_font_face(cr, mTypeface->getFamily().c_str()
        , mTypeface->isItalic() ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL
        , mTypeface->isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size(cr, mTextSize);

    // �ı�����
    const char *text = txt.c_str();

    // �����ı��Ŀ���
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);

    if (w) *w = extents.width + extents.x_bearing;
    if (h) *h = extents.height;

    // ���ٻ�ͼ�����ĺ�ͼ��
    cairo_destroy(cr);
    cairo_surface_destroy(surface);    
}

void Paint::drawText(int x, int y, int w, int h, const std::string &txt
                     , int alignment/* = 0*/
                     , cdroid::Canvas *ctx/* = nullptr*/)
{
    if (ctx == nullptr)
        ctx = mCanvas;

    ctx->begin_new_path();    
    
    if (mAntiAlias)
    {
        ctx->set_antialias(Cairo::Antialias::ANTIALIAS_DEFAULT);
    }

    ctx->set_color(mColor);
    ctx->set_font_size(mTextSize);
    ctx->draw_text(cdroid::RECT::Make(x, y, w, h), txt, alignment);

    LOG(VERBOSE) << "x=" << x << " y=" << y << " w=" << w << " h=" << h 
        << " txt=[" << txt << "] alignment=" << alignment << " color=" << std::hex << mColor 
        << " size=" << std::dec << mTextSize;
}

void Paint::drawArc(int x, int y, int radius, int sAngle, int eAngle
                    , cdroid::Canvas *ctx/* = nullptr*/)
{
    if (ctx == nullptr)
        ctx = mCanvas;

    ctx->begin_new_path();

    if (mAntiAlias)
    {
        ctx->set_antialias(Cairo::Antialias::ANTIALIAS_DEFAULT);
    }

    ctx->set_line_cap(mStrokeCap);
    ctx->set_line_width(mStrokeWidth);
    ctx->set_color(mColor);    
    ctx->arc(x, y, radius, sAngle / 180.0 * M_PI, eAngle / 180.0 * M_PI);    

    if (mStyle & STROKE)
    {
        ctx->stroke_preserve();
    }

    if (mStyle & FILL)
    {   
        ctx->set_color(mFillColor);
        ctx->fill();
    }
}
