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

#ifndef PROGRESSBAR_WINDOW_H
#define PROGRESSBAR_WINDOW_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>

class progressbar_window : public QWidget
{
    Q_OBJECT

public:
    progressbar_window(int width, int height);
    void update_label(std::string label);

signals:
    void update_label_signal(QString);
    void update_bar_signal(int);

public slots:
    void update_bar(int value);

private:
    QProgressBar *bar;
    QLabel *barlabel;

    int width, height;
};

#endif
