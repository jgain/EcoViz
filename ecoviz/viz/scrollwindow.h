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
