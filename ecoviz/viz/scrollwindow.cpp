#include "scrollwindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIntValidator>

#include "glwidget.h"

scrollwindow::scrollwindow(QWidget *parent, int step_start, int step_end, int width, int height)
    : QWidget(parent, Qt::Window)
{
    this->resize(width, height);

    tstep_slider = new QSlider(Qt::Horizontal, this);
    tstep_slider->setMinimum(step_start);
    tstep_slider->setMaximum(step_end);
    int nticks = step_end - step_start;
    while (nticks > 20)
    {
        nticks /= 2;
    }
    tstep_slider->setTickInterval((step_end - step_start) / nticks);
    tstep_slider->setTickPosition(QSlider::TicksBelow);

    /*
    smooth_slider = new QSlider(Qt::Horizontal, this);
    smooth_slider->setMinimum(0);
    smooth_slider->setMaximum(10);
    smooth_slider->setTickInterval(1);
    smooth_slider->setTickPosition(QSlider::TicksBelow);
    */

    QIntValidator *intvalidate = new QIntValidator(this);
    smooth_textfield = new QLineEdit("0", this);
    smooth_textfield->setValidator(intvalidate);

    QLabel *scroll_label = new QLabel("Time step", this);
    smooth_label = new QLabel("Smoothing distance", this);
    QLabel *set_label = new QLabel("Current step: ", this);
    value_label = new QLabel(std::to_string(step_start).c_str(), this);
    value_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addStretch();

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(set_label);
    hbox->addWidget(value_label);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addStretch();

    hbox = new QHBoxLayout;
    hbox->addWidget(scroll_label);
    hbox->addWidget(tstep_slider);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addWidget(smooth_label);
    //hbox->addWidget(smooth_slider);
    hbox->addWidget(smooth_textfield);
    vbox->addLayout(hbox);

    vbox->addStretch();

    setLayout(vbox);

    GLWidget *p = dynamic_cast<GLWidget *>(parent);

    if (p)
    {
        connect(tstep_slider, SIGNAL(sliderMoved(int)), p, SLOT(set_timestep(int)));
        //connect(smooth_slider, SIGNAL(sliderMoved(int)), p, SLOT(set_smoothing_distance(int)));
        connect(smooth_textfield, SIGNAL(returnPressed()), p, SLOT(set_smoothing_distance()));
    }
}


void scrollwindow::set_labelvalue(int value)
{
    value_label->setText(std::to_string(value).c_str());
}

int scrollwindow::get_sliderval()
{
    return tstep_slider->value();
}
