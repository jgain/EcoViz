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

// chartwindow.h: graphing subwindow

#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QtCharts/QChart>
#include <QtCharts/QAreaSeries>

#include "scene.h"

class Window;

class ChartWindow : public QWidget
{
    Q_OBJECT

private:
    Scene * scene;              //< attached underlying scene
    Window * winparent;
    std::vector<TimelineGraph*> all_graphs; // collection of graphs
    TimelineGraph * graphdata;  //< current graph model corresponding to this graph view
    QChart * chart;             //< graphical chart from qt
    std::vector<int> xlabels;   //< labelling for timeline
    QStringList chart_desc;
    QLabel *chart_help_label;
signals:
    void signalRepaintAllGL();

public slots:

    /**
    * @brief updateTimeBar  Move the vertical timebar rectangle to align with the current time
    */
    void updateTimeBar();

    void chartSelected(int index);

public:

    ChartWindow(QWidget *parent, int width, int height);

    void setParent(Window * wp){ winparent = wp; }

    void init();

    void paintEvent(QPaintEvent * ev);

    void setScene(Scene * s){ scene = s; }
    void setXLabels(std::vector<int> xaxislabels){ xlabels = xaxislabels; }
    void setData(TimelineGraph * gdata);
    void setGraphs(std::vector<TimelineGraph*> all_gr) { all_graphs = all_gr; }
    int getNumGraphs(){ return (int) all_graphs.size(); }
    string getGraphName(int idx){ return all_graphs[idx]->getTitle(); }
};

#endif // TIMEWINDOW_H
