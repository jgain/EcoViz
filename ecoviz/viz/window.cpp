/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2020  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/


/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "window.h"
#include "vecpnt.h"
#include "export_dialog.h"

#include <cmath>
#include <string>
#include<QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

using namespace std;


////
// Window
///

QSize Window::sizeHint() const
{
    return QSize(1600, 1000);
}

void Window::setupRenderPanel()
{
    // default rendering parameters, set using text entry
    // mirrors TRenderer settings
    // grid params
    gridIntensity = 0.8f; // 80% of base colour
    gridSepX = 2500.0f; // separation of grid lines, depends on how input data is scaled
    gridSepZ = 2500.0f; //
    gridWidth = 1.5f; // in pixels?

    // contour params
    contourSep = 25.f; // separation (Y direction) depends on how input data is normalized
    numContours = 1.0f / contourSep;
    contourWidth = 1.0f; // in pixels ?
    contourIntensity = 1.2f; // 130% of base colour

    // radiance scaling parameters
    radianceTransition = 0.2f;
    radianceEnhance = 3.0f;

    // map parameters
    sunMonth = 1;
    wetMonth = 1;
    tempMonth = 1;

    // render panel
    renderPanel = new QWidget;
    QVBoxLayout *renderLayout = new QVBoxLayout;

    // Grid Line Widgets
    QGroupBox *gridGroup = new QGroupBox(tr("Grid Lines"));
    QCheckBox * checkGridLines = new QCheckBox(tr("Show Grid Lines"));
    checkGridLines->setChecked(false);
    QLabel *gridSepXLabel = new QLabel(tr("Grid Sep X:"));
    gridSepXEdit = new QLineEdit;
    gridSepXEdit->setFixedWidth(60);
    // gridSepXEdit->setValidator(new QDoubleValidator(0.0, 500000.0, 2, gridSepXEdit));
    gridSepXEdit->setInputMask("0000.0");
    QLabel *gridSepZLabel = new QLabel(tr("Grid Sep Z:"));
    gridSepZEdit = new QLineEdit;
    gridSepZEdit->setFixedWidth(60);
    // gridSepZEdit->setValidator(new QDoubleValidator(0.0, 500000.0, 2, gridSepZEdit));
    gridSepZEdit->setInputMask("0000.0");
    QLabel *gridWidthLabel = new QLabel(tr("Grid Line Width:"));
    gridWidthEdit = new QLineEdit;
    gridWidthEdit->setFixedWidth(60);
    // gridWidthEdit->setValidator(new QDoubleValidator(0.0, 10.0, 2, gridWidthEdit));
    gridWidthEdit->setInputMask("0.0");
    QLabel *gridIntensityLabel = new QLabel(tr("Grid Intensity:"));
    gridIntensityEdit = new QLineEdit;
    gridIntensityEdit->setFixedWidth(60);
    // gridIntensityEdit->setValidator(new QDoubleValidator(0.0, 2.0, 2, gridIntensityEdit));
    gridIntensityEdit->setInputMask("0.0");

    // set initial grid values
    gridSepXEdit->setText(QString::number(gridSepX, 'g', 2));
    gridSepZEdit->setText(QString::number(gridSepZ, 'g', 2));
    gridWidthEdit->setText(QString::number(gridWidth, 'g', 2));
    gridIntensityEdit->setText(QString::number(gridIntensity, 'g', 2));

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(checkGridLines, 0, 0);
    gridLayout->addWidget(gridSepXLabel, 1, 0);
    gridLayout->addWidget(gridSepXEdit, 1, 1);
    gridLayout->addWidget(gridSepZLabel, 2, 0);
    gridLayout->addWidget(gridSepZEdit, 2, 1);
    gridLayout->addWidget(gridWidthLabel, 3, 0);
    gridLayout->addWidget(gridWidthEdit, 3, 1);
    gridLayout->addWidget(gridIntensityLabel, 4, 0);
    gridLayout->addWidget(gridIntensityEdit, 4, 1);
    gridGroup->setLayout(gridLayout);

    // Contour Widgets
    QGroupBox *contourGroup = new QGroupBox(tr("Contours"));
    QCheckBox * checkContours = new QCheckBox(tr("Show Contours"));
    checkContours->setChecked(false);
    QLabel *contourSepLabel = new QLabel(tr("Contour Sep:"));
    contourSepEdit = new QLineEdit;
    contourSepEdit->setFixedWidth(60);
    //contourSepEdit->setValidator(new QDoubleValidator(0.0, 10000.0, 2, contourSepEdit));
    contourSepEdit->setInputMask("000.0");
    QLabel *contourWidthLabel = new QLabel(tr("Contour Line Width:"));
    contourWidthEdit = new QLineEdit;
    // contourWidthEdit->setValidator(new QDoubleValidator(0.0, 10.0, 2, contourWidthEdit));
    contourWidthEdit->setInputMask("0.0");
    contourWidthEdit->setFixedWidth(60);
    QLabel *contourIntensityLabel = new QLabel(tr("Contour Intensity:"));
    contourIntensityEdit = new QLineEdit;
    contourIntensityEdit->setFixedWidth(60);
    contourIntensityEdit->setInputMask("0.0");

    // set initial contour values
    contourSepEdit->setText(QString::number(contourSep, 'g', 2));
    contourWidthEdit->setText(QString::number(contourWidth, 'g', 2));
    contourIntensityEdit->setText(QString::number(contourIntensity, 'g', 2));

    QGridLayout *contourLayout = new QGridLayout;
    contourLayout->addWidget(checkContours, 0, 0);
    contourLayout->addWidget(contourSepLabel, 1, 0);
    contourLayout->addWidget(contourSepEdit, 1, 1);
    contourLayout->addWidget(contourWidthLabel, 2, 0);
    contourLayout->addWidget(contourWidthEdit, 2, 1);
    contourLayout->addWidget(contourIntensityLabel, 3, 0);
    contourLayout->addWidget(contourIntensityEdit, 3, 1);
    contourGroup->setLayout(contourLayout);

    // Radiance
    QGroupBox *radianceGroup = new QGroupBox(tr("Radiance"));
    QLabel *radianceEnhanceLabel = new QLabel(tr("Radiance Enhancement:"));
    radianceEnhanceEdit = new QLineEdit;
    radianceEnhanceEdit->setFixedWidth(60);
    radianceEnhanceEdit->setInputMask("0.0");

    // set initial radiance values
    radianceEnhanceEdit->setText(QString::number(radianceEnhance, 'g', 2));

    QGridLayout *radianceLayout = new QGridLayout;
    radianceLayout->addWidget(radianceEnhanceLabel, 0, 0);
    radianceLayout->addWidget(radianceEnhanceEdit, 0, 1);
    radianceGroup->setLayout(radianceLayout);

    // map display
    QGroupBox *mapGroup = new QGroupBox(tr("Maps"));
    QGridLayout *mapLayout = new QGridLayout;

    sunMapRadio = new QRadioButton(tr("Sunlight  mth"));
    wetMapRadio = new QRadioButton(tr("Moisture  mth"));
    chmMapRadio = new QRadioButton(tr("Canopy Height"));
    noMapRadio = new QRadioButton(tr("None"));

    sunMapEdit = new QLineEdit;
    sunMapEdit->setFixedWidth(60);
    sunMapEdit->setInputMask("00");
    sunMapEdit->setText(QString::number(sunMonth, 'i', 1));

    wetMapEdit = new QLineEdit;
    wetMapEdit->setFixedWidth(60);
    wetMapEdit->setInputMask("00");
    wetMapEdit->setText(QString::number(wetMonth, 'i', 1));

    mapLayout->addWidget(sunMapRadio, 0, 0);
    mapLayout->addWidget(sunMapEdit, 0, 1);
    mapLayout->addWidget(wetMapRadio, 1, 0);
    mapLayout->addWidget(wetMapEdit, 1, 1);
    mapLayout->addWidget(chmMapRadio, 3, 0);
    mapLayout->addWidget(noMapRadio, 4, 0);
    mapGroup->setLayout(mapLayout);

    renderLayout->addWidget(gridGroup);
    renderLayout->addWidget(contourGroup);
    renderLayout->addWidget(radianceGroup);
    renderLayout->addWidget(mapGroup);

    // signal to slot connections
    connect(checkContours, SIGNAL(stateChanged(int)), this, SLOT(showContours(int)));
    connect(checkGridLines, SIGNAL(stateChanged(int)), this, SLOT(showGridLines(int)));
    connect(gridSepXEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridSepZEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridWidthEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridIntensityEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourSepEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourWidthEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourIntensityEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(radianceEnhanceEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(radianceEnhanceEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));
    connect(sunMapEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));
    connect(wetMapEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));

    // map radio buttons
    connect(sunMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(wetMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(chmMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(noMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));

    renderPanel->setLayout(renderLayout);
}

void Window::setupPlantPanel()
{
    // plant panel
    plantPanel = new QWidget;
    QVBoxLayout *plantLayout = new QVBoxLayout;

    // global plant parameters
    QGroupBox *globalGroup = new QGroupBox(tr("Global"));
    QGridLayout *globalLayout = new QGridLayout;
    // checkCanopy = new QCheckBox(tr("Show Plants"));
    // checkCanopy->setChecked(true);

    // checkUndergrowth = new QCheckBox(tr("Show Undergrowth Plants"));
    // checkUndergrowth->setChecked(true);

    // Smoothing
    QLabel *smoothLabel = new QLabel(tr("Smoothing Level:"));
    QIntValidator *intvalidate = new QIntValidator(this);
    smoothEdit = new QLineEdit("0", this);
    smoothEdit->setValidator(intvalidate);
    smoothEdit->setFixedWidth(30);

    // globalLayout->addWidget(checkCanopy, 0, 0);
    globalLayout->addWidget(smoothLabel, 0, 0);
    globalLayout->addWidget(smoothEdit, 0, 1);
    globalGroup->setLayout(globalLayout);

    // per species plant parameters
    QGroupBox *speciesGroup = new QGroupBox(tr("Per Species"));
    QGridLayout *speciesLayout = new QGridLayout;
    checkS0 = new QCheckBox(tr("Species 0"));
    checkS0->setChecked(true);
    checkS1 = new QCheckBox(tr("Species 1"));
    checkS1->setChecked(true);
    checkS2 = new QCheckBox(tr("Species 2"));
    checkS2->setChecked(true);
    checkS3 = new QCheckBox(tr("Species 3"));
    checkS3->setChecked(true);
    checkS4 = new QCheckBox(tr("Species 4"));
    checkS4->setChecked(true);
    checkS5 = new QCheckBox(tr("Species 5"));
    checkS5->setChecked(true);
    checkS6 = new QCheckBox(tr("Species 6"));
    checkS6->setChecked(true);
    checkS7 = new QCheckBox(tr("Species 7"));
    checkS7->setChecked(true);
    checkS8 = new QCheckBox(tr("Species 8"));
    checkS8->setChecked(true);
    checkS9 = new QCheckBox(tr("Species 9"));
    checkS9->setChecked(true);
    checkS10 = new QCheckBox(tr("Species 10"));
    checkS10->setChecked(true);
    checkS11 = new QCheckBox(tr("Species 11"));
    checkS11->setChecked(true);
    checkS12 = new QCheckBox(tr("Species 12"));
    checkS12->setChecked(true);
    checkS13 = new QCheckBox(tr("Species 13"));
    checkS13->setChecked(true);
    checkS14 = new QCheckBox(tr("Species 14"));
    checkS14->setChecked(true);
    checkS15 = new QCheckBox(tr("Species 15"));
    checkS15->setChecked(true);
    QPushButton * plantsOn = new QPushButton(tr("All Visible"));
    QPushButton * plantsOff = new QPushButton(tr("None Visible"));;

    speciesLayout->addWidget(checkS0, 0, 0);
    speciesLayout->addWidget(checkS1, 1, 0);
    speciesLayout->addWidget(checkS2, 2, 0);
    speciesLayout->addWidget(checkS3, 3, 0);
    speciesLayout->addWidget(checkS4, 4, 0);
    speciesLayout->addWidget(checkS5, 5, 0);
    speciesLayout->addWidget(checkS6, 6, 0);
    speciesLayout->addWidget(checkS7, 7, 0);
    speciesLayout->addWidget(checkS8, 8, 0);
    speciesLayout->addWidget(checkS9, 9, 0);
    speciesLayout->addWidget(checkS10, 10, 0);
    speciesLayout->addWidget(checkS11, 11, 0);
    speciesLayout->addWidget(checkS12, 12, 0);
    speciesLayout->addWidget(checkS13, 13, 0);
    speciesLayout->addWidget(checkS14, 14, 0);
    speciesLayout->addWidget(checkS15, 15, 0);
    speciesLayout->addWidget(plantsOn, 16, 0);
    speciesLayout->addWidget(plantsOff, 17, 0);
    speciesGroup->setLayout(speciesLayout);

    plantLayout->addWidget(globalGroup);
    plantLayout->addWidget(speciesGroup);

    // signal to slot connections
    connect(plantsOn, SIGNAL(clicked()), this, SLOT(allPlantsOn()));
    connect(plantsOff, SIGNAL(clicked()), this, SLOT(allPlantsOff()));
    // connect(checkCanopy, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    // connect(checkUndergrowth, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS0, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS1, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS2, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS3, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS4, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS5, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS6, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS7, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS8, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS9, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS10, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS11, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS12, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS13, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS14, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(checkS15, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));
    connect(smoothEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));

    plantPanel->setLayout(plantLayout);
}

