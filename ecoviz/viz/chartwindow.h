// chartwindow.h: graphing subwindow
// author: James Gain
// date: June 2021

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
    void setData(TimelineGraph * gdata);
    void setGraphs(std::vector<TimelineGraph*> all_gr) { all_graphs = all_gr; }
};

#endif // TIMEWINDOW_H
