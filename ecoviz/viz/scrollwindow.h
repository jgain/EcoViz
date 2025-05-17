/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/

#ifndef SCROLLWINDOW_H
#define SCROLLWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>

class scrollwindow : public QWidget
{
    Q_OBJECT

public:
    scrollwindow(QWidget *parent, int step_start, int step_end, int width, int height);

    QSlider *tstep_slider;
    QLabel *value_label;
    QSlider *smooth_slider;
    QLabel *smooth_label;
    QLineEdit *smooth_textfield;
    void set_labelvalue(int value);
    int get_sliderval();
};

#endif