void Window::setupVizPanel()
{
    vizPanel = new QWidget;
    QGridLayout *vizLayout = new QGridLayout;
    vizLayout->setSpacing(3);
    // vizLayout->setMargin(1);
    vizLayout->setContentsMargins(3, 3, 3, 3);

    // OpenGL widget
    // Specify an OpenGL 3.2 format.
    QGLFormat glFormat;
    glFormat.setVersion( 4, 1 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( false );

    vizLayout->setRowStretch(0, 6);
    vizLayout->setRowStretch(1, 24);
    vizLayout->setRowStretch(2, 1);
    vizLayout->setRowStretch(3, 8);

    // transect views
    for(int i = 0; i < 2; i++)
    {
        GLTransect * tview = new GLTransect(glFormat, scenes[i], transectControls[i]);
        tview->getRenderer()->setRadianceScalingParams(radianceEnhance);

        // signal to slot connections
        connect(tview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
        transectViews.push_back(tview);

        vizLayout->addWidget(tview, 0, i);
    }

    // main perspective views
    for(int i = 0; i < 2; i++)
    {
        GLWidget * pview = new GLWidget(glFormat, scenes[i], transectControls[i]);

        numGridX = 1.0f / gridSepX;
        numGridZ = 1.0f / gridSepZ;
        pview->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
        pview->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
        pview->getRenderer()->setRadianceScalingParams(radianceEnhance);

        // signal to slot connections
        connect(pview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
        connect(pview, SIGNAL(signalShowTransectView()), this, SLOT(showTransectViews()));

        perspectiveViews.push_back(pview);
        vizLayout->addWidget(pview, 1, i);
    }

    // chart views
    for(int i = 0; i < 2; i++)
    {
        ChartWindow * cview = new ChartWindow(this, 800, 200);
        TimelineGraph * gmodel = new TimelineGraph();

        // signal to slot connections
        // connect(cview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));

        chartViews.push_back(cview);
        graphModels.push_back(gmodel);
        vizLayout->addWidget(cview, 3, i);
    }

    // timeline views
    for(int i = 0; i < 2; i++)
    {
        TimeWindow * tview = new TimeWindow(this, 1, 2, 800, 50);

        // signal to slot connections
        connect(tview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
        connect(tview, SIGNAL(signalRebindPlants()), perspectiveViews[i], SLOT(rebindPlants()));
        connect(tview, SIGNAL(signalRebindPlants()), transectViews[i], SLOT(rebindPlants()));
        connect(tview, SIGNAL(signalRebindPlants()), chartViews[i], SLOT(updateTimeBar()));

        timelineViews.push_back(tview);
        vizLayout->addWidget(tview, 2, i);
    }

    vizPanel->setLayout(vizLayout);
    vizPanel->setStyleSheet("background-color:grey;");
    /*
    vizPanel->setStyleSheet(QString::fromUtf8("ChartWindow\n"
    "{\n"
    "     background-color: red;\n"
    "}\n"
    ""));*/
}

Window::Window(string datadir)
{
    QWidget *mainWidget = new QWidget;
    QGridLayout *mainLayout = new QGridLayout();

    mainLayout->setSpacing(1);
    // mainLayout->setMargin(1);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(mainWidget);

    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 0);
    mainLayout->setColumnStretch(2, 1);

    setupRenderPanel();
    setupPlantPanel();

    // load scenes
    for(int i = 0; i < 2; i++)
    {
        Scene * s = new Scene(datadir);
        scenes.push_back(s);
    }

    for(int i = 0; i < 2; i++)
    {
        Transect * t = new Transect(scenes[i]->getTerrain());
        transectControls.push_back(t);
    }

    setupVizPanel();

    mainLayout->addWidget(renderPanel, 0, 0, Qt::AlignTop);
    mainLayout->addWidget(plantPanel, 0, 1, Qt::AlignTop);
    mainLayout->addWidget(vizPanel, 0, 2);

    createActions();
    createMenus();

    readMitsubaExportProfiles("../../data/mitsubaExportProfiles");

    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("EcoViz"));
    mainWidget->setMouseTracking(true);
    setMouseTracking(true);

    renderPanel->hide();
    plantPanel->hide();
}

Window::~Window()
{
    // delete transect controllers
    for(auto &it: transectControls)
        delete(it);
}

void Window::run_viewer()
{
    for(int i = 0; i < 2; i++)
    {
        scenes[i]->loadScene(1, 25);
        transectControls[i]->init();
        transectViews[i]->setScene(scenes[i]);
        perspectiveViews[i]->setScene(scenes[i]);
        timelineViews[i]->setScene(scenes[i]);
        transectViews[i]->setVisible(false);
        graphModels[i]->setTimeLine(scenes[i]->getTimeline());
        graphModels[i]->extractDBHSums(scenes[i]);
        graphModels[i]->setVertScale(400000);
        chartViews[i]->setScene(scenes[i]);
        chartViews[i]->setData(graphModels[i]);
    }

    repaintAllGL();
}

void Window::scaleRenderParams(float scale)
{
    gridSepX = scale / 5.0f; // separation of grid lines, depends on how input data is scaled
    gridSepZ = scale / 5.0f;
    numGridX = 1.0f / gridSepX;
    numGridZ = 1.0f / gridSepZ;
    gridSepXEdit->setText(QString::number(gridSepX, 'g', 2));
    gridSepZEdit->setText(QString::number(gridSepZ, 'g', 2));

    contourSep = scale / 100.f; // separation (Y direction) depends on how input data is normalized
    numContours = 1.0f / contourSep;
    contourSepEdit->setText(QString::number(contourSep, 'g', 2));

    for(auto pview: perspectiveViews)
    {
        pview->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
        pview->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
        pview->getRenderer()->setRadianceScalingParams(radianceEnhance);
    }
}

void Window::keyPressEvent(QKeyEvent *e)
{
    // pass to render windows
    for(auto pview: perspectiveViews)
        pview->keyPressEvent(e);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    QWidget *child=childAt(event->pos());
    QGLWidget *glwidget = qobject_cast<QGLWidget *>(child);
    if(glwidget) {
        QMouseEvent *glevent=new QMouseEvent(event->type(),glwidget->mapFromGlobal(event->globalPos()),event->button(),event->buttons(),event->modifiers());
        QCoreApplication::postEvent(glwidget,glevent);
    }
}

void Window::repaintAllGL()
{
    for(auto pview: perspectiveViews)
        pview->repaint();
    for(auto tview: transectViews)
        tview->repaint();
    for(auto mview: timelineViews)
        mview->repaint();
    for(auto cview: chartViews)
        cview->repaint();
}

void Window::showRenderOptions()
{
    renderPanel->setVisible(showRenderAct->isChecked());
}

void Window::showPlantOptions()
{
    plantPanel->setVisible(showPlantAct->isChecked());
}

void Window::showContours(int show)
{
    for(auto pview: perspectiveViews)
        pview->getRenderer()->drawContours(show == Qt::Checked);
    repaintAllGL();
}

void Window::showTransectViews()
{
    for(int i = 0; i < 2; i++)
    {
        if(transectControls[i]->getValidFlag())
            transectViews[i]->setVisible(true);
    }
}

void Window::showGridLines(int show)
{
    for(auto pview: perspectiveViews)
        pview->getRenderer()->drawGridlines(show == Qt::Checked);
    repaintAllGL();
}

void Window::allPlantsOn()
{
    for(auto pview: perspectiveViews)
        pview->setAllSpecies(true);
    checkS0->setChecked(true);
    checkS1->setChecked(true);
    checkS2->setChecked(true);
    checkS3->setChecked(true);
    checkS4->setChecked(true);
    checkS5->setChecked(true);
    checkS6->setChecked(true);
    checkS7->setChecked(true);
    checkS8->setChecked(true);
    checkS9->setChecked(true);
    checkS10->setChecked(true);
    checkS11->setChecked(true);
    checkS12->setChecked(true);
    checkS13->setChecked(true);
    checkS14->setChecked(true);
    checkS15->setChecked(true);
}

void Window::allPlantsOff()
{
    for(auto pview: perspectiveViews)
        pview->setAllSpecies(false);
    checkS0->setChecked(false);
    checkS1->setChecked(false);
    checkS2->setChecked(false);
    checkS3->setChecked(false);
    checkS4->setChecked(false);
    checkS5->setChecked(false);
    checkS6->setChecked(false);
    checkS7->setChecked(false);
    checkS8->setChecked(false);
    checkS9->setChecked(false);
    checkS10->setChecked(false);
    checkS11->setChecked(false);
    checkS12->setChecked(false);
    checkS13->setChecked(false);
    checkS14->setChecked(false);
    checkS15->setChecked(false);
}

void Window::plantChange(int show)
{
    bool vis = (bool) show;

    for(auto pview: perspectiveViews)
    {
        if(sender() == checkCanopy)
            pview->setCanopyVis(vis);
        if(sender() == checkUndergrowth)
            pview->setUndergrowthVis(vis);
        if(sender() == checkS0)
            pview->toggleSpecies(0, vis);
        if(sender() == checkS1)
            pview->toggleSpecies(1, vis);
        if(sender() == checkS2)
            pview->toggleSpecies(2, vis);
        if(sender() == checkS3)
            pview->toggleSpecies(3, vis);
        if(sender() == checkS4)
            pview->toggleSpecies(4, vis);
        if(sender() == checkS5)
            pview->toggleSpecies(5, vis);
        if(sender() == checkS6)
            pview->toggleSpecies(6, vis);
        if(sender() == checkS7)
            pview->toggleSpecies(7, vis);
        if(sender() == checkS8)
            pview->toggleSpecies(8, vis);
        if(sender() == checkS9)
            pview->toggleSpecies(9, vis);
        if(sender() == checkS10)
            pview->toggleSpecies(10, vis);
        if(sender() == checkS11)
            pview->toggleSpecies(11, vis);
        if(sender() == checkS12)
            pview->toggleSpecies(12, vis);
        if(sender() == checkS13)
            pview->toggleSpecies(13, vis);
        if(sender() == checkS14)
            pview->toggleSpecies(14, vis);
        if(sender() == checkS15)
            pview->toggleSpecies(15, vis);
    }
}

void Window::lineEditChange()
{
    bool ok;
    float val;
    int ival;
    float tx, ty, hr;

    tx = 1.0f; ty = 1.0f; // to fix when scale added
    hr = 1.0f;

    if(sender() == gridSepXEdit)
    {
        val = gridSepXEdit->text().toFloat(&ok);
        if(ok)
        {
            gridSepX = val;
            numGridX = tx / gridSepX; // convert separation to num grid lines
        }
    }
    if(sender() == gridSepZEdit)
    {
        val = gridSepZEdit->text().toFloat(&ok);
        if(ok)
        {
            gridSepZ = val;
            numGridZ = ty / gridSepZ;
        }
    }
    if(sender() == gridWidthEdit)
    {
        val = gridWidthEdit->text().toFloat(&ok);
        if(ok)
        {
            gridWidth = val;
        }
    }
    if(sender() == gridIntensityEdit)
    {
        val = gridIntensityEdit->text().toFloat(&ok);
        if(ok)
        {
            gridIntensity = val;
        }
    }
    if(sender() == contourSepEdit)
    {
        val = contourSepEdit->text().toFloat(&ok);
        if(ok)
        {
            contourSep = val;
            numContours = hr / contourSep;
        }
    }
    if(sender() == contourWidthEdit)
    {
        val = contourWidthEdit->text().toFloat(&ok);
        if(ok)
        {
            contourWidth = val;
        }
    }
    if(sender() == contourIntensityEdit)
    {
        val = contourIntensityEdit->text().toFloat(&ok);
        if(ok)
        {
            contourIntensity = val;
        }
    }
    if(sender() == radianceEnhanceEdit)
    {
        val = radianceEnhanceEdit->text().toFloat(&ok);
        if(ok)
        {
            radianceEnhance = val;
        }
    }
    if(sender() == sunMapEdit)
    {
        ival = sunMapEdit->text().toInt(&ok);
        if(ok)
        {
            sunMonth = ival;
            if(sunMonth < 1)
                sunMonth = 1;
            if(sunMonth > 12)
                sunMonth = 12;
        }
        if(sunMapRadio->isChecked())
            for(auto pview: perspectiveViews)
                pview->setMap(TypeMapType::SUNLIGHT, sunMonth-1);
    }
    if(sender() == wetMapEdit)
    {
        ival = wetMapEdit->text().toInt(&ok);
        if(ok)
        {
            wetMonth = ival;
            if(wetMonth < 1)
                wetMonth = 1;
            if(wetMonth > 12)
                wetMonth = 12;
        }
        if(wetMapRadio->isChecked())
            for(auto pview: perspectiveViews)
                pview->setMap(TypeMapType::WATER, wetMonth-1);
    }
    if(sender() == smoothEdit)
    {
        ival = smoothEdit->text().toInt(&ok);
        if(ok)
            setSmoothing(ival);
    }

    // cerr << "val entered " << val << endl;

    // without this the renderer defaults back to factory settings at certain stages - very wierd bug
    for(auto pview: perspectiveViews)
    {
        pview->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
        pview->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
        pview->getRenderer()->setRadianceScalingParams(radianceEnhance);
    }
    repaintAllGL();
}

void Window::mapChange(bool on)
{
    for(auto pview: perspectiveViews)
    {
        if(sunMapRadio->isChecked() && on)
            pview->setMap(TypeMapType::SUNLIGHT, sunMonth-1);
        if(wetMapRadio->isChecked() && on)
            pview->setMap(TypeMapType::WATER, wetMonth-1);
        if(chmMapRadio->isChecked() && on)
            pview->setOverlay(TypeMapType::CHM);
        if(noMapRadio->isChecked() && on)
            pview->setOverlay(TypeMapType::EMPTY);
    }
}

void Window::createActions()
{
    showRenderAct = new QAction(tr("Show Terrain Options"), this);
    showRenderAct->setCheckable(true);
    showRenderAct->setChecked(false);
    showRenderAct->setStatusTip(tr("Hide/Show Rendering Options"));
    connect(showRenderAct, SIGNAL(triggered()), this, SLOT(showRenderOptions()));

    showPlantAct = new QAction(tr("Show Plant Options"), this);
    showPlantAct->setCheckable(true);
    showPlantAct->setChecked(false);
    showPlantAct->setStatusTip(tr("Hide/Show Plant Options"));
    connect(showPlantAct, SIGNAL(triggered()), this, SLOT(showPlantOptions()));

    // Export Mitsuba
    exportMitsubaAct = new QAction(tr("Export Mitsuba"), this);
    connect(exportMitsubaAct, SIGNAL(triggered()), this, SLOT(exportMitsuba()));
}

void Window::createMenus()
{
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showRenderAct);
    viewMenu->addAction(showPlantAct);

    // Export Mitsuba
    viewMenu->addAction(exportMitsubaAct);
}

class AdjustmentRunnable : public QRunnable
{
public:
    AdjustmentRunnable(TimeWindow * tview, Scene * scene, int distance, int tstep)
        : QRunnable(), tview(tview), scene(scene), distance(distance), tstep(tstep)
    {
    }

    void run()
    {
        CohortMaps * maps = scene->cohortmaps.get();

        if (maps)
        {
            maps->do_adjustments(distance);
            scene->reset_sampler(maps->get_maxpercell());
        }
        if (tstep >= 0)
            tview->updateScene(tstep);
    }
private:
    TimeWindow * tview;
    Scene * scene;
    int distance, tstep;
};

void Window::setSmoothing(int d)
{
    for(int i = 0; i < 2; i++)
    {
        AdjustmentRunnable *runnable = new AdjustmentRunnable(timelineViews[i], scenes[i], d, scenes[i]->getTimeline()->getNow());
        runnable->setAutoDelete(true);
        QThreadPool::globalInstance()->start(runnable);
    }
}

void Window::readMitsubaExportProfiles(string profilesDirPath)
{
    QDir profilesDir(profilesDirPath.data());

    QStringList csvProfiles = profilesDir.entryList(QStringList() << "*.csv" << "*.CSV", QDir::Files);
    for (QString csvName : csvProfiles)
    {
        ifstream csvFile(profilesDirPath + "/" + csvName.toUtf8().data());

        string profileName = csvName.remove(".csv").toUtf8().data();
        string line;
        string plantCode;
        string maxHeightStr;
        string instanceId;
        string actualHeightStr;
        int count = 0;

        getline(csvFile, line);

        while (getline(csvFile, line))
        {
            count++;

            string delimiter = ";";

            // Plant code
            size_t pos = line.find(delimiter);
            string token = line.substr(0, pos);
            plantCode = token;
            line.erase(0, pos + delimiter.length());

            // Max height
            pos = line.find(delimiter);
            token = line.substr(0, pos);
            maxHeightStr = token;
            line.erase(0, pos + delimiter.length());

            // Instance id
            pos = line.find(delimiter);
            token = line.substr(0, pos);
            instanceId = token;
            line.erase(0, pos + delimiter.length());

            // Actual height
            actualHeightStr = line;

            if (this->profileToSpeciesMap.find(profileName) == this->profileToSpeciesMap.end())
            {
                this->profileToSpeciesMap.insert({ profileName , {} });
            }

            map<string, map<string, vector<MitsubaModel>>>::iterator itProfile = this->profileToSpeciesMap.find(profileName);

            if (itProfile->second.find(plantCode) == itProfile->second.end())
            {
                itProfile->second.insert({ plantCode, {} });
            }

            map<string, vector<MitsubaModel>>::iterator itPlantCode = itProfile->second.find(plantCode);
            itPlantCode->second.push_back({ stod(maxHeightStr), instanceId, stod(actualHeightStr) });
        }
    }

    cout << "readMitsubaExportProfiles finished !" << endl;
}

void Window::exportMitsuba()
{
    QStringList allProfiles;

    map<string, map<string, vector<MitsubaModel>>>::iterator it;
    for (it = this->profileToSpeciesMap.begin(); it != this->profileToSpeciesMap.end(); it++)
    {
        allProfiles.append(it->first.data());
    }

    bool ok = false;
    ExportSettings exportSettings = ExportDialog::getExportSettings(this, allProfiles, &ok);

    if (ok)
    {
        map<string, vector<MitsubaModel>> speciesMap = this->profileToSpeciesMap.find(exportSettings.profile)->second;

        QDir().mkdir("./instances");

        ofstream sceneXml;
        sceneXml.open("./instances/instances.xml");

        sceneXml << "<scene version=\"0.5.0\">\n";

        if (exportSettings.transect)
        {
            Transect* transect = this->transectControls[exportSettings.sceneIndex];
            this->scenes[exportSettings.sceneIndex]->exportSceneXml(speciesMap, sceneXml, transect);
        }
        else
        {
            this->scenes[exportSettings.sceneIndex]->exportSceneXml(speciesMap, sceneXml);
        }

        sceneXml << "</scene>\n";

        sceneXml.close();
        cout << "Export finished !" << endl;
    }
}