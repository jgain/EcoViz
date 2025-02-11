#include "timewindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIntValidator>
#include <QRunnable>
#include <QThreadPool>
#include <QMessageBox>

#include "glwidget.h"
#include "scene.h"
#include "window.h"

TimeWindow::TimeWindow(QWidget *parent, Window * wp, int step_start, int step_end, int width, int height)
    : QWidget(parent, Qt::Window)
{
    setParent(wp);
    this->resize(width, height);

    // media controls
    playing = false; // not currently animating timeline
    viewlock = false; // side-by-side timebars are not initially synchronised
    ptimer = new QTimer(this);
    back_button = new QPushButton("", this);
    play_button = new QPushButton("", this);
    advance_button = new QPushButton("", this);

    QPixmap playmap(":/resources/icons/playicon32.png");
    playIcon = new QIcon(playmap);

    QPixmap pausemap(":/resources/icons/pauseicon32.png");
    pauseIcon = new QIcon(pausemap);

    play_button->setIcon((* playIcon));
    play_button->setIconSize(QSize(20, 20));

    QPixmap advancemap(":/resources/icons/advanceicon32.png");
    QIcon advanceIcon(advancemap);
    advance_button->setIcon(advanceIcon);
    advance_button->setIconSize(QSize(18, 18));

    QPixmap backmap(":/resources/icons/backicon32.png");
    QIcon backIcon(backmap);
    back_button->setIcon(backIcon);
    back_button->setIconSize(QSize(18, 18));

    std::string tvalstr = std::to_string(step_start)+std::string("/")+std::to_string(step_end);
    value_label = new QLabel(tvalstr.c_str(), this);
    value_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    value_label->setFixedWidth(60);

    tstep_slider = new QSlider(Qt::Horizontal, this);
    setSliderBounds(step_start, step_end); // relies on value_label being created

    // QVBoxLayout *vbox = new QVBoxLayout;

    // vbox->addStretch();

    QHBoxLayout *hbox = new QHBoxLayout;
    // hbox->setSpacing(1);
    // hbox->setMargin(1);
    hbox->setContentsMargins(1, 1, 1, 1);
    hbox->addWidget(back_button);
    hbox->addWidget(play_button);
    hbox->addWidget(advance_button);
    hbox->addWidget(tstep_slider);
    hbox->addWidget(value_label);
    // vbox->addLayout(hbox);
    // vbox->addStretch();

    setLayout(hbox);

    // connect(tstep_slider, SIGNAL(sliderMoved(int)), this, SLOT(updateScene(int)));
    connect(tstep_slider, SIGNAL(valueChanged(int)), this, SLOT(updateScene(int)));
    connect(advance_button, SIGNAL(clicked()), this, SLOT(advance()));
    connect(back_button, SIGNAL(clicked()), this, SLOT(backtrack()));
    connect(ptimer, SIGNAL(timeout()), this, SLOT(advance()));
    connect(play_button, SIGNAL(clicked()), this, SLOT(playControl()));
}

QSize TimeWindow::minimumSizeHint() const
{
    return QSize(80, 2);
}

QSize TimeWindow::sizeHint() const
{
    return QSize(800, 25);
}


void TimeWindow::set_labelvalue(int value, int range)
{
    std::string tvalstr = std::to_string(value)+std::string("/")+std::to_string(range);
    value_label->setText(tvalstr.c_str());
}

int TimeWindow::get_sliderval()
{
    return tstep_slider->value();
}

void TimeWindow::set_sliderval(int v)
{
    tstep_slider->setValue(v);
}

void TimeWindow::synchronize(int t)
{
    set_sliderval(t);
    updateSingleScene(t);
}

void TimeWindow::advance()
{
    int t = get_sliderval();
    if(t < scene->getTimeline()->getTimeEnd())
    {
        t+= 1;
        set_sliderval(t);
        updateScene(t);
    }
    else { // pause once end of timeline reached
        if(playing)
            playControl();
    }
}

void TimeWindow::backtrack()
{
    int t = get_sliderval();
    if(t > scene->getTimeline()->getTimeStart())
    {
        t-= 1;
        set_sliderval(t);
        updateScene(t);
    }
    else { // pause once end of timeline reached
        if(playing)
            playControl();
    }
}

void TimeWindow::playControl()
{
    if(playing)
    {
        ptimer->stop(); // stop timer from updating
        play_button->setIcon((* playIcon));
        playing = false;
    }
    else
    {
        ptimer->start(10); // start timer, on every timeout the timeline is advanced
        play_button->setIcon((* pauseIcon));
        playing = true;
    }
}

void TimeWindow::setSliderBounds(int tstart, int tend)
{
    tstep_slider->setMinimum(tstart);
    tstep_slider->setMaximum(tend);
    int nticks = tend - tstart;
    while (nticks > 20)
    {
        nticks /= 2;
    }
    tstep_slider->setTickInterval((tend - tstart) / nticks);
    tstep_slider->setTickPosition(QSlider::TicksBelow);
    set_labelvalue(get_sliderval(), tend);
}

