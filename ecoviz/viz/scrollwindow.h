#ifndef SCROLLWINDOW_H
#define SCROLLWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QLabel>

class scrollwindow : public QWidget
{
    Q_OBJECT

public:
    scrollwindow(QWidget *parent, int step_start, int step_end, int width, int height);

    QSlider *tstep_slider;
    QLabel *value_label;
    void set_labelvalue(int value);
};

#endif
