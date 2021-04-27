#include "progressbar_window.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <iostream>

progressbar_window::progressbar_window(int width, int height)
    : QWidget(nullptr, Qt::Window), width(width), height(height)
{
    resize(width, height);

    bar = new QProgressBar(this);
    bar->setMinimum(0);
    bar->setMaximum(100);
    barlabel = new QLabel(this);

    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;

    hbox->addWidget(barlabel);
    hbox->addWidget(bar);

    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(this, SIGNAL(update_bar_signal(int)), bar, SLOT(setValue(int)));
    connect(this, SIGNAL(update_label_signal(QString)), barlabel, SLOT(setText(QString)));
}

void progressbar_window::update_label(std::string label)
{
    //barlabel->setText(label.c_str());
    update_label_signal(QString(label.c_str()));
}

void progressbar_window::update_bar(int value)
{
    //bar->setValue(value);
    update_bar_signal(value);
    std::cout << "Setting value of progress bar to " << value << std::endl;
}


