#include "scrollwindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

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

    QLabel *scroll_label = new QLabel("Time step", this);
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
    vbox->addStretch();

    setLayout(vbox);

    GLWidget *p = dynamic_cast<GLWidget *>(parent);

    if (p)
    {
        connect(tstep_slider, SIGNAL(sliderMoved(int)), p, SLOT(set_timestep(int)));
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
