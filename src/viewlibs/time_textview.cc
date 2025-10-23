/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:33:03
 * @FilePath: /t5_ai_coffee/src/viewlibs/time_textview.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "time_textview.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_WIDGET(TimeTextView)
TimeTextView::TimeTextView(Context *ctx, const AttributeSet &attrs) : TextView(ctx, attrs) {
    initViewData();

    mAutoStart  = attrs.getBoolean("autoStart", true);
    mTickerTime = attrs.getInt("tickerTime", mTickerTime);
    mMidWidth   = attrs.getDimensionPixelSize("midWidth", mMidWidth);
}

TimeTextView::TimeTextView(int w, int h) : TimeTextView(std::string(), w, h) {
    initViewData();
}

TimeTextView::TimeTextView(const std::string &txt, int w, int h) : TextView(txt, w, h) {
    initViewData();
}

void TimeTextView::initViewData() {
    mTickerTime= 1000;
    mAutoStart = true;
    mIsStart   = false;
    mIsDot     = true;
    mIsUpdate  = true;
    mTicker    = std::bind(&TimeTextView::onTicker, this);
    mMidWidth  = 0;
}

void TimeTextView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    TextView::onMeasure(widthMeasureSpec, heightMeasureSpec);

    LOG(VERBOSE) << "w=" << getMeasuredWidth() << " h=" << getMeasuredHeight();

    if (mCharWidthHeight.empty()) { calcCharWidthHeight(); }

    LayoutParams *lp = getLayoutParams();
    int           max_width, max_height;

    getConnMax(max_width, max_height);

    if ((lp->width == LayoutParams::WRAP_CONTENT && getMeasuredWidth() < max_width) ||
        (lp->height == LayoutParams::WRAP_CONTENT && getMeasuredHeight() < max_height)) {

        if (lp->width != LayoutParams::WRAP_CONTENT) { max_width = getMeasuredWidth(); }
        if (lp->height != LayoutParams::WRAP_CONTENT) { max_height = getMeasuredHeight(); }

        LOG(VERBOSE) << "mw=" << max_width << " mh=" << max_height;
        setMeasuredDimension(max_width, max_height);
    }

    if (mAutoStart && !mIsStart) {
        View::postDelayed([this](){start();}, 10);
    }
}

void TimeTextView::onDraw(Canvas &cans) {
    Typeface   *tf  = mLayout->getTypeface();
    cairo_t    *cr  = cans.cobj();
    std::string txt = mLayout->getString();

    if (txt.empty()) return;

    uchar       is_hour;
    std::string txt_hour;
    std::string txt_min;

    is_hour = true;
    for (size_t n = 0; n < txt.size(); n++) {
        if (txt[n] == ':') {
            is_hour = false;
            continue;
        }
        if (is_hour) {
            txt_hour.push_back(txt[n]);
        } else {
            txt_min.push_back(txt[n]);
        }
    }

    int    x, y, w, h;
    int    mw, mh;
    double x0, y0;

    x = getPaddingLeft();
    y = getPaddingTop();
    w = getWidth() - getPaddingLeft() - getPaddingRight();
    h = getHeight() - getPaddingTop() - getPaddingBottom();

    getConnMax(mw, mh);

    int gravity = getGravity();
    if (w > mw) {
        if (gravity & Gravity::CENTER_HORIZONTAL) {
            x += (w - mw) / 2;
            w -= (w - mw);
        }
    }
    if (h > mh) {
        if (gravity & Gravity::CENTER_VERTICAL) {
            y += (h - mh) / 2;
            h -= (h - mh);
        }
    }

    LOGV("x=%d y=%d w=%d h=%d mw=%d mh=%d", x, y, w, h, mw, mh);

    char  draw_txt[2] = {0};
    int   x_offset    = 0;
    Point pt;

    // 设置字体
    if (tf) { cairo_set_font_face(cr, const_cast<cairo_font_face_t *>(tf->getFontFace()->get_font_face()->cobj())); }

    cairo_set_font_size(cr, getTextSize());

    int   txt_color = getCurrentTextColor();
    Color c(txt_color);
    cairo_set_source_rgba(cr, c.red(), c.green(), c.blue(), c.alpha());

    // 小时
    for (size_t n = 0; n < txt_hour.size(); n++) {
        draw_txt[0] = txt_hour[n];
        getCharWH(draw_txt[0], pt);

        x0 = x + x_offset + (mNumWidth - pt.x) / 2.0;
        if (draw_txt[0] == '-') {
            y0 = y + mh - pt.y / 2.0;
        } else {
            y0 = y + mh - (mh - pt.y) / 2.0;
        }
        cairo_move_to(cr, x0, y0);
        cairo_show_text(cr, draw_txt);
        LOGV("ch=[%s] x=%.2f y=%.2f", draw_txt, x0, y0);

        x_offset += mNumWidth;
    }

    // 符号
    int midWidth = mMidWidth > mCharWidth ? mMidWidth : mCharWidth;
    draw_txt[0] = mIsDot ? ':' : ' ';
    if (draw_txt[0] == ':') {
        getCharWH(draw_txt[0], pt);
        x0 = x + x_offset + (midWidth - pt.x) / 2.0;
        y0 = y + mh - (mh - pt.y) / 2.0;
        cairo_move_to(cr, x0, y0);
        cairo_show_text(cr, draw_txt);
    }
    x_offset += midWidth;

    // 分钟
    for (size_t n = 0; n < txt_min.size(); n++) {
        draw_txt[0] = txt_min[n];
        getCharWH(draw_txt[0], pt);

        x0 = x + x_offset + (mNumWidth - pt.x) / 2.0;
        if (draw_txt[0] == '-') {
            y0 = y + mh - pt.y / 2.0;
        } else {
            y0 = y + mh - (mh - pt.y) / 2.0;
        }
        cairo_move_to(cr, x0, y0);
        cairo_show_text(cr, draw_txt);
        LOGV("ch=[%s] x=%.2f y=%.2f", draw_txt, x0, y0);

        x_offset += mNumWidth;
    }
}

