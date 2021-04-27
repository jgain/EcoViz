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
