/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <plotpoint.h>
#include <cyplotobject.h>
#include <core/rect.h>
#include <algorithm>

namespace cdroid{

class PlotPoint::Private
{
public:
    Private(PlotPoint *qq, const PointF &p, const std::string &l, double bw)
        : q(qq)     , point(p) , plot(nullptr)
        , label(l)  , barWidth(bw)
    {
    }
    
    PlotPoint *q;
    CYPlotObject*plot;
    PointF point;
    std::string label;
    double barWidth;
};

PlotPoint::PlotPoint()
    : d(new Private(this, PointF(), std::string(), 0.0))
{
}

PlotPoint::PlotPoint(double x, double y, const std::string &label, double barWidth)
    : d(new Private(this, PointF{(float)x, (float)y}, label, barWidth))
{
}

PlotPoint::PlotPoint(const PointF &p, const std::string &label, double barWidth)
    : d(new Private(this, p, label, barWidth))
{
}

void PlotPoint::setPlot(CYPlotObject*p){
    d->plot = p;
}

PlotPoint::~PlotPoint()
{
    delete d;
}

PointF PlotPoint::position() const
{
    return d->point;
}

void PlotPoint::setPosition(const PointF &pos)
{
    d->point = pos;
}

double PlotPoint::x() const
{
    return d->point.x;
}

void PlotPoint::setX(double x)
{
    d->point.x= static_cast<float>(x);
}

double PlotPoint::y() const
{
    return d->point.y;
}

void PlotPoint::setY(double y)
{
    d->point.y=static_cast<float>(y);
}

std::string PlotPoint::label() const
{
    return d->label;
}

void PlotPoint::setLabel(const std::string &label)
{
    d->label = label;
}

double PlotPoint::barWidth() const
{
    return d->barWidth;
}

void PlotPoint::setBarWidth(double w)
{
    d->barWidth = w;
}
}//endof namespace
