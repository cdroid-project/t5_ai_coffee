/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:38:32
 * @FilePath: /t5_ai_coffee/src/viewlibs/cyplotobject.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <cairomm/pattern.h>
#include <cyplotobject.h>
#include <plotpoint.h>
#include <cyplotview.h>
#include <cdtypes.h>
#include <cdlog.h>

namespace cdroid{

class CYPlotObject::Private
{
public:
    Private(CYPlotObject *qq)
        :q(qq),size(0),thickness(0){
        type = UnknownType;
        pointStyle = NoPoints;
    }

    ~Private(){
        for(auto i:pList)delete i;
    }

    CYPlotObject *q;

    std::list<PlotPoint *> pList;
    int type;
    PointStyle pointStyle;
    double size;
    PointF mCenter;
    double thickness;
    RefPtr<Pattern> pen, linePen, barPen, labelPen;
    RefPtr<Pattern> brush, barBrush;
};

CYPlotObject::CYPlotObject(uint32_t color, PlotType t, double size, PointStyle ps)
    : d(new Private(this)){
    // By default, all pens and brushes are set to the given color
    mLineWidth = 2.f;
    mDashWidth = 0.f;
    mDashGap = 0.f;
    Color clr(color);
    RefPtr<Pattern> brush = Cairo::SolidPattern::create_rgba(clr.red(),clr.green(),clr.blue(),clr.alpha());
    setBrush(brush);
    setBarBrush(brush);
    setPen(brush);
    setLinePen(brush);
    setBarPen(brush);
    setLabelPen(brush);

    d->type |= t;
    setSize(size);
    setPointStyle(ps);
}

CYPlotObject::CYPlotObject(uint32_t color,const PointF& center, double radius,double thicknees)
    :d(new Private(this)){
    mLineWidth = 2.f;
    mDashWidth = 0.f;
    mDashGap = 0.f;
    Color clr(color);
    RefPtr<Pattern> brush = Cairo::SolidPattern::create_rgba(clr.red(),clr.green(),clr.blue(),clr.alpha());
    setBrush(brush);
    setBarBrush(brush);
    setPen(brush);
    setLinePen(brush);
    setBarPen(brush);
    setLabelPen(brush);
    d->type |= Pie;
    setSize(radius);
    d->mCenter = center;
    d->thickness = thicknees;
}

CYPlotObject::~CYPlotObject(){
    delete d;
}

CYPlotObject::PlotTypes CYPlotObject::plotTypes() const{
    return (CYPlotObject::PlotTypes)d->type;
}

void CYPlotObject::setShowPoints(bool b){
    if (b) {
        d->type |= CYPlotObject::Points;
    } else {
        d->type &= ~CYPlotObject::Points;
    }
}

void CYPlotObject::setShowLines(bool b){
    if (b) {
        d->type |= CYPlotObject::Lines;
    } else {
        d->type &= ~CYPlotObject::Lines;
    }
}

void CYPlotObject::setShowBars(bool b){
    if (b) {
        d->type |= CYPlotObject::Bars;
    } else {
        d->type &= ~CYPlotObject::Bars;
    }
}

double CYPlotObject::size() const{
    return d->size;
}

void CYPlotObject::setSize(double s){
    d->size = s;
}

CYPlotObject::PointStyle CYPlotObject::pointStyle() const{
    return d->pointStyle;
}

void CYPlotObject::setPointStyle(PointStyle p){
    d->pointStyle = p;
}

const RefPtr<Pattern> &CYPlotObject::pen() const{
    return d->pen;
}

void CYPlotObject::setPen(const RefPtr<Pattern> &p){
    d->pen = p;
}

const RefPtr<Pattern> &CYPlotObject::linePen() const{
    return d->linePen;
}

void CYPlotObject::setLinePen(const RefPtr<Pattern> &p){
    d->linePen = p;
}

const RefPtr<Pattern> &CYPlotObject::barPen() const{
    return d->barPen;
}

void CYPlotObject::setBarPen(const RefPtr<Pattern> &p){
    d->barPen = p;
}

const RefPtr<Pattern> &CYPlotObject::labelPen() const{
    return d->labelPen;
}

void CYPlotObject::setLabelPen(const RefPtr<Pattern> &p){
    d->labelPen = p;
}

const RefPtr<Pattern> CYPlotObject::brush() const{
    return d->brush;
}

void CYPlotObject::setBrush(const RefPtr<Pattern> &b){
    d->brush = b;
}

const RefPtr<Pattern> CYPlotObject::barBrush() const{
    return d->barBrush;
}

void CYPlotObject::setBarBrush(const RefPtr<Pattern> &b){
    d->barBrush = b;
}

std::list<PlotPoint *> CYPlotObject::points() const{
    return d->pList;
}

PlotPoint& CYPlotObject::addPoint(const PointF &p, const std::string &label, double barWidth){
    return addPoint(new PlotPoint(p.x, p.y, label, barWidth));
}

PlotPoint& CYPlotObject::addPoint(PlotPoint *p){
    if (!p) {
        throw "PlotPoint cant be nullptr";
    }
    d->pList.push_back(p);
    p->setPlot(this);
    return *p;
}

PlotPoint& CYPlotObject::addPoint(double x, double y, const std::string &label, double barWidth){
    return addPoint(new PlotPoint(x, y, label, barWidth));
}

void CYPlotObject::removePoint(int index){
    if ((index < 0) || (index >= d->pList.size())) {
        LOG(WARN) << "CYPlotObject::removePoint(): index " << index << " out of range!";
        return;
    }
    auto it= d->pList.begin();
    std::advance(it,index);
    d->pList.erase(it);
}

void CYPlotObject::clearPoints(){
    for(auto i:d->pList)delete i;
    d->pList.clear();
}

void CYPlotObject::draw(cdroid::Canvas&painter,CYPlotView*pw){
    // Order of drawing determines z-distance: Bars in the back, then lines,
    // then points, then labels.

    if (d->type & Bars) {
        double w = 0;
        for (int i = 0; i < d->pList.size(); ++i) {
            auto it =d->pList.begin();
            std::advance(it,i);
            if ((*it)->barWidth() == 0.0) {
                if (i < d->pList.size() - 1) {
                    auto next=it;
                    std::advance(next,1);
                    w = (*next)->x() - (*it)->x();
                }
                // For the last bin, we'll just keep the previous width
            } else {
                w = (*it)->barWidth();
            }

            PointF pp = (*it)->position();
            PointF p1={float(pp.x - 0.5f * w), 0.f};
            PointF p2={float(pp.x + 0.5f * w), pp.y};
            PointF sp1 = pw->mapToWidget(p1);
            PointF sp2 = pw->mapToWidget(p2);

            const RectF barRect = RectF::Make(sp1.x, sp1.y, sp2.x - sp1.x, sp2.y - sp1.y);
            painter.rectangle(static_cast<int>(sp1.x), static_cast<int>(sp1.y),
                static_cast<int>(barRect.width), static_cast<int>(barRect.height));
            painter.set_source(barBrush());
            painter.fill_preserve();
            painter.set_source(barPen());
            painter.stroke();
            pw->maskRect(barRect, 0.25);
        }
    }
    if(d->type&Pie){
        double w;
        for (int i = 0; i < d->pList.size(); ++i) {
            auto it =d->pList.begin();
            std::advance(it,i);
            if ((*it)->barWidth() == 0.0) {
                if (i < d->pList.size() - 1) {
                    auto next=it;
                    std::advance(next,1);
                    w = (*next)->x() - (*it)->x();
                }
                // For the last bin, we'll just keep the previous width
            } else {
                w = (*it)->barWidth();
            }
            PointF pp =(*it)->position();
            PointF sp1= pw->mapToWidget(pp);
            PointF pc = pw->mapToWidget(d->mCenter);
            PointF ps = pw->mapToWidget({float(d->size),0});
            RectF dr = pw->dataRect();
            painter.arc(pc.x,pc.y,d->size,pp.x*M_PI/180.f,(pp.x+pp.y)*M_PI/180.f);
            if(d->thickness!=0.f){d->thickness=d->size-20;
                painter.arc_negative(pc.x,pc.y,d->size-d->thickness,(pp.x+pp.y)*M_PI/180.f,(pp.x*M_PI)/180.f);
            }
            painter.set_source(barBrush());
            painter.fill_preserve();
            painter.set_source(barPen());
            painter.stroke();
        }	
    }
    // Draw lines:
    if (d->type & Lines) {
	    bool bPrevious = false;
        painter.set_source(linePen());
        // 需要设置线条宽度（目前4宽度仅适用于ms7）
        painter.set_line_width(mLineWidth);
        if(mDashWidth != 0.f && mDashGap != 0.f){
            std::vector<double>dashs={mDashWidth,mDashGap};
            painter.set_dash(dashs,0);
        }
        PointF Previous; // Initialize to null
        for (const PlotPoint *pp : d->pList) {
            // q is the position of the point in screen pixel coordinates
            PointF q = pw->mapToWidget(pp->position());
            if (bPrevious) {
                //painter->drawLine(Previous, q);
                painter.line_to(q.x,q.y);
                pw->maskAlongLine(Previous, q);
            }else{
                bPrevious = true;
                painter.move_to(q.x,q.y);
            }
            Previous = q;
        }
        painter.stroke();
    }

    // Draw points:
    if (d->type & Points) {
        for (const PlotPoint *pp : d->pList) {
            // q is the position of the point in screen pixel coordinates
            PointF q = pw->mapToWidget(pp->position());
            if (pw->pixRect().contains(static_cast<int>(q.x),static_cast<int>(q.y))) {
                double x1 = q.x - size();
                double y1 = q.y - size();
                Rect rect;
                RectF qr = RectF::Make(static_cast<float>(x1), static_cast<float>(y1),
                    static_cast<float>(2.f * size()), static_cast<float>(2 * size()));
                // Mask out this rect in the plot for label avoidance
                pw->maskRect(qr, 2.0);

                switch (pointStyle()) {
                case Circle:
                    painter.arc(q.x,q.y,size(),0,M_PI*2.f);
                    painter.stroke();
                    break;

                case Letter:
                    //painter->drawText(qr, Qt::AlignCenter, pp->label().left(1));
                    rect = Rect::Make(static_cast<int>(qr.left), static_cast<int>(qr.top),
                        static_cast<int>(2.f * size()), static_cast<int>(2.f * size()));
                    painter.draw_text(rect,pp->label(),cdroid::Gravity::CENTER);
                    break;

                case Triangle:
                    painter.set_source(brush());
                    painter.move_to(q.x - size(), q.y + size());
                    painter.line_to(q.x, q.y - size());
                    painter.line_to(q.x + size(), q.y + size());
                    painter.line_to(q.x - size(), q.y + size());
                    painter.line_to(q.x - size(), q.y + size());//line to 1st point(closepath)
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();
                    break;

                case Square:
                    painter.rectangle(static_cast<int>(qr.left),static_cast<int>(qr.top),
                        static_cast<int>(qr.width), static_cast<int>(qr.height) );
                    painter.set_source(brush());
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();
                    break;

                case Pentagon:
                    painter.move_to(q.x, q.y - size());
                    painter.line_to(q.x + size(), q.y - 0.309 * size());
                    painter.line_to(q.x + 0.588 * size(), q.y + size());
                    painter.line_to(q.x - 0.588 * size(), q.y + size());
                    painter.line_to(q.x - size(), q.y - 0.309 * size());
                    painter.line_to(q.x, q.y - size());//line to 1st point(closepath)

                    painter.set_source(brush());
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();		    
                    break;

                case Hexagon:
                    painter.move_to(q.x, q.y + size());
                    painter.line_to(q.x + size(), q.y + 0.5 * size());
                    painter.line_to(q.x + size(), q.y - 0.5 * size());
                    painter.line_to(q.x, q.y - size());
                    painter.line_to(q.x - size(), q.y + 0.5 * size());
                    painter.line_to(q.x - size(), q.y - 0.5 * size());
                    painter.line_to(q.x, q.y + size());//line to 1st point(closepath)
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();
                    break;

                case Asterisk:
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x, q.y + size());
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x + size(), q.y - 0.5 * size());
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x + size(), q.y - 0.5 * size());
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x, q.y - size());
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x - size(), q.y + 0.5 * size());
                    painter.move_to(q.x,q.y);
                    painter.line_to(q.x - size(), q.y - 0.5 * size());
                    painter.line_to(q.x,q.y);//line to 1st point(closepath)
                    painter.set_source(brush());
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();
                    break;

