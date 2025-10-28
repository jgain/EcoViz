#include "chartwindow.h"
#include "glwidget.h"
#include "scene.h"
#include "window.h"

#include<QtCharts/QLineSeries>
#include<QtCharts/QAreaSeries>
#include<QtCharts/QChartView>
#include<QtCharts/QValueAxis>
#include<QtCharts/QCategoryAxis>
#include<QHBoxLayout>
#include<QGraphicsLayout>
#include <QGroupBox>
#include<QStackedBarSeries>
#include<QBarCategoryAxis>
#include<QBarSet>
#include<QComboBox>

ChartWindow::ChartWindow(QWidget *parent, int width, int height)
    : QWidget(parent, Qt::Window)
{
    this->resize(width, height);
    graphdata = nullptr;

    chart = new QChart();
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);

    QChartView * chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    // hbox->setContentsMargins(40, 0, 45, 0);
    hbox->setContentsMargins(19, 0, 45, 0);
    hbox->addWidget(chartView);
    // only works if paintEvent is also sub-classed as below
    setStyleSheet("background:white;");

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);


    /*
    QHBoxLayout *optlayout = new QHBoxLayout;

    QComboBox *vistype = new QComboBox;
    vistype->addItem(tr("Basal area"), TimelineGraph::ChartBasalArea);
    vistype->addItem(tr("Stem number"), TimelineGraph::ChartStemNumber);
    vistype->addItem(tr("DBH Distribution"), TimelineGraph::ChartDBHDistribution);
    chart_desc << "basal area per species (m2/ha)" << "number of trees per ha" << "diameter distribution in 10cm bins";
    connect(vistype, SIGNAL(currentIndexChanged(int)), this, SLOT(chartSelected(int)));

    //connect(vistype, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //    [=](int index){ if (index>=0) chart_help_label->setText(chart_desc[index]); else chart_help_label->setText(""); });
    vistype->setCurrentIndex(0);

    chart_help_label = new QLabel;
    optlayout->addWidget(vistype);
    optlayout->addWidget(chart_help_label);

    vbox->addLayout(optlayout);*/

    vbox->addLayout(hbox, 1);

    setLayout(vbox);
}

void ChartWindow::updateTimeBar()
{
    if(graphdata != nullptr)
    {
        TimelineGraph *data = all_graphs[currentChartType];
        setData(data);
    }
}

void ChartWindow::chartSelected(int index)
{
    if (index<0) {
        // chart_help_label->setText("");
        return;
    }

    // chart_help_label->setText(chart_desc[index]);

    // switch graph
    // chartViews[i]->setData(graphModels[i].front()); // set to first visualization
    currentChartType = (TimelineGraph::ChartType)index;
    TimelineGraph *data = all_graphs[index];
    setData(data);
}

