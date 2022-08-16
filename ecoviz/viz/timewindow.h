// timewindow.h: timeline GUI controls
// author: James Gain
// date: June 2021

#ifndef TIMEWINDOW_H
#define TIMEWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

#include "scene.h"

class TimeWindow : public QWidget
{
    Q_OBJECT

private:
    Scene * scene;
    bool playing;
    bool viewlock;

    QTimer * ptimer;
    QSlider *tstep_slider;
    QLabel *value_label;
    QPushButton * back_button, * advance_button, * play_button;
    QIcon * playIcon, * pauseIcon;

    /**
     * @brief setSliderBounds Adjust limits on timeline slider
     * @param tstart    start time
     * @param tend      end time
     */
    void setSliderBounds(int tstart, int tend);

    /**
     * @brief updateSingleScene Match the displayed plants to the current timestep, without signalling a lock with other timelines
     * @param t current timestep on slider
     */
    void updateSingleScene(int t);

signals:
    void signalRepaintAllGL();
    void signalRebindPlants();
    void doCohortMapsAdjustments(int);
    void setTimestepAndSample(int);
    void signalSync(int);

public slots:
    /**
     * @brief updateScene Match the displayed plants to the current timestep
     * @param t current timestep on slider
     */
    void updateScene(int t);

    /**
     * @brief synchronize   Directly set the slider value. Used by timeline locking functionality.
     * @param t timestep value for setting slider
     */
    void synchronize(int t);

    /**
     * @brief advance Advance the slider by a single timestep if possible
     */
    void advance();

    /**
     * @brief back Backtrack the slider by a single timestep if possible
     */
    void backtrack();

    /**
     * @brief playControl Either play or pause the timeline depending on the current play button state
     */
    void playControl();

public:
    TimeWindow(QWidget *parent, int step_start, int step_end, int width, int height);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /**
     * @brief set_labelvalue Adjust numerical indication of timeline
     * @param value current slider time position
     * @param range max slider position
     */
    void set_labelvalue(int value, int range);

    int get_sliderval();
    void set_sliderval(int v);

    /**
     * @brief setScene Setup the timeline to match the scene
     * @param s Scene to match
     */
    void setScene(Scene * s);

    // getters and setters
    Scene * getScene(){ return scene; }

    /// toggle lock flag
    void setViewLockState(bool state){ viewlock = state; }
};

#endif // TIMEWINDOW_H
