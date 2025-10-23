/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:36:56
 * @FilePath: /t5_ai_coffee/src/viewlibs/plotaxis.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/


/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Andreas Nicolai <Andreas.Nicolai@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KPLOTAXIS_H__
#define __KPLOTAXIS_H__

#include <list>
#include <string>
namespace cdroid{
/**
 * @short Axis for KPlotWidget
 *
 * Contains all data for drawing an axis including format specification axis labels.
 *
 * @author Andreas Nicolai
 * @version 1.0
 */
class PlotAxis{
public:
    /**
     * Constructor, constructs an axis with the label @p label.
     */
    explicit PlotAxis(const std::string &label = std::string());

    /**
     * Destructor.
     */
    ~PlotAxis();

    /**
     * @return whether the axis is visible or not
     */
    bool isVisible() const;

    /**
     * Sets the "visible" property of the axis.
     * @param visible if true, this axis will be drawn on the KPlotWidget
     */
    void setVisible(bool visible);

    bool isTickmarkVisible()const;
    void setTickmarkVisible(bool);
    /**
     * @return whether tick labels will be drawn for this axis
     */
    bool areTickLabelsShown() const;

    /**
     * Determine whether tick labels will be drawn for this axis.
     * @param b if true, tick labels will be drawn.
     */
    void setTickLabelsShown(bool b);
    float fontSize()const;
    void setFontSize(float);
    /**
     * Sets the axis label.
     * Set the label to an empty string to omit the axis label.
     * @param label a string describing the data plotted on the axis.
     */
    void setLabel(const std::string &label);

    /**
     * @return the label string for this axis
     */
    std::string label() const;

    /**
     * @return the ticklabel string for the given value, rendered according
     * to the current format specification.
     * @param the value to be rendered as a tick label.
     * @sa setTickLabelFormat()
     */
    std::string tickLabel(double value) const;

    /**
     * Set the display format for converting the double value of the
     * tick's position to the std::string for the tick label.
     *
     * Normally, the format character is one of 'e', 'E', 'f', 'g', or 'G'
     * (see the documentation for std::string::arg(double) for details).
     *
     * In addition, it is possible to set the format character to 't';
     * in this case the tickmark value is interpreted as a time in hours,
     * and the ticklabel string will be in "hh:mm" clock format.
     * Note that when the format character is 't', the fieldWidth and prec
     * values are ignored.
     *
     * @param format the format specification character
     * @param fieldWidth the number of characters in the output string.
     * If set to 0, the string will be as wide as it needs to be to fully
     * render the value.
     * @param precision the number of characters following the decimal point.
     */
    void setTickLabelFormat(char format = 'g', int fieldWidth = 0, int precision = -1);

    /**
     * @return the field width of the tick labels
     */
    int tickLabelWidth() const;

    /**
     * @return the number format of the tick labels
     */
    char tickLabelFormat() const;

    /**
     * @return the number precision of the tick labels
     */
    int tickLabelPrecision() const;

    /**
     * Determine the positions of major and minor tickmarks for this axis.
     * @note this function is called by KPlotWidget whenever the plot's
     * limits are modified.
     * @param x0 the minimum data coordinate of the axis.
     * @param length the range covered by the axis, in data units.
     * @sa majorTickMarks()
     * @sa minorTickMarks()
     */
    void setTickMarks(double x0, double length);

    /**
     * @return the list of coordinates of the major tickmarks for this axis
     * @note the positions of tickmarks are automatically computed by setTickMarks().
     * @sa setTickMarks()
     * @sa minorTickMarks()
     */
    std::list<double> majorTickMarks() const;

    /**
     * @return the list with the minor tickmarks
     * @note the positions of tickmarks are automatically computed by setTickMarks().
     * @sa setTickMarks()
     * @sa majorTickMarks()
     */
    std::list<double> minorTickMarks() const;

private:
    class Private;
    Private *const d;
};
}//endof namespace
#endif // __KPLOTAXIS_H__