void ChartWindow::paintEvent(QPaintEvent * ev)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChartWindow::setData(TimelineGraph * gdata)
{
    if (gdata->getTitle() == "DBH distribution") {
        setDBHDistributionData(gdata);
        return;
    }

    QLineSeries * prev;

    graphdata = gdata;
    std::vector<float> cumulate;

    // clear previous series
    chart->removeAllSeries();

    // create new series
    cumulate.resize(gdata->getHoriScale(), 0); // accumulation of previous series for stacking
    for(int a = 0; a < gdata->getNumSeries(); a++)
    {
        int tot = 0;
        for(int t = 0; t < gdata->getHoriScale(); t++)
            tot += gdata->getData(a, t);

        if(tot > 0) // ignore uniformly empty series
        {
            // cerr << "a = " << a << " tot = " << tot << endl;
            QLineSeries * curr = new QLineSeries();
            QLineSeries * prev = new QLineSeries();

            int idx = 0;
            for(int t = gdata->getTimeLine()->getTimeStart(); t <= gdata->getTimeLine()->getTimeEnd(); t++)
            {
                prev->append(QPointF(t, cumulate[idx]));
                cumulate[idx] += gdata->getData(a, idx);
                curr->append(QPointF(t, cumulate[idx]));
                idx++;
            }

            // get species colour
            float r = scene->getBiome()->getPFType(a)->basecol[0];
            float g = scene->getBiome()->getPFType(a)->basecol[1];
            float b = scene->getBiome()->getPFType(a)->basecol[2];

            QColor spccol((int) (r*255.0f), (int) (g*255.0f), (int) (b*255.0f));
            QAreaSeries * series = new QAreaSeries(curr, prev);
            QPen pen(spccol);
            pen.setWidth(0.5);
            series->setPen(pen);

            QLinearGradient gradient(QPointF(0,0), QPointF(0, 1));
            gradient.setColorAt(0.0, spccol);
            gradient.setColorAt(1.0, spccol);
            gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
            series->setBrush(gradient);

            chart->addSeries(series);
        }
    }

    // vertical timeline bar
    QLineSeries * above = new QLineSeries();
    QLineSeries * below = new QLineSeries();
    int tpos = gdata->getTimeLine()->getNow();
    float vmax = gdata->getVertScale();
    if(tpos == gdata->getTimeLine()->getTimeStart())
    {
        *above << QPointF((float) tpos + 0.05, vmax) << QPointF((float) tpos + 0.1f, vmax);
        *below << QPointF((float) tpos + 0.05, 0) << QPointF((float) tpos + 0.1f, 0);
    }
    else
    {
        if(tpos == gdata->getTimeLine()->getTimeEnd())
        {
            *above << QPointF((float) tpos - 0.1f, vmax) << QPointF((float) tpos - 0.05f, vmax);
            *below << QPointF((float) tpos - 0.1f, 0) << QPointF((float) tpos - 0.05f, 0);
        }
        else
        {
            *above << QPointF((float) tpos - 0.025f, vmax) << QPointF((float) tpos + 0.025f, vmax);
            *below << QPointF((float) tpos - 0.025f, 0) << QPointF((float) tpos + 0.025f, 0);
        }
    }

    QAreaSeries * bar = new QAreaSeries(above, below);
    QPen pen(Qt::black);
    pen.setWidth(2);
    bar->setPen(pen);

    QLinearGradient gradient(QPointF(0,0), QPointF(0, 1));
    gradient.setColorAt(0.0, Qt::black);
    gradient.setColorAt(1.0, Qt::black);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    bar->setBrush(gradient);
    chart->addSeries(bar);

    // chart->removeAxis(Qt:Vertical);

    chart->createDefaultAxes();

    QCategoryAxis *axisX = new QCategoryAxis;
    axisX->setMin((float) gdata->getTimeLine()->getTimeStart());
    axisX->setMax((float) gdata->getTimeLine()->getTimeEnd());
    axisX->setStartValue((float) gdata->getTimeLine()->getTimeStart());
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    for(int i = 0; i < (int) xlabels.size(); i++)
    {
          axisX->append(std::to_string(xlabels[i]).c_str(), i+1);
    }
    chart->setAxisX(axisX, bar);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0.0f, gdata->getVertScale());
    axisY->setTitleText(QString::fromStdString(gdata->getTitle()));
    // axisX->setTickCount(5);
    axisY->setLabelFormat("%d");
    chart->setAxisY(axisY, bar);
    /*
    chart->axes(Qt::Horizontal).first()->setRange(0, gdata->getHoriScale());
    chart->axes(Qt::Horizontal).first()->setLabelFormat("%d");
    chart->axes(Qt::Vertical).first()->setRange(0, gdata->getVertScale());*/
}

void ChartWindow::setDBHDistributionData(TimelineGraph * gdata)
{
    chart->removeAllSeries();
    for(auto axis: chart->axes())
        chart->removeAxis(axis);

    int nspecies = scene->getBiome()->numPFTypes();
    int num_dbh_classes = 10;
    int current_timestep = gdata->getTimeLine()->getCurrentIdx();

    QStackedBarSeries *series = new QStackedBarSeries();

    for (int spc = 0; spc < nspecies; ++spc) {
        QBarSet *set = new QBarSet(QString::fromStdString(scene->getBiome()->getPFType(spc)->code));
        for (int dbh_class = 0; dbh_class < num_dbh_classes; ++dbh_class) {
            set->append(gdata->getData(dbh_class * nspecies + spc, current_timestep));
        }
        float r = scene->getBiome()->getPFType(spc)->basecol[0];
        float g = scene->getBiome()->getPFType(spc)->basecol[1];
        float b = scene->getBiome()->getPFType(spc)->basecol[2];
        QColor spccol((int) (r*255.0f), (int) (g*255.0f), (int) (b*255.0f));
        set->setBrush(spccol);
        series->append(set);
    }

    chart->addSeries(series);

    QStringList categories;
    for (int i = 0; i < num_dbh_classes; ++i) {
        categories << QString("%1-%2").arg(i*10).arg((i+1)*10);
    }
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0.0f, gdata->getVertScale());
    axisY->setTitleText(QString::fromStdString(gdata->getTitle()));
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}