void TimeWindow::setScene(Scene * s)
{
    int tstart, tend;
    scene = s;
    scene->getTimeline()->getTimeBounds(tstart, tend);
    setSliderBounds(tstart, tend);

    //PCM: no idea what this does? Seems to be a map tied to resolution of the terrain? May cause issues.
  /*  if(scene->getTypeMap(TypeMapType::COHORT)->getNumSamples() == -1)
    {
        cerr << "type map error" << endl;
        QMessageBox(QMessageBox::Warning, "Typemap Error", "Type map for cohorts does not have a valid colour table").exec();
    }
    else*/
    if (scene->cohortmaps->get_nmaps() > 0)
    {
        int gw, gh;
        float rw, rh;

        //PCM: changed to get params of master/high res terrain from which this is extracted
        scene->getMasterTerrain()->getGridDim(gw, gh);
        scene->getMasterTerrain()->getTerrainDim(rw, rh);

        // auto amap = scene->cohortmaps->get_actionmap_floats(gw, gh, rw, rh);
        updateScene(scene->getTimeline()->getNow());
    }
    else
    {
        cerr << "no cohort plant counts" << endl;
        QMessageBox(QMessageBox::Warning, "Typemap Error", "No cohort plant count maps available").exec();
    }
    //   tstep_scrollwindow->set_labelvalue(tstep);
}

void TimeWindow::updateScene(int t)
{
    updateSingleScene(t);
    if(viewlock)
    {
        cerr << "signalSync " << t << endl;
        signalSync(t);
    }
}

void TimeWindow::updateSingleScene(int t)
{

     // auto bt_master = std::chrono::steady_clock::now().time_since_epoch();
     set_labelvalue(t, scene->getTimeline()->getTimeEnd());
     scene->getTimeline()->setNow(t);
     int curr_cohortmap = scene->getTimeline()->getCurrentIdx();
     if (curr_cohortmap >= scene->cohortmaps->get_nmaps())
         curr_cohortmap = scene->cohortmaps->get_nmaps() - 1;

     // auto bt_sample = std::chrono::steady_clock::now().time_since_epoch();
     std::vector<basic_tree> trees(scene->sampler->sample(scene->cohortmaps->get_map(curr_cohortmap), nullptr));
     // auto et_sample = std::chrono::steady_clock::now().time_since_epoch();
     std::vector<basic_tree> mature = scene->cohortmaps->get_maturetrees(curr_cohortmap);

     int out_of_bounds = 0;
     float in_x_min = 10000000000, in_x_max = -10000000000, in_y_min = 100000000000, in_y_max = -100000000000;
     for(auto &tree: mature)
     {
         // PCM: changed to use Master terrain - we will place all then cull away (to avoid issues with Timeline)
         // PCM: why are x/y swapped?
         if(scene->getMasterTerrain()->inGridBounds(tree.y, tree.x)) {
             trees.push_back(tree);
             in_x_min = std::min(in_x_min, tree.x); in_y_min = std::min(in_y_min, tree.y);
             in_x_max = std::max(in_x_max, tree.x); in_y_max = std::max(in_y_max, tree.y);
         } else {
             //cerr << "tree out of bounds at (" << tree.x << ", " << tree.y << ")" << endl;
             ++out_of_bounds;
         }
     }

     if (out_of_bounds>0)
         cerr << out_of_bounds << " mature trees out of bound!" << endl <<
             "in bounds rectangle: x min: " << in_x_min << ", x max: " << in_x_max << ", y min: " << in_y_min << ", y max: " << in_y_max;


     // auto bt_render = std::chrono::steady_clock::now().time_since_epoch();
     scene->getEcoSys()->clear();
     scene->getEcoSys()->placeManyPlants(scene->getMasterTerrain(), scene->getNoiseField(), scene->cohortmaps, trees);
     signalRebindPlants();
     winparent->rendercount++;
     signalRepaintAllGL();
     // update(); // JG should not be needed because of RepaintAllGL immediately above
     // auto et_render = std::chrono::steady_clock::now().time_since_epoch();

     // std::cout << "Timestep changed to " << tstep << std::endl;
     // auto et_master = std::chrono::steady_clock::now().time_since_epoch();

     // int sampletime = std::chrono::duration_cast<std::chrono::milliseconds>(et_sample - bt_sample).count();
     // int rendertime = std::chrono::duration_cast<std::chrono::milliseconds>(et_render - bt_render).count();
     // int overalltime = std::chrono::duration_cast<std::chrono::milliseconds>(et_master - bt_master).count();

     // std::cout << "Number of plants: " << trees.size() << std::endl;
     // std::cout << "Overall time: " << overalltime << std::endl;
     // std::cout << "Sample time: " << sampletime << std::endl;
     // std::cout << "Render time: " << rendertime << std::endl;
 }