                case Star:
                    painter.move_to(q.x, q.y - size());
                    painter.line_to(q.x + 0.2245 * size(), q.y - 0.309 * size());
                    painter.line_to(q.x + size(), q.y - 0.309 * size());
                    painter.line_to(q.x + 0.363 * size(), q.y + 0.118 * size());
                    painter.line_to(q.x + 0.588 * size(), q.y + size());
                    painter.line_to(q.x, q.y + 0.382 * size());
                    painter.line_to(q.x - 0.588 * size(), q.y + size());
                    painter.line_to(q.x - 0.363 * size(), q.y + 0.118 * size());
                    painter.line_to(q.x - size(), q.y - 0.309 * size());
                    painter.line_to(q.x - 0.2245 * size(), q.y - 0.309 * size());
                    painter.line_to(q.x, q.y - size());//line to 1st point(closepath)
                    painter.set_source(brush());
                    painter.fill_preserve();
                    painter.set_source(pen());
                    painter.stroke();
                    break;

                default:
                    break;
                }
            }
        }
    }

    // Draw labels
    painter.set_source(labelPen());

    for (PlotPoint *pp : d->pList) {
        const PointF q = pw->mapToWidget(pp->position());
        if (pw->pointInView(static_cast<int>(q.x),static_cast<int>(q.y), 1) && !pp->label().empty()) {
            pw->placeLabel(painter, pp);
        }
    }
}


void CYPlotObject::setLineWidth(double w){
    mLineWidth = w;
}

void CYPlotObject::setDashedLine(double dashWidth, double dashGap){
    mDashWidth = dashWidth;
    mDashGap = dashGap;
}
}//endof namespace