void TimeTextView::start() {
    if (mIsStart) return;
    mIsStart = true;
    mIsDot   = false;
    mIsUpdate= true;
    invalidate();
    View::postDelayed(mTicker, mTickerTime);
}

void TimeTextView::stop() {
    if (!mIsStart) return;
    mIsStart = false;
    mIsDot   = true;
    mIsUpdate= false;
    invalidate();
    View::removeCallbacks(mTicker);
}

void TimeTextView::calcCharWidthHeight() {
    char      cr_txt[2] = {0};
    uchar     chars[]   = "-0123456789: ";
    Typeface *tf        = mLayout->getTypeface();
    int       w, h;
    Point     pt;

    if (!mCharWidthHeight.empty()) return;

    // 创建一个 Cairo 图面
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);

    // 创建一个 Cairo 绘图上下文
    cairo_t *cr = cairo_create(surface);

    // 设置字体大小和样式
    cairo_set_font_face(cr, const_cast<cairo_font_face_t *>(tf->getFontFace()->get_font_face()->cobj()));

    cairo_set_font_size(cr, getTextSize());

    for (size_t n = 0, m = sizeof(chars) - 1; n < m; n++) {
        cr_txt[0] = chars[n];

        // 计算文本的宽度
        cairo_text_extents_t extents;
        cairo_text_extents(cr, cr_txt, &extents);

        pt.x = ceil(extents.width + extents.x_bearing);
        pt.y = ceil(extents.height);
        LOGV("ch=[%s] w=%d h=%d", cr_txt, pt.x, pt.y);

        mCharWidthHeight[chars[n]] = pt;
    }

    // 销毁绘图上下文和图面
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    mNumWidth   = 0;
    mNumHeight  = 0;
    mCharWidth  = 0;
    mCharHeight = 0;
}

void TimeTextView::getConnMax(int &w, int &h) {
    if (mNumWidth == 0) {
        for (auto kv : mCharWidthHeight) {
            if (kv.first == '-' || (kv.first >= '0' && kv.first <= '9')) {
                if (kv.second.x > mNumWidth) { mNumWidth = kv.second.x; }
                if (kv.second.y > mNumHeight) { mNumHeight = kv.second.y; }
            } else {
                if (kv.second.x > mCharWidth) { mCharWidth = kv.second.x; }
                if (kv.second.y > mCharHeight) { mCharHeight = kv.second.y; }
            }
        }
    }

    if (mMidWidth > mCharWidth) {
        w = mNumWidth * 4 + mMidWidth;
    } else {
        w = mNumWidth * 4 + mCharWidth;
    }
    h = std::max(mNumHeight, mCharHeight);
}

void TimeTextView::getCharWH(uchar ch, Point &pt) {
    auto it = mCharWidthHeight.find(ch);
    if (it == mCharWidthHeight.end()) {
        pt.x = 0;
        pt.y = 0;
        return;
    }
    pt.x = it->second.x;
    pt.y = it->second.y;
}

void TimeTextView::onTicker() {
    View::postDelayed(mTicker, mTickerTime);
    mIsDot = !mIsDot;
    if(mIsUpdate) invalidate();
    // postInvalidateOnAnimation();
}

void TimeTextView::setIsUpdate(bool flag){
    mIsUpdate = flag;
}