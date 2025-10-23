/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:36:50
 * @FilePath: /t5_ai_demo/src/viewlibs/plotpoint.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KPLOTPOINT_H__
#define __KPLOTPOINT_H__

#include <string>
#include <core/rect.h>
#include <cyplotobject.h>
namespace cdroid{
/**
 * @class PlotPoint
 * @short Encapsulates a point in the plot.
 * A PlotPoint consists of X and Y coordinates (in Data units),
 * an optional label string, and an optional bar-width,
 * The bar-width is only used for plots of type KPlotObject::Bars,
 * and it allows the width of each bar to be set manually.  If
 * bar-widths are omitted, then the widths will be set automatically,
 * based on the halfway-mark between adjacent points.
 */
class PlotPoint{
    friend class CYPlotObject;
public:
    /**
     * Default constructor.
     */
    explicit PlotPoint();
    /**
     * Constructor.  Sets the PlotPoint according to the given arguments
     * @param x the X-position for the point, in Data units
     * @param y the Y-position for the point, in Data units
     * @param label the label string for the point.  If the string
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type KPlotObject::Bars)
     */
    PlotPoint(double x, double y, const std::string &label = std::string(), double width = 0.0);

    /**
     * Constructor.  Sets the PlotPoint according to the given arguments
     * @param p the position for the point, in Data units
     * @param label the label string for the point.  If the string
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type KPlotObject::Bars)
     */
    explicit PlotPoint(const PointF &p, const std::string &label = std::string(), double width = 0.0);
    /**
     * Destructor
     */
    ~PlotPoint();

    /**
     * @return the position of the point, in data units
     */
    PointF position() const;

    /**
     * Set the position of the point, in data units
     * @param pos the new position for the point.
     */
    void setPosition(const PointF &pos);

    /**
     * @return the X-position of the point, in data units
     */
    double x() const;

    /**
     * Set the X-position of the point, in Data units
     */
    void setX(double x);

    /**
     * @return the Y-position of the point, in data units
     */
    double y() const;

    /**
     * Set the Y-position of the point, in Data units
     */
    void setY(double y);

    /**
     * @return the label for the point
     */
    std::string label() const;

    /**
     * Set the label for the point
     */
    void setLabel(const std::string &label);

    /**
     * @return the bar-width for the point
     */
    double barWidth() const;

    /**
     * Set the bar-width for the point
     */
    void setBarWidth(double w);

private:
    class Private;
    Private *const d;
    void setPlot(CYPlotObject*p);
};
}//endof namespace
#endif
