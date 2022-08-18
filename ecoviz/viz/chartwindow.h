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

QT_CHARTS_USE_NAMESPACE

class ChartWindow : public QWidget
{
    Q_OBJECT

private:
    Scene * scene;              //< attached underlying scene
    TimelineGraph * graphdata;  //< graph model corresponding to this graph view
    QChart * chart;             //< graphical chart from qt
    enum ChartType { ChartBasalArea, ChartStemNumber, ChartDBHDistribution} ;
    QStringList chart_desc;
    QLabel *chart_help_label;
signals:
    void signalRepaintAllGL();

public slots:

    /**
    * @brief updateTimeBar  Move the vertical timebar rectangle to align with the current time
    */
    void updateTimeBar();

public:

    ChartWindow(QWidget *parent, int width, int height);

    void init();

    void paintEvent(QPaintEvent * ev);

    void setScene(Scene * s){ scene = s; }
    void setData(TimelineGraph * gdata);
};

#endif // TIMEWINDOW_H
