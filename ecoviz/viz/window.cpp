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
#include <QtCharts/QChartView>
#include <QButtonGroup>
#include <QAction>

using namespace std;

////
// PlantPanel
///

void PlantPanel::closeEvent(QCloseEvent* event)
{
    wparent->uncheckPlantPanel();
    event->accept();
}

////
// DataMapPanel
///

void DataMapPanel::closeEvent(QCloseEvent* event)
{
    wparent->uncheckDataMapPanel();
    event->accept();
}

////
// ViewPanel
///

void ViewPanel::closeEvent(QCloseEvent* event)
{
    wparent->uncheckViewPanel();
    event->accept();
}

////
// Window
///

QSize Window::sizeHint() const
{
    return QSize(1600, 1000); // 1600, 1000
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
/*
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
    mapGroup->setLayout(mapLayout);*/

    // camera controls
    QGroupBox *cameraGroup = new QGroupBox(tr("Camera"));
    QGridLayout *cameraLayout = new QGridLayout;
    cameraDropDown = new QComboBox();
    cameraDropDown->addItem(tr("Orbit"));
    cameraDropDown->addItem(tr("Flyover"));
    cameraLayout->addWidget(cameraDropDown, 0, 0);
    cameraGroup->setLayout(cameraLayout);

    renderLayout->addWidget(gridGroup);
    renderLayout->addWidget(contourGroup);
    renderLayout->addWidget(radianceGroup);
    renderLayout->addWidget(cameraGroup);
    // renderLayout->addWidget(mapGroup);

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
    // connect(sunMapEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));
    // connect(wetMapEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));

    // map radio buttons
    /*
    connect(sunMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(wetMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(chmMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));
    connect(noMapRadio, SIGNAL(toggled(bool)), this, SLOT(mapChange(bool)));*/
    // connect(cameraDropDown, SIGNAL(currentIndexChanged(int)), this, SLOT(cameraChange(int)));

    renderPanel->setLayout(renderLayout);
}

void Window::setupPlantPanel()
{
    /*if (plantPanel != nullptr)
    {
        // delete all sub items
        auto children = plantPanel->findChildren<QWidget*>();
        for (auto &widget: children)
          delete widget;
    } */
    // plant panel
    plantPanel = new PlantPanel(this);
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

    if (scenes.size()>0)
    {
        auto &sdata = scenes[0]->getBiome()->getSpeciesMetaData();

        for (int i=0; i<sdata.size(); ++i)
        {
            QString sname = QString::fromStdString(sdata[i].species_name);
            if (sname != "NA")
            {
                QCheckBox *cb = new QCheckBox();
                cb->setChecked(true);
                cb->setProperty("species_num", sdata[i].species_num_id);
                cb->setToolTip( QString::fromStdString(sdata[i].scientific_name));
                connect(cb, SIGNAL(stateChanged(int)), this, SLOT(plantChange(int)));

                QLabel *cln = new QLabel(sname);
                cln->setToolTip( QString::fromStdString(sdata[i].scientific_name));

                QLabel *clbox = new QLabel();
                clbox->setFixedWidth(60);
                clbox->setFixedHeight(24);
                QColor scolor;
                scolor.setRgbF(sdata[i].species_color[0], sdata[i].species_color[1], sdata[i].species_color[2], sdata[i].species_color[3]);

                clbox->setStyleSheet(QString("QLabel { background-color : %1; }").arg(scolor.name()));


                speciesLayout->addWidget(cb,i,0); // checkbox
                speciesLayout->addWidget(cln,i,2); // name
                speciesLayout->addWidget(clbox, i, 1); // color

            }
        }

    }


    QPushButton * plantsOn = new QPushButton(tr("All Visible"));
    QPushButton * plantsOff = new QPushButton(tr("None Visible"));;
    connect(plantsOn, SIGNAL(clicked()), this, SLOT(allPlantsOn()));
    connect(plantsOff, SIGNAL(clicked()), this, SLOT(allPlantsOff()));
    globalLayout->addWidget(plantsOn);
    globalLayout->addWidget(plantsOff);

    plantLayout->addWidget(globalGroup);
    plantLayout->addWidget(speciesGroup);


    speciesGroup->setLayout(speciesLayout);
    plantPanel->setLayout(plantLayout);
}

void Window::setupDataMapPanel()
{
    dataMapPanel = new DataMapPanel(this);
    QHBoxLayout *dataMapLayout = new QHBoxLayout;

    // Matrices of radio buttons
    QGroupBox *matrixMapLeftGroup = new QGroupBox(tr("Left Panel"));
    QGridLayout *matrixMapLeftLayout = new QGridLayout;
    QGroupBox *matrixMapRightGroup = new QGroupBox(tr("Right Panel"));
    QGridLayout *matrixMapRightLayout = new QGridLayout;

    if (scenes.size() > 0)
    {
        int rows = scenes[0]->getDataMaps()->getNumMaps();
        int cols = numRamps;

        // L and R postfix denotes left and right panels
        qbmgL = new QButtonGroup(); // exclusive radio button group for map choice
        QButtonGroup *qbrgL = new QButtonGroup(); // exclusive radio button group for ramp choice
        qbmgR = new QButtonGroup();
        QButtonGroup *qbrgR = new QButtonGroup();


        // headers
        QString hdr1 = QString::fromStdString(string("Data Type:"));
        QLabel *hdl1L = new QLabel(hdr1);
        matrixMapLeftLayout->addWidget(hdl1L,0,0);
        QLabel *hdl1R = new QLabel(hdr1);
        matrixMapRightLayout->addWidget(hdl1R,0,0);

        QString hdr2 = QString::fromStdString(string("Colours:"));
        QLabel *hdl2L = new QLabel(hdr2);
        matrixMapLeftLayout->addWidget(hdl2L,0,2);
        QLabel *hdl2R = new QLabel(hdr2);
        matrixMapRightLayout->addWidget(hdl2R,0,2);

        for(int r = 0; r <= rows; r++)
        {
            // radio button for map selection
            QRadioButton * qrmbL = new QRadioButton();
            QRadioButton * qrmbR = new QRadioButton();
            matrixMapLeftLayout->addWidget(qrmbL,r+1,0);
            matrixMapRightLayout->addWidget(qrmbR,r+1,0);
            if(r == 0)
            {
                qrmbL->setChecked(true);
                qrmbR->setChecked(true);
            }
            qbmgL->addButton(qrmbL, r);
            qbmgR->addButton(qrmbR, r);

            // data map name
            QString dmname;
            if(r == 0)
                dmname = QString::fromStdString(string("None"));
            else
                dmname = QString::fromStdString((* scenes[0]->getDataMaps()->getNames())[r-1]);

            QLabel *dmnlL = new QLabel(dmname);
            QLabel *dmnlR = new QLabel(dmname);

            matrixMapLeftLayout->addWidget(dmnlL,r+1,1); // name
            matrixMapRightLayout->addWidget(dmnlR,r+1,1);

            if(r < numRamps)
            {
                 // radio button for ramp selection
                QRadioButton * qrrbL = new QRadioButton();
                QRadioButton * qrrbR = new QRadioButton();
                if(r == 0)
                {
                    qrrbL->setChecked(true);
                    qrrbR->setChecked(true);
                }
                qbrgL->addButton(qrrbL, r);
                qbrgR->addButton(qrrbR, r);
                matrixMapLeftLayout->addWidget(qrrbL,r+1,2);
                matrixMapRightLayout->addWidget(qrrbR,r+1,2);

                // ramp name label
                QString rname = QString::fromStdString(ramp_names[r]);
                QLabel *rnlL = new QLabel(rname);
                QLabel *rnlR = new QLabel(rname);
                matrixMapLeftLayout->addWidget(rnlL,r+1,3);
                matrixMapRightLayout->addWidget(rnlR,r+1,3);
            }
        }

        // headers
        QString hdr3 = QString::fromStdString(string("Graph:"));
        QLabel *hdl3L = new QLabel(hdr3);
        matrixMapLeftLayout->addWidget(hdl3L,rows+2,0);
        QLabel *hdl3R = new QLabel(hdr3);
        matrixMapRightLayout->addWidget(hdl3R,rows+2,0);

        // left panel graphs
        int grows = chartViews[0]->getNumGraphs();
        bggL = new QButtonGroup();
        // get access to graphs
        for(int r = 0; r < grows; r++)
        {
            // radio button for map selection
            QRadioButton * qrmbL = new QRadioButton();
            matrixMapLeftLayout->addWidget(qrmbL,rows+r+3,0);
            if(r == 0)
                qrmbL->setChecked(true);
            bggL->addButton(qrmbL, r);

            // graph names
            QString gname = QString::fromStdString(chartViews[0]->getGraphName(r));
            QLabel *dmnlL = new QLabel(gname);
            matrixMapLeftLayout->addWidget(dmnlL,rows+r+3,1);
        }

        // right panel graphs
        grows = chartViews[1]->getNumGraphs();
        bggR = new QButtonGroup();
        // get access to graphs
        for(int r = 0; r < grows; r++)
        {
            // radio button for map selection
            QRadioButton * qrmbR = new QRadioButton();
            matrixMapRightLayout->addWidget(qrmbR,rows+r+3,0);
            if(r == 0)
                qrmbR->setChecked(true);
            bggR->addButton(qrmbR, r);

            // graph names
            QString gname = QString::fromStdString(chartViews[0]->getGraphName(r));
            QLabel *dmnlR = new QLabel(gname);
            matrixMapRightLayout->addWidget(dmnlR,rows+r+3,1);
        }

        connect(qbmgL, SIGNAL(idClicked(int)), this, SLOT(leftDataMapChoice(int)));
        connect(qbmgR, SIGNAL(idClicked(int)), this, SLOT(rightDataMapChoice(int)));
        connect(qbrgL, SIGNAL(idClicked(int)), this, SLOT(leftRampChoice(int)));
        connect(qbrgR, SIGNAL(idClicked(int)), this, SLOT(rightRampChoice(int)));
        connect(bggL, SIGNAL(idClicked(int)), this, SLOT(leftGraphChoice(int)));
        connect(bggR, SIGNAL(idClicked(int)), this, SLOT(rightGraphChoice(int)));
    }

    dataMapLayout->addWidget(matrixMapLeftGroup);
    dataMapLayout->addWidget(matrixMapRightGroup);
    matrixMapLeftGroup->setLayout(matrixMapLeftLayout);
    matrixMapRightGroup->setLayout(matrixMapRightLayout);
    dataMapPanel->setLayout(dataMapLayout);
}

void Window::setupViewPanel()
{
    viewPanel = new ViewPanel(this);
    QHBoxLayout *viewLayout = new QHBoxLayout;

    // Matrices of radio buttons
    QGroupBox *matrixViewLeftGroup = new QGroupBox(tr("Left Panel"));
    QGridLayout *matrixViewLeftLayout = new QGridLayout;
    QGroupBox *matrixViewRightGroup = new QGroupBox(tr("Right Panel"));
    QGridLayout *matrixViewRightLayout = new QGridLayout;

    if (scenes.size() > 0)
    {
        int rows = 12;
        int cols = 1;

        QPushButton * leftSave = new QPushButton(tr("Save View"));
        QPushButton * rightSave = new QPushButton(tr("Save View"));
        connect(leftSave,  &QPushButton::clicked, [=] { saveSceneView(0); });
        connect(rightSave,   &QPushButton::clicked, [=] { saveSceneView(1); });

        QPushButton * leftLoad = new QPushButton(tr("Load View"));
        QPushButton * rightLoad = new QPushButton(tr("Load View"));
        connect(leftLoad, &QPushButton::clicked, [=] { loadSceneView(0); });
        connect(rightLoad, &QPushButton::clicked, [=] { loadSceneView(1); });

        matrixViewLeftLayout->addWidget(leftSave);
        matrixViewLeftLayout->addWidget(leftLoad);
        matrixViewRightLayout->addWidget(rightSave);
        matrixViewRightLayout->addWidget(rightLoad);

        // minimap check box
        QCheckBox * mrbL = new QCheckBox(tr("left minimap"));
        QCheckBox * mrbR = new QCheckBox(tr("right minimap"));
        mrbL->setChecked(true);
        mrbR->setChecked(true);
        matrixViewLeftLayout->addWidget(mrbL);
        matrixViewRightLayout->addWidget(mrbR);
        connect(mrbL, SIGNAL(stateChanged(int)), this, SLOT(leftMinimapToggle(int)));
        connect(mrbR, SIGNAL(stateChanged(int)), this, SLOT(rightMinimapToggle(int)));
    }

    viewLayout->addWidget(matrixViewLeftGroup);
    viewLayout->addWidget(matrixViewRightGroup);
    matrixViewLeftGroup->setLayout(matrixViewLeftLayout);
    matrixViewRightGroup->setLayout(matrixViewRightLayout);
    viewPanel->setLayout(viewLayout);
}

void Window::setupVizTransect(QSurfaceFormat glFormat, int i)
{
    // transect views

    assert(i>=0 && i < 2);

    if (transectViews.size() == 0)
    {
        transectViews.resize(2);
        transectViews[0] = nullptr;
        transectViews[1] = nullptr;
    }
    else if (transectViews[i] != nullptr)
    {
        std::cerr << "Window::setupVizTransect - trying to add new transect where one exists already, must be removed first: index = " << i << std::endl;
        return;
    }

    GLTransect * tview = new GLTransect(glFormat, this, scenes[i], transectControls[i]);
    tview->getRenderer()->setRadianceScalingParams(radianceEnhance);

        // signal to slot connections
    connect(tview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    transectViews[i] = tview;

    vizLayout->addWidget(tview, 0, i*2);

}

void Window::destroyVizTransect(int i)
{
    assert(i >=0 && i < 2);

    if (transectViews.size() != 2 || transectControls.size() != 2)
    {
        std::cerr << "Window::destroyVizTransects - missing data, arrays need to have 2 elements\n";
        return;
    }

    if (transectViews[i] == nullptr)
    {
        std::cerr << "Window::destroyVizTransects - transectViews is null at index: " << i << std::endl;
        return;
    }

    if (transectControls[i] == nullptr)
    {
        std::cerr << "Window::destroyVizTransects -transectControls is null at index: " << i << std::endl;
        return;
    }

//    vizLayout->setRowStretch(0, 0);
//    for(int i = 0; i < 2; i++)
 //   {
  //
   //     vizLayout->removeWidget(transectViews[i]);
   // }

    unlockTransects();

    //vizLayout->removeWidget(lockTGroup);
    //QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    transectViews[i]->setVisible(false);
    transectViews[i]->setActive(false);
    transectViews[i]->hide();
    vizLayout->removeWidget(transectViews[i]);
    delete transectViews[i];
    transectViews[i] = nullptr;

    delete transectControls[i];
    transectControls[i] = nullptr;

    //transectsValid = false;
}


void Window::setupVizPerspective(QSurfaceFormat glFormat, int i)
{
    // main perspective views

    assert(i >=0 && i < 2);

    if (perspectiveViews.size() == 0)
    {
        perspectiveViews.resize(2);
        perspectiveViews[0] = nullptr;
        perspectiveViews[1] = nullptr;
    }
    else if (perspectiveViews[i] != nullptr)
    {
        std::cerr << "Window::setupVizPerspective - trying to add new view where one exists already, must be removed first: index = " << i << std::endl;
        return;
    }

    GLWidget * pview = new GLWidget(glFormat, this, scenes[i],
                                    transectControls[i],
                                    (i == 0 ? string("left"): string("right")),
                                    mapScenes[i]);

    numGridX = 1.0f / gridSepX;
    numGridZ = 1.0f / gridSepZ;

    pview->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
    pview->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
    pview->getRenderer()->setRadianceScalingParams(radianceEnhance);


    // signal to slot connections

    connect(pview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(pview, SIGNAL(signalShowTransectView()), this, SLOT(showTransectViews()));
    connect(pview, SIGNAL(signalSyncPlace(bool)), this, SLOT(transectSyncPlace(bool)));
    connect(pview, SIGNAL(signalRebindTransectPlants()), transectViews[i], SLOT(rebindPlants()));
    connect(pview, SIGNAL(signalExtractNewSubTerrain(int, int,int,int,int)), this,
            SLOT(extractNewSubTerrain(int,int,int,int,int)) );
    connect(pview, SIGNAL(signalExtractOtherSubTerrain(int, int,int,int,int)), this,
            SLOT(extractNewSubTerrain(int,int,int,int,int)) );
    connect(pview, SIGNAL(signalSyncDataMap()), this, SLOT(syncDataMapPanel()));
    //connect(pview, SIGNAL(signalUpdateOverviews()), this, SLOT(updateOverviews()));

    perspectiveViews[i] = pview;
    //perspectiveViews.push_back(pview);
    vizLayout->addWidget(pview, 1, i*2);

}

void Window::destroyVizPerspective(int i)
{
    assert(i >=0 && i < 2);

    if (perspectiveViews[i] == nullptr)
    {
        std::cerr << "Window::destroyVizPerspective - invalid index, view is null: " << i << std::endl;
        return;
    }

    perspectiveViews[i]->makeCurrent(); // PCM - requried to esnure the previous context is not current

    perspectiveViews[i]->hide();
    vizLayout->removeWidget(perspectiveViews[i]);
    delete perspectiveViews[i];
    perspectiveViews[i] = nullptr;
}

/*
void Window::destroyVizOvermap(int i)
{
    assert(i >=0 && i < 2);

    if (overviewMaps[i] == nullptr)
    {
        std::cerr << "Window::destroyVizOvermap - invalid index, view is null: " << i << std::endl;
        return;
    }

    overviewMaps[i]->hide();
    delete overviewMaps[i];
    overviewMaps[i] = nullptr;
}

*/

void Window::setupVizChartViews(QSurfaceFormat glFormat, int i)
{
    // chart views

    ChartWindow * cview = new ChartWindow(this, 800, 200);
    cview->setParent(this);
    //TimelineGraph * gmodel = new TimelineGraph();

    // signal to slot connections
    // connect(cview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    std::vector< TimelineGraph* > tgs;
    chartViews.push_back(cview);

    graphModels.push_back( tgs );
    vizLayout->addWidget(cview, 3, i*2);

}

void Window::setupVizTimeline(QSurfaceFormat glFormat, int i)
{
    // timeline views

    TimeWindow * tview = new TimeWindow(this, this, 1, 2, 800, 50);

    // signal to slot connections
    connect(tview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(tview, SIGNAL(signalRebindPlants()), perspectiveViews[i], SLOT(rebindPlants()));
    connect(tview, SIGNAL(signalRebindPlants()), transectViews[i], SLOT(rebindPlants()));
    connect(tview, SIGNAL(signalRebindPlants()), chartViews[i], SLOT(updateTimeBar()));
    connect(tview, SIGNAL(signalSync(int)), this, SLOT(timelineSync(int)));

    timelineViews.push_back(tview);
    vizLayout->addWidget(tview, 2, i*2);
}

/*
void Window::setupVizOverMap(QSurfaceFormat glFormat, int i)
{
    if (overviewMaps.size() == 0)
    {
        overviewMaps.resize(2);
        overviewMaps[0] = nullptr;
        overviewMaps[1] = nullptr;
    }
    else if (overviewMaps[i] != nullptr)
    {
        std::cerr << "Window::setupVizOverMap - trying to add new view where one exists already, must be removed first: index = " << i << std::endl;
        return;
    }

    // PCM: overview maps L/R

    GLOverview *oview = new GLOverview(glFormat, this, mapScenes[i], i);

    // set params

    // signal to slot connections
    connect(oview, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(oview, SIGNAL(signalExtractNewSubTerrain(int, int,int,int,int)), this,
            SLOT(extractNewSubTerrain(int,int,int,int,int)) );
    //connect(oview, SIGNAL(signalRebindPlants()), perspectiveViews[i], SLOT(rebindPlants()));
    //connect(pview, SIGNAL(signalSyncPlace(bool)), this, SLOT(transectSyncPlace(bool)));

    overviewMaps[i] = oview;
    // vizLayout->addWidget(oview, 4, i*2);
}
*/

/*
void Window::positionVizOverMap(int i)
{
    if(perspectiveViews.size() == 2 && overviewMaps.size() == 2)
        if(perspectiveViews[i] != nullptr && overviewMaps[i] != nullptr)
        {
            overviewMaps[i]->setParent(0);
            overviewMaps[i]->show();
            overviewMaps[i]->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
            overviewMaps[i]->setWindowState( (overviewMaps[i]->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            overviewMaps[i]->raise(); overviewMaps[i]->activateWindow();

            QRect perspectiveRect = perspectiveViews[i]->geometry();
            QPoint cornerPos = this->mapToGlobal(perspectiveRect.topRight());
            QRect overviewRect = overviewMaps[i]->geometry();

            // cerr << "cornerpos " << i << " = " << cornerPos.x() << ", " << cornerPos.y() << endl;
            cornerPos.setX(cornerPos.x() - overviewRect.width()+1);
            overviewMaps[i]->move(cornerPos);
            overviewMaps[i]->resize(overviewMaps[i]->sizeHint());
            // cerr << "cornerpos " << i << " = " << cornerPos.x() << ", " << cornerPos.y() << endl;
        }
}
*/

void Window::setupVizPanel()
{
    vizPanel = new QWidget;
    vizLayout = new QGridLayout;
    vizLayout->setSpacing(3);
    // vizLayout->setMargin(1);
    vizLayout->setContentsMargins(3, 3, 3, 3);


    QSurfaceFormat glFormat;
    glFormat.setProfile( QSurfaceFormat::CoreProfile );
    glFormat.setDepthBufferSize(24);
    // QSurfaceFormat::setDefaultFormat(format);
    glFormat.setVersion(4,1);


    // glFormat.setOptions(QSurfaceFormat::DeprecatedFunctions);

    // vizLayout->setRowStretch(0, 6);
    vizLayout->setRowStretch(0, 0);
    vizLayout->setRowStretch(1, 24);
    vizLayout->setRowStretch(2, 1);
    vizLayout->setRowStretch(3, 8);
    // PCM: add overview map
    // vizLayout->setRowStretch(4,6);

    vizLayout->setColumnStretch(0, 600);
    vizLayout->setColumnStretch(1, 50);
    vizLayout->setColumnStretch(2, 600);

    // setup widgets for panel
    setupVizTransect(glFormat, 0);
    setupVizTransect(glFormat, 1);
    setupVizPerspective(glFormat, 0);
    setupVizPerspective(glFormat, 1);
    setupVizChartViews(glFormat, 0);
    setupVizChartViews(glFormat, 1);
    setupVizTimeline(glFormat, 0);
    setupVizTimeline(glFormat, 1);
    //setupVizOverMap(glFormat, 0);
    //setupVizOverMap(glFormat, 1);

    // lock buttons
    QVBoxLayout *lockTLayout = new QVBoxLayout;
    lockTGroup = new QGroupBox;
    QVBoxLayout *lockVLayout = new QVBoxLayout;
    QGroupBox *lockVGroup = new QGroupBox;
    QVBoxLayout *lockGLayout = new QVBoxLayout;
    QGroupBox *lockGGroup = new QGroupBox;

    lockT1 = new QPushButton("", this);
    lockT2 = new QPushButton("", this);
    lockV1 = new QPushButton("", this);
    lockV2 = new QPushButton("", this);
    lockG1 = new QPushButton("", this);
    lockG2 = new QPushButton("", this);
  /*  lockT1->setFixedSize(50, 50);
    lockT2->setFixedSize(50, 50);
    lockV1->setFixedSize(50, 50);
    lockV2->setFixedSize(50, 50);
    lockG1->setFixedSize(50, 50);
    lockG2->setFixedSize(50, 50);*/

    QPixmap lockleftmap("resources/icons/locklefticon32.png");
    lockleftIcon = new QIcon(lockleftmap);
    QPixmap lockrightmap("resrouces/icons/lockrighticon32.png");
    lockrightIcon = new QIcon(lockrightmap);
    QPixmap unlockleftmap("resources/icons/unlocklefticon32.png");
    unlockleftIcon = new QIcon(unlockleftmap);
    QPixmap unlockrightmap("resources/icons/unlockrighticon32.png");
    unlockrightIcon = new QIcon(unlockrightmap);

    lockV1->setIcon((* unlockleftIcon));
    lockV1->setIconSize(QSize(32, 32));
    lockV2->setIcon((* unlockrightIcon));
    lockV2->setIconSize(QSize(32, 32));
    lockT1->setIcon((* unlockleftIcon));
    lockT1->setIconSize(QSize(32, 32));
    lockT2->setIcon((* unlockrightIcon));
    lockT2->setIconSize(QSize(32, 32));
    lockG1->setIcon((* unlockleftIcon));
    lockG1->setIconSize(QSize(32, 32));
    lockG2->setIcon((* unlockrightIcon));
    lockG2->setIconSize(QSize(32, 32));

    lockTLayout->addWidget(lockT1);
    lockTLayout->addWidget(lockT2);
    lockTGroup->setLayout(lockTLayout);

    lockVLayout->addWidget(lockV1);
    lockVLayout->addWidget(lockV2);
    // lockVLayout->setAlignment(lockV1, Qt::AlignVCenter);
    // lockVLayout->setAlignment(lockV2, Qt::AlignVCenter);
    lockVGroup->setLayout(lockVLayout);

    lockGLayout->addWidget(lockG1);
    lockGLayout->addWidget(lockG2);
    lockGGroup->setLayout(lockGLayout);

    vizLayout->addWidget(lockVGroup, 1, 1);
    vizLayout->addWidget(lockGGroup, 3, 1);

    connect(lockT1, SIGNAL(clicked()), this, SLOT(lockTransectFromLeft()));
    connect(lockT2, SIGNAL(clicked()), this, SLOT(lockTransectFromRight()));
    connect(lockV1, SIGNAL(clicked()), this, SLOT(lockViewsFromLeft()));
    connect(lockV2, SIGNAL(clicked()), this, SLOT(lockViewsFromRight()));
    connect(lockG1, SIGNAL(clicked()), this, SLOT(lockTimelineFromLeft()));
    connect(lockG2, SIGNAL(clicked()), this, SLOT(lockTimelineFromRight()));

    vizPanel->setLayout(vizLayout);
    vizPanel->setStyleSheet("background-color:grey;");
    viewLock = LockState::UNLOCKED;
    transectLock = LockState::UNLOCKED;
    timelineLock = LockState::UNLOCKED;

    /*
    vizPanel->setStyleSheet(QString::fromUtf8("ChartWindow\n"
    "{\n"
    "     background-color: red;\n"
    "}\n"
    ""));*/
}

// if copydata = T, then compute when scene_idx = 0, and copy when it is 1, else compute for both cases
void Window::setupGraphModels(int scene_index, bool copyData)
{
    auto charts = TimelineGraph::getChartTypes();
    graphModels[scene_index].clear();

    // loop over all charts and extract the data for it
    for (auto c : charts) {
        TimelineGraph *tg;

        if (scene_index < 0 || scene_index > 1)
        {
            std::ostringstream oss;
            oss << "run-time error: setupGraphModels() - invalid scene selected";
            throw std::runtime_error(oss.str());
        }

        if (copyData == false || scene_index == 0)
        {
            tg = new TimelineGraph;
            tg->setTimeLine(scenes[scene_index]->getTimeline());
            tg->extractDataSeries(scenes[scene_index], c);
        }
        else // share precomputed data from scene 0
            tg = new TimelineGraph(*graphModels[0][int(c)]);

        graphModels[scene_index].push_back(tg);
    }
}

// PCM: add in constructor names for map overlay  - TBD
Window::Window(string datadir, string lprefix, string rprefix)
{
    QWidget *mainWidget = new QWidget;
    QGridLayout *mainLayout = new QGridLayout();


    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(4,1);
    fmt.setDepthBufferSize(24);
    // fmt.setOptions(QSurfaceFormat::DeprecatedFunctions);
    QSurfaceFormat::setDefaultFormat(fmt);
    basedir = datadir;
    prefix[0] = lprefix; prefix[1] = rprefix;

    // NOTE: to enable MSAA rendering into FBO requires some more work, fix then enable - else white screen.
    //glFormat.setSampleBuffers( true );
    //glFormat.setSamples(4);

    transectsValid = false;
    active = false;
    visible = true;

    dmapIdx[0] = dmapIdx[1] = 0;
    rampIdx[0] = rampIdx[1] = 0;
    grphIdx[0] = grphIdx[1] = 0;

   // overviewTimer.setSingleShot(true);
   // connect( &overviewTimer, SIGNAL(timeout()), SLOT(overviewShow()) );

    rendercount = 0;
    mainLayout->setSpacing(1);
    // mainLayout->setMargin(1);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(mainWidget);

    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 0);
    mainLayout->setColumnStretch(2, 0);
    mainLayout->setColumnStretch(3, 0);
    mainLayout->setColumnStretch(4, 1);

    setupRenderPanel();
    plantPanel = nullptr;
    setupPlantPanel();

    // mapDownSampleFactor = 4;

    // load scenes
    for(int i = 0; i < 2; i++)
    {
        Scene * s = new Scene(datadir, (i == 0 ? lprefix : rprefix) );
        scenes.push_back(s);
        mapScene * ms;
        // NB: we assume that L/R scene data is the same if the *base names* (prefixes) are the same!
        // always load left view fully if 1st allocation OR the windows load *different* scene data
        if (i == 0 || lprefix != rprefix)
            ms = new mapScene(datadir, mapOverlayFile[i], (i ==0 ? lprefix : rprefix) );
        else
            ms = new mapScene(*mapScenes[0]);
        mapScenes.push_back(ms);
        coredir[i] = datadir;
    }

    for(int i = 0; i < 2; i++)
    {
        Transect * t = new Transect(scenes[i]->getTerrain());
        transectControls.push_back(t);
    }
    setupVizPanel();
    dataMapPanel = nullptr;
    setupDataMapPanel();
    setupViewPanel();

    mainLayout->addWidget(renderPanel, 0, 0, Qt::AlignTop);
    mainLayout->addWidget(plantPanel, 0, 1, Qt::AlignTop);
    mainLayout->addWidget(dataMapPanel, 0, 2, Qt::AlignTop);
    mainLayout->addWidget(viewPanel, 0, 3, Qt::AlignTop);
    mainLayout->addWidget(vizPanel, 0, 4);

    createActions();
    createMenus();

    readMitsubaExportProfiles("resources/mitsuba/ModelSpecies");
    this->installEventFilter(this);

    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("EcoViz"));
    mainWidget->setMouseTracking(true);
    setMouseTracking(true);

    renderPanel->hide();
    plantPanel->hide();
    dataMapPanel->hide();
    viewPanel->hide();
}

Window::~Window()
{
    // delete transect controllers
    for(auto &it: transectControls)
        if (it != nullptr) delete it;

}

void Window::acquireTimeline(std::vector<int> & timestepIDs, std::string prefix)
{
    QDir directory(basedir.c_str());

    // search for files in base directory that obey the prefix and extract the ID
    QStringList files = directory.entryList(QStringList() << "*.pdb" << "*.PDB" << "*.pdbb" << "*.PDBB",
                                            QDir::Files | QDir::NoDot | QDir::NoDotDot);
    for(int i = 0; i < (int) files.size(); i++)
    {
        std::string filtername = files[i].toStdString();
        if(filtername.find(prefix) == 0)  // does file match prefix
        {
            // extract integer ID
            int endpos = filtername.find_first_of(".");
            int startpos = prefix.length();
            std::string id = filtername.substr(startpos, endpos-startpos);
            if(std::all_of(id.begin(), id.end(), ::isdigit))
                timestepIDs.push_back(std::stoi(id));
            else
                cerr << "Error Window::acquireTimeline : malformed input file " << filtername << endl;
        }
    }
    std::sort(timestepIDs.begin(), timestepIDs.end(), std::less<int>()); // sort in ascending order
    timestepIDs.erase( unique( timestepIDs.begin(), timestepIDs.end() ), timestepIDs.end() );// remove duplicates
}

void Window::run_viewer()
{
    int extractWindowDSample = 6;

    for(int i = 0; i < 2; i++)
    {
        // PCM: added overview map
        // load large scale terrain, downsample for oevrview map, and extract  default sub-region for
        // main render window.
std::cerr << " -- load overview (w. dsample)\n";
        // if left scene data is same as right scene data, we share - so load happens for left view only
        std::unique_ptr<Terrain> subTerr =
                mapScenes[i]->loadOverViewData(extractWindowDSample, ( (prefix[0]==prefix[1]) && i == 1) );
        // (1) set extracted sub-region as the region for this window
        // (2) pass in a pointer to highres (master) terrain
std::cerr << " -- set up terrain copy.\n";
        scenes[i]->setNewTerrainData(std::move(subTerr), mapScenes[i]->getHighResTerrain().get());
        vpPoint midPoint;
        scenes[i]->getTerrain()->getMidPoint(midPoint);
        scenes[i]->getTerrain()->setFocus(midPoint);
        // load in remaing eco-system data - NOTE:
        // the original source region extent is stored in scene[i] - this is later queried to
        // ensure only plants overlapping that region are correctly displayed (translated to the sub-region)
std::cerr << " -- acquire timeline.\n";
        std::vector<int> timelineIDs;
        acquireTimeline(timelineIDs, prefix[i]);
 std::cerr << " -- Load scene start: \n";
        scenes[i]->loadScene(timelineIDs,
                             (prefix[0]==prefix[1]), // if these match, assume we have IDENTICAL left/right cohort data!
                             (i==0?std::shared_ptr<CohortMaps>() : scenes[0]->getCohortMaps())); // if L/R cohorts are same, only allocate 1st call
std::cerr << " -- Load scene end.\n";
        cerr << "loading Data Maps" << endl;
        scenes[i]->loadDataMaps((int) timelineIDs.size());


        transectViews[i]->setScene(scenes[i]);
        perspectiveViews[i]->setScene(scenes[i]);
        //overviewMaps[i]->setSelectionRegion(mapScenes[i]->getSelectedRegion());
        perspectiveViews[i]->getOverviewWindow()->setSelectionRegion(mapScenes[i]->getSelectedRegion());
        timelineViews[i]->setScene(scenes[i]);
        transectViews[i]->setVisible(false);
        setupGraphModels(i, (prefix[0]==prefix[1]) ); // if left and right files are same, only compute series once
        chartViews[i]->setScene(scenes[i]);
        chartViews[i]->setGraphs(graphModels[i]);
        chartViews[i]->setXLabels(timelineIDs);
        chartViews[i]->setData(graphModels[i].front()); // set to first visualization
        transectControls[i]->init(scenes[i]->getTerrain());
    }

    setupPlantPanel();
    setupDataMapPanel();
    setupViewPanel();
    rendercount++;
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
    QOpenGLWidget *glwidget = qobject_cast<QOpenGLWidget *>(child);
    if(glwidget) {
        QMouseEvent *glevent=new QMouseEvent(event->type(),glwidget->mapFromGlobal(event->globalPos()),event->button(),event->buttons(),event->modifiers());
        QCoreApplication::postEvent(glwidget,glevent);
    }
}

void Window::repaintAllGL()
{
    // position overmaps on first active render
    /*
    cerr << perspectiveViews[0]->getPainted() << " " << perspectiveViews[1]->getPainted() << " " << overviewMaps[0]->getActive() << " " << overviewMaps[1]->getActive() << endl;
    if(!active)
    {
        if(perspectiveViews[0]->getPainted() && perspectiveViews[1]->getPainted() && overviewMaps[0]->getActive() && overviewMaps[1]->getActive())
        {
            active = true;
        }
    }*/
    // updateOverviews();

    rendercount = 0;
    for(auto pview: perspectiveViews)
        pview->repaint();
    for(auto tview: transectViews)
        tview->repaint();
    for(auto mview: timelineViews)
        mview->repaint();
    for(auto cview: chartViews)
        cview->repaint();
    // PCM: probbaly not needed mostly...
    //for (auto mapviews: overviewMaps)
    //    if (mapviews != nullptr) mapviews->repaint();
}

void Window::saveSceneView(int i)
{
    viewScene scnview(perspectiveViews[i]->getMapRegion(), (* perspectiveViews[i]->getView()));

    // get data directory
    QString qFileName = QFileDialog::getSaveFileName(this, tr("Save View"), coredir[i].c_str(), tr("View Files (*.vew)"));
    // add file name to list
    // remember to auto-populate views
    scnview.save(qFileName.toStdString());
    cerr << endl << "SCENE VIEW SAVED " << i << endl;
}

void Window::loadSceneView(int i)
{
    Region region;
    View view;
    viewScene scnview;

    // get data directory

    QString qFileName = QFileDialog::getOpenFileName(this, tr("Open View"), coredir[i].c_str(), tr("View Files (*.vew)"));
    scnview.load(qFileName.toStdString());
    cerr << endl << "SCENE VIEW LOADED " << i << endl;

    region = scnview.getRegion();
    view = scnview.getView();
    extractNewSubTerrain(i, region.x0, region.y0, region.x1, region.y1);
    perspectiveViews[i]->setView(view);
    repaintAllGL();
}


// ISSUES: 1) this may not remove/add transect buttons correctly
//         2) this does not touch the chart/timeline (these were built with  original sub-terrain and stats are for that -
//            note that the Timeline should br OK since it uses full terrain data, but chart is based on original terrain)

void Window::extractNewSubTerrain(int i, int x0, int y0, int x1, int y1)
{
    active = false;

    // clear transects and widgets
    for(int j = 0; j < 2; j++)
    {
        if(i == 2 || i == j) // one (i == 0 or i == 1) or both (i == 3) perspective views
        {
            destroyVizTransect(j);

            // destroy perspective views and  associated widgets
            View *oldView = new View(*perspectiveViews[j]->getView());
            bool oldLock = perspectiveViews[j]->getViewLockState();

            destroyVizPerspective(j);

            QSurfaceFormat glFormat;
            glFormat.setProfile( QSurfaceFormat::CoreProfile );
            glFormat.setDepthBufferSize(24);
            glFormat.setVersion(4,1);
            // glFormat.setOptions(QSurfaceFormat::DeprecatedFunctions);

            /*
            glFormat.setVersion( 4, 1 );
            // fmt.setSamples(16);
            // fmt.setDepthBufferSize(24);
            glFormat.setProfile( QSurfaceFormat::CoreProfile );*/

            // get current region (which should have changed from before)
            Region newReg = Region(x0,y0,x1,y1);

            mapScenes[j]->setSelectedRegion(newReg);
            // (0) add check to see if this Region has changed (else wasteful) TBD ...PCM
            std::unique_ptr<Terrain> subTerr = mapScenes[j]->extractTerrainSubwindow(newReg);

            // (1) set extracted sub-region as the region for this window
            // (2) pass in a pointer to highres (master) terrain
            scenes[j]->setNewTerrainData(std::move(subTerr), mapScenes[j]->getHighResTerrain().get());
            vpPoint midPoint;
            scenes[j]->getTerrain()->getMidPoint(midPoint);
            scenes[j]->getTerrain()->setFocus(midPoint);
            // cerr << "MIDPOINT = " << midPoint.x << ", " << midPoint.y << ", " << midPoint.z << endl;

            // rebuild transect control structure
            assert(transectControls.size() == 2);
            assert(transectControls[j] == nullptr);
            Transect * t = new Transect(scenes[j]->getTerrain());
            transectControls[j] = t;

            // rebuild transect widget
            setupVizTransect(glFormat, j);
            transectViews[j]->setVisible(false);
            transectControls[j]->init(scenes[j]->getTerrain());

            // rebuild perspective widget
            setupVizPerspective(glFormat, j);
            perspectiveViews[j]->setScene(scenes[j]);
            perspectiveViews[j]->getOverviewWindow()->setSelectionRegion(mapScenes[j]->getSelectedRegion());
            // required to preserve View Mx, will cause issues if left/reight perspective views are 'locked'
            // NOTE: the glWidget never frees 'view' and thus leaks memory. But changing this would require a
            // lot more code (should store View not View * and make copies - it's a lightweight object, but embedded
            // in many places, so lot of rewriting.
            perspectiveViews[j]->lockView(oldView);
            perspectiveViews[j]->getView()->setForcedFocus(midPoint); // otherwise focus change is not copied from terrain

            // (3) for new terrain (aftr initial terrain) we need to ensure thats buffer size changes are accounted for.
            scenes[j]->getTerrain()->setBufferToDirty();
            mapScenes[j]->getLowResTerrain()->setBufferToDirty();

            perspectiveViews[j]->rebindPlants();

            // restablish broken connection from timeline widget signals
            //connect(timelineViews[i], SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
            //connect(timelineViews[i], SIGNAL(signalRebindPlants()), chartViews[i], SLOT(updateTimeBar()));
            //connect(timelineViews[i], SIGNAL(signalSync(int)), this, SLOT(timelineSync(int)));
            connect(timelineViews[j], SIGNAL(signalRebindPlants()), perspectiveViews[j], SLOT(rebindPlants()));
            connect(timelineViews[j], SIGNAL(signalRebindPlants()), transectViews[j], SLOT(rebindPlants()));

            //overviewMaps[i]->setSelectionRegion(mapScenes[i]->getSelectedRegion());
            //overviewMaps[i]->updateViewParams();
            //overviewMaps[i]->forceUpdate();

            rendercount++;
            perspectiveViews[j]->setDataMap(dmapIdx[j], convertRampIdx(i), false); // note that update is deferred until texture has been initialized
            perspectiveViews[j]->setViewLockState(oldLock);
        }
        // PCM: + signal to clear transect!!!

    }

    // if the views are locked then resync
    if(viewLock == LockState::LOCKEDFROMLEFT)
        perspectiveViews[1]->lockView(perspectiveViews[0]->getView());
    if(viewLock == LockState::LOCKEDFROMRIGHT)
        perspectiveViews[0]->lockView(perspectiveViews[1]->getView());

    repaintAllGL();
}


void Window::showRenderOptions()
{
    renderPanel->setVisible(showRenderAct->isChecked());
}

void Window::showPlantOptions()
{
    plantPanel->setVisible(showPlantAct->isChecked());
}

void Window::showDataMapOptions()
{
    dataMapPanel->setVisible(showDataMapAct->isChecked());
}

void Window::showViewOptions()
{
    viewPanel->setVisible(showViewAct->isChecked());
}

void Window::unlockViews()
{
    cerr << "((((((( UNLOCK VIEWS ))))))))" << endl;
    if((int) perspectiveViews.size() == 2)
    {
        perspectiveViews[0]->unlockView();
        for(auto &p: perspectiveViews)
            p->setViewLockState(false);
        lockV1->setIcon((* unlockleftIcon));
        lockV2->setIcon((* unlockrightIcon));
    }
    else
    {
        cerr << "Error Window::unlockViews: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockViewsFromLeft()
{
    if((int) perspectiveViews.size() == 2)
    {
        if(viewLock == LockState::LOCKEDFROMLEFT)
        {
            unlockViews();
            viewLock = LockState::UNLOCKED;
        }
        else
        {
            if(viewLock == LockState::LOCKEDFROMRIGHT) // need to unlock first
                unlockViews();
            for(auto &p: perspectiveViews)
                p->setViewLockState(true);
            perspectiveViews[1]->lockMap(perspectiveViews[0]->getMapRegion());
            perspectiveViews[1]->lockView(perspectiveViews[0]->getView()); // Do not re-order this and previous line
            viewLock = LockState::LOCKEDFROMLEFT;
            lockV1->setIcon((* lockleftIcon));

        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockViewFromLeft: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockViewsFromRight()
{
    if((int) perspectiveViews.size() == 2)
    {
        if(viewLock == LockState::LOCKEDFROMRIGHT)
        {
            unlockViews();
            viewLock = LockState::UNLOCKED;
        }
        else
        {
            if(viewLock == LockState::LOCKEDFROMLEFT) // need to unlock first
                unlockViews();
            for(auto &p: perspectiveViews)
                p->setViewLockState(true);
            perspectiveViews[0]->lockMap(perspectiveViews[1]->getMapRegion());
            perspectiveViews[0]->lockView(perspectiveViews[1]->getView()); // Do not re-order this and previous line
            viewLock = LockState::LOCKEDFROMRIGHT;
            lockV2->setIcon((* lockrightIcon));

        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockViewFromRight: single panel so unlocking is not possible" << endl;
    }
}

// for transect locking, the same DEM must be used in both left/right views and the selection boxes must be identical
bool Window::canLockTransects(void) const
{
    bool canLock = true;
    Region left = mapScenes[0]->getSelectedRegion();
    Region right = mapScenes[1]->getSelectedRegion();
    if (left.x0 != right.x0 || left.x1 != right.x1 || left.y0 != right.y0 || left.y1 != right.y1)
        return false;
    if (mapScenes[0]->getBaseName() != mapScenes[1]->getBaseName())
        return false;
    return canLock;
}

void Window::unlockTransects()
{
    if((int) transectViews.size() == 2)
    { 
        for(auto &t: transectViews)
            t->setViewLockState(false);

        Transect * pretrx = transectControls[0];
        transectControls[0] = new Transect(scenes[0]->getTerrain());
        basic_types::MapFloat * premapviz = transectControls[0]->getTransectMap();
        (* transectControls[0]) = (* pretrx);
        transectControls[0]->setTransectMap(premapviz);
        transectViews[0]->unlockView(transectControls[0]);

        perspectiveViews[0]->seperateTransectCreate(transectControls[0]);
        transectLock = LockState::UNLOCKED;
        lockT1->setIcon((* unlockleftIcon));
        lockT2->setIcon((* unlockrightIcon));
    }
    else
    {
        cerr << "Error Window::unlockTransects: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockTransectFromLeft()
{
    cerr << "LockTransectFromLeft" << endl;

    if (canLockTransects() == false)
    {
        cerr << "can't lock transects - the selection boxes and or DEMs differ\n";
        return;
    }

    if((int) transectViews.size() == 2)
    {
        if(transectLock == LockState::LOCKEDFROMLEFT)
        {
             unlockTransects();
        }
        else
        {
            if(transectLock == LockState::LOCKEDFROMRIGHT) // need to unlock first
                unlockTransects();

            for(auto &t: transectViews)
                t->setViewLockState(true);
            delete transectControls[1];
            transectControls[1] = transectControls[0];
            perspectiveViews[1]->setTransectCreate(perspectiveViews[0]->getTransectCreate());
            transectViews[1]->lockView(transectViews[0]->getView(), transectViews[0]->getTransect());
            transectLock = LockState::LOCKEDFROMLEFT;
            lockT1->setIcon((* lockleftIcon));
            showTransectViews();
        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockTransectFromLeft: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockTransectFromRight()
{
    cerr << "LockTransectFromRight" << endl;

    if (canLockTransects() == false)
    {
        cerr << "can't lock transects - the selection boxes and or DEMs differ\n";
        return;
    }

    if((int) transectViews.size() == 2)
    {
        if(transectLock == LockState::LOCKEDFROMRIGHT)
        {
             unlockTransects();
        }
        else
        {
            if(transectLock == LockState::LOCKEDFROMLEFT) // need to unlock first
                unlockTransects();

            for(auto &t: transectViews)
                t->setViewLockState(true);
            delete transectControls[0];
            transectControls[0] = transectControls[1];

            perspectiveViews[0]->setTransectCreate(perspectiveViews[1]->getTransectCreate());
            transectViews[0]->lockView(transectViews[1]->getView(),  transectViews[1]->getTransect());
            transectLock = LockState::LOCKEDFROMRIGHT;
            lockT2->setIcon((* lockrightIcon));
            showTransectViews();
        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockTransectFromRight: single panel so unlocking is not possible" << endl;
    }
}

void Window::transectSyncPlace(bool firstPoint)
{
    if(transectLock != LockState::UNLOCKED)
    {
        if(sender() == perspectiveViews[0])
            perspectiveViews[1]->pointPlaceTransect(firstPoint);
        else
            perspectiveViews[0]->pointPlaceTransect(firstPoint);
    }
}

void Window::unlockTimelines()
{
    if((int) timelineViews.size() == 2)
    {
        for(auto &t: timelineViews)
            t->setViewLockState(false);
        timelineLock = LockState::UNLOCKED;
        lockG1->setIcon((* unlockleftIcon));
        lockG2->setIcon((* unlockrightIcon));
    }
    else
    {
        cerr << "Error Window::unlockTimelines: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockTimelineFromLeft()
{
    if((int) timelineViews.size() == 2)
    {
        if(timelineLock == LockState::LOCKEDFROMLEFT)
        {
            unlockTimelines();
        }
        else
        {
            if(timelineLock == LockState::LOCKEDFROMRIGHT)
                unlockTimelines();
            for(auto &t: timelineViews)
                t->setViewLockState(true);
            timelineLock = LockState::LOCKEDFROMLEFT;
            lockG1->setIcon((* lockleftIcon));
            timelineViews[1]->synchronize(timelineViews[0]->get_sliderval());
        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockTimelineFromLeft: single panel so unlocking is not possible" << endl;
    }
}

void Window::lockTimelineFromRight()
{
    if((int) timelineViews.size() == 2)
    {
        if(timelineLock == LockState::LOCKEDFROMRIGHT)
        {
             unlockTimelines();
        }
        else
        {
            if(timelineLock == LockState::LOCKEDFROMLEFT) // need to unlock first
                unlockTimelines();

            for(auto &t: timelineViews)
                t->setViewLockState(true);
            timelineLock = LockState::LOCKEDFROMRIGHT;
            lockG2->setIcon((* lockrightIcon));
            timelineViews[0]->synchronize(timelineViews[1]->get_sliderval());
        }

        rendercount++;
        repaintAllGL();
    }
    else
    {
        cerr << "Error Window::lockTimelineFromRight: single panel so unlocking is not possible" << endl;
    }
}

void Window::timelineSync(int t)
{
    if(timelineLock != LockState::UNLOCKED)
    {
        if(sender() == timelineViews[0])
            timelineViews[1]->synchronize(t);
        else
            timelineViews[0]->synchronize(t);
    }
}

void Window::showContours(int show)
{
    for(auto pview: perspectiveViews)
        pview->getRenderer()->drawContours(show == Qt::Checked);
    rendercount++;
    repaintAllGL();
}

void Window::showTransectViews()
{
    if (transectControls.size() == 0 || transectViews.size() == 0)
    {
        std::cerr << "\nWindow::showTransectViews - no transect data defined?";
        return;
    }

    bool transectNowValid = transectControls[0]->getValidFlag() || transectControls[1]->getValidFlag();
    if(transectNowValid && !transectsValid)
    {
        vizLayout->setRowStretch(0, 6);
        for(int i = 0; i < 2; i++)
            vizLayout->addWidget(transectViews[i], 0, i*2);
        vizLayout->addWidget(lockTGroup, 0, 1);
        transectsValid = true;
    }
    for(int i = 0; i < 2; i++)
    {
        transectViews[i]->setVisible(transectControls[i]->getValidFlag());
        transectViews[i]->setActive(transectControls[i]->getValidFlag());
    }
    rendercount++;
    repaintAllGL();
}

void Window::clearTransects()
{
    if (transectControls.size() == 0 || transectViews.size() == 0)
    {
        std::cerr << "\nWWindow::clearTransects - no transect data defined?";
        return;
    }

    // change transects back to initial state
    for(int i = 0; i < 2; i++)
    {
        transectControls[i]->reset();
        perspectiveViews[i]->resetTransectState();
    }

    vizLayout->setRowStretch(0, 0);
    for(int i = 0; i < 2; i++)
    {
        transectViews[i]->setVisible(false);
        transectViews[i]->setActive(false);
        vizLayout->removeWidget(transectViews[i]);
    }
    vizLayout->removeWidget(lockTGroup);
    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    resize( sizeHint() );
    transectsValid = false;
}

void Window::leftMinimapToggle(int status)
{
    perspectiveViews[0]->setMinimapVisible(status == Qt::Checked);
    rendercount++;
    repaintAllGL();
}

void Window::rightMinimapToggle(int status)
{
    perspectiveViews[1]->setMinimapVisible(status == Qt::Checked);
    rendercount++;
    repaintAllGL();
}

void Window::showGridLines(int show)
{
    for(auto pview: perspectiveViews)
        pview->getRenderer()->drawGridlines(show == Qt::Checked);
    rendercount++;
    repaintAllGL();
}

void Window::allPlantsOn()
{
    for(auto pview: perspectiveViews)
        pview->setAllSpecies(true);

}

void Window::allPlantsOff()
{
    for(auto pview: perspectiveViews)
        pview->setAllSpecies(false);
}

void Window::uncheckPlantPanel()
{
    showPlantAct->setChecked(false);
}

void Window::plantChange(int show)
{
    bool vis = (bool) show;

    QVariant snum = sender()->property("species_num");

    auto &sdata = scenes[0]->getBiome()->getSpeciesMetaData();
    if (!snum.isValid())
    {
        cerr << "Click plant on/off: invalid button!";
        return;
    }

    for(auto pview: perspectiveViews)
    {
        pview->toggleSpecies(snum.toInt(), vis);
        cerr << "Click plant on/off: toggle " << sdata[snum.toInt()].scientific_name << "to " << vis;
    }

}

TypeMapType Window::convertRampIdx(int side)
{
    if(dmapIdx[side] == 0)
    {
        return TypeMapType::TRANSECT;
    }
    else
    {
        switch(rampIdx[side])
        {
        case 0: return TypeMapType::GREYRAMP;
            break;
        case 1: return TypeMapType::HEATRAMP;
            break;
        case 2: return TypeMapType::BLUERAMP;
            break;
        default: return TypeMapType::GREYRAMP;
            break;
        }
    }
}

void Window::leftDataMapChoice(int id)
{
    cerr << "leftDataMapChoice" << endl;
    dmapIdx[0] = id;
    perspectiveViews[0]->setDataMap(dmapIdx[0], convertRampIdx(0), true);
}

void Window::rightDataMapChoice(int id)
{
     dmapIdx[1] = id;
     perspectiveViews[1]->setDataMap(dmapIdx[1], convertRampIdx(1), true);
}

void Window::leftGraphChoice(int id)
{
    grphIdx[0] = id;
    chartViews[0]->chartSelected(id);
}

void Window::rightGraphChoice(int id)
{
     grphIdx[1] = id;
     chartViews[1]->chartSelected(id);
}

void Window::leftRampChoice(int id)
{
     rampIdx[0] = id;
     perspectiveViews[0]->setDataMap(dmapIdx[0], convertRampIdx(0), true);
}

void Window::rightRampChoice(int id)
{
     rampIdx[1] = id;
     perspectiveViews[1]->setDataMap(dmapIdx[1], convertRampIdx(1), true);
}

void Window::uncheckDataMapPanel()
{
    showDataMapAct->setChecked(false);
}

void Window::syncDataMapPanel()
{
    // check state in perspective views and update accordingly
    for(int i = 0; i < 2; i++)
        if(perspectiveViews[i]->getOverlay() == TypeMapType::EMPTY || perspectiveViews[i]->getOverlay() == TypeMapType::TRANSECT)
        {
            dmapIdx[i] = 0;
            // update check mark
            if(i == 0)
                qbmgL->button(0)->setChecked(true);
            else
                qbmgR->button(0)->setChecked(true);
        }
}

void Window::uncheckViewPanel()
{
    showViewAct->setChecked(false);
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
    rendercount++;
    repaintAllGL();
}

void Window::mapChange(bool on)
{
    for(auto pview: perspectiveViews)
    {
        /*
        if(sunMapRadio->isChecked() && on)
            pview->setMap(TypeMapType::SUNLIGHT, sunMonth-1);
        if(wetMapRadio->isChecked() && on)
            pview->setMap(TypeMapType::WATER, wetMonth-1);
        if(chmMapRadio->isChecked() && on)
            pview->setOverlay(TypeMapType::CHM);
        if(noMapRadio->isChecked() && on)
            pview->setOverlay(TypeMapType::EMPTY);
        */
    }
}

void Window::cameraChange(int idx)
{
    for(auto pview: perspectiveViews)
    {
        if(idx == 0)
            pview->changeViewMode(ViewMode::ARCBALL);
        else
            pview->changeViewMode(ViewMode::FLY);
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

    showDataMapAct = new QAction(tr("Show DataMap Options"), this);
    showDataMapAct->setCheckable(true);
    showDataMapAct->setChecked(false);
    showDataMapAct->setStatusTip(tr("Hide/Show Data Map Options"));
    connect(showDataMapAct, SIGNAL(triggered()), this, SLOT(showDataMapOptions()));

    clearTransectsAct = new QAction(tr("Clear Transects"), this);
    clearTransectsAct->setCheckable(false);
    clearTransectsAct->setStatusTip(tr("Remove Transects"));
    connect(clearTransectsAct, SIGNAL(triggered()), this, SLOT(clearTransects()));

    showViewAct = new QAction(tr("Show View Controls"), this);
    showViewAct->setCheckable(true);
    showViewAct->setChecked(false);
    showViewAct->setStatusTip(tr("Hide/Show View Controls"));
    connect(showViewAct, SIGNAL(triggered()), this, SLOT(showViewOptions()));

    // Export Mitsuba
    exportMitsubaAct = new QAction(tr("Export Mitsuba"), this);
    connect(exportMitsubaAct, SIGNAL(triggered()), this, SLOT(exportMitsubaJSON()));
}

void Window::createMenus()
{
    // Export Mitsuba
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exportMitsubaAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showRenderAct);
    viewMenu->addAction(showPlantAct);
    viewMenu->addAction(showDataMapAct);
    viewMenu->addAction(showViewAct);
    viewMenu->addAction(clearTransectsAct);
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

    if (profilesDir.exists())
    {
        QStringList csvProfiles = profilesDir.entryList(QStringList() << "*.csv" << "*.CSV", QDir::Files);
        for (QString csvName : csvProfiles)
        {
            ifstream csvFile(profilesDirPath + "/" + csvName.toUtf8().data());

            string profileName = csvName.remove(".csv").toUtf8().data();
            string line;
            string plantCode;
            string maxHeightStr;
            double maxHeight;
            string instanceId;
            string actualHeightStr;
            double actualHeight;
            int count = 0;

            while (getline(csvFile, line))
            {
                count++;

                if (line.find("plant code;") == 0)
                {
                    // Headers line
                    continue;
                }

                if (line.empty() || line.find_first_not_of(" ") == string::npos)
                {
                    continue;
                }

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

                int (*fn_isspace)(int) = std::isspace;  // required because std::isspace is overloaded,
                                                        // template argument deduction for std::isspace fails

                maxHeightStr.erase(remove_if(maxHeightStr.begin(), maxHeightStr.end(), fn_isspace), maxHeightStr.end());
                char* end = nullptr;
                maxHeight = strtod(maxHeightStr.c_str(), &end);
                if (end == maxHeightStr.c_str() || *end != '\0' || maxHeight == HUGE_VAL)
                {
                    maxHeight = -1.0;
                    cerr << "Error in Window::readMitsubaExportProfiles : in the profile [" << profileName << "], line [" << count << "] max height could not be converted to double. Max height was automatically set to -1.0 !" << endl;
                }
                else if (maxHeight < 0.0)
                {
                    cerr << "Warning in Window::readMitsubaExportProfiles : in the profile [" << profileName << "], line [" << count << "] max height is a negative value" << endl;
                }

                // Instance id
                pos = line.find(delimiter);
                token = line.substr(0, pos);
                instanceId = token;
                line.erase(0, pos + delimiter.length());

                // Actual height
                actualHeightStr = line;

                actualHeightStr.erase(remove_if(actualHeightStr.begin(), actualHeightStr.end(), fn_isspace), actualHeightStr.end());
                end = nullptr;
                actualHeight = strtod(actualHeightStr.c_str(), &end);
                if (end == actualHeightStr.c_str() || *end != '\0' || actualHeight == HUGE_VAL)
                {
                    actualHeight = -1.0;
                    cerr << "Error in Window::readMitsubaExportProfiles : in the profile [" << profileName << "], line [" << count << "] actual height could not be converted to double. Actual height was automatically set to -1.0 !" << endl;
                }
                else if (actualHeight < 0.0)
                {
                    cerr << "Warning in Window::readMitsubaExportProfiles : in the profile [" << profileName << "], line [" << count << "] actual height is a negative value" << endl;
                }

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
                itPlantCode->second.push_back({ maxHeight, instanceId, actualHeight });
            }
        }

        cout << "readMitsubaExportProfiles finished !" << endl;
    }
}

/*
void Window::closeEvent(QCloseEvent* event)
{
    for(auto &it: overviewMaps) // floating so need to be closed separately
        it->close();
    event->accept();
}


void Window::updateOverviews()
{
    bool firstactivation = false;

    if(perspectiveViews[0]->getPainted() && perspectiveViews[1]->getPainted() && overviewMaps[0]->getActive() && overviewMaps[1]->getActive())
    {
         firstactivation = !active; // will need to call paint on first activation
         active = true;
    }

    // cerr << perspectiveViews[0]->getPainted() << " " << perspectiveViews[1]->getPainted() << " " << overviewMaps[0]->getActive() << " " << overviewMaps[1]->getActive() << " " << visible << endl;
    if(active && visible) // show overviews
    {
        for(int i = 0; i < 2; i++)
            positionVizOverMap(i);
    }
    else // hide overview
    {
        for(auto &it: overviewMaps)
            if (it != nullptr) it->hide();
    }
    if(firstactivation)
        repaintAllGL();
}

void Window::overviewShow()
{
    visible = true;
    updateOverviews();
}
*/

/*
void Window::resizeEvent(QResizeEvent* event)
{
    if(active)
    {
        visible = false;
        for(auto &pview: perspectiveViews)
            pview->setUpdatesEnabled(false);
        overviewTimer.start(300);
        updateOverviews();
    }
    event->accept();
}


void Window::moveEvent(QMoveEvent* event)
{
    if(active)
    {
        visible = false;
        for(auto &pview: perspectiveViews)
            pview->setUpdatesEnabled(false);
        overviewTimer.start(300);
        updateOverviews();
    }
    event->accept();
}
*/

/*
bool Window::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type event_type = event->type();

    if(event_type == QEvent::NonClientAreaMouseButtonPress)
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (pMouseEvent->button() == Qt::MouseButton::LeftButton)
        {
            visible = false;
            for(auto &pview: perspectiveViews)
                pview->setUpdatesEnabled(false);
            updateOverviews();
        }
    }
    else if(event_type == QEvent::NonClientAreaMouseButtonRelease) // || event_type == QEvent::MouseButtonRelease)
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (pMouseEvent->button() == Qt::MouseButton::LeftButton)
        {
            visible = true;
            for(auto &pview: perspectiveViews)
                pview->setUpdatesEnabled(true);
            updateOverviews();
        }
    }
    return QObject::eventFilter(obj, event);
}
*/

void Window::exportMitsuba()
{
	std::cout << "Obsolete function exportMitsuba() called" << std::endl;
  /*
    QStringList allProfiles;

    map<string, map<string, vector<MitsubaModel>>>::iterator it;
    for (it = this->profileToSpeciesMap.begin(); it != this->profileToSpeciesMap.end(); it++)
    {
        allProfiles.append(it->first.data());
    }

    if (allProfiles.isEmpty())
    {
        QMessageBox messageBox;
        messageBox.warning(this, "No profile found", "No export profile was found.\nPlease check that you have created at least one profile in the folder \"data/mitsubaExportProfiles\"");
        return;
    }

    bool ok = false;
    ExportSettings exportSettings = ExportDialog::getExportSettings(this, allProfiles, ok);

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
    }*/
}


void Window::exportMitsubaJSON()
{
  QStringList allProfiles;

  map<string, map<string, vector<MitsubaModel>>>::iterator it;
  for (it = this->profileToSpeciesMap.begin(); it != this->profileToSpeciesMap.end(); it++)
  {
    allProfiles.append(it->first.data());
  }

  if (allProfiles.isEmpty())
  {
    QMessageBox messageBox;
    messageBox.warning(this, "No profile found", "No export profile was found.\nPlease check that you have created at least one profile in the folder \"data/mitsubaExportProfiles\"");
    return;
  }

  bool ok = false;
  ExportSettings exportSettings = ExportDialog::getExportSettings(this, allProfiles, ok);

  if (ok)
  {
    cout << "Export started !" << endl;

		string jsonDirPath = exportSettings.path;

    //QDir().mkdir(QString::fromStdString(jsonDirPath) + "/Terrain");
    //QDir().mkdir(QString::fromStdString(jsonDirPath) + "/Instances");

		map<string, vector<MitsubaModel>> speciesMap = this->profileToSpeciesMap.find(exportSettings.profile)->second;


    if (exportSettings.sceneLeft)
		{
			const int left = 0;

			// Data
			Scene* sceneLeft = this->scenes[left];
      Transect* transectLeft = this->transectControls[left];

			// Export Cameras JSON
			cout << "- Export camera left" << endl;
			this->perspectiveViews[left]->getView()->exportCameraJSON(jsonDirPath + "/Cameras/", exportSettings.filenameLeft + "_cameraLeft");
          
			// Export Terrain JSON
			cout << "- Export terrain left" << endl;
			sceneLeft->exportTerrainJSON( jsonDirPath + "/Terrain/", exportSettings.filenameLeft + "_terrainLeft");

			// Export Instances
			cout << "- Export vegetation instances" << endl;
			sceneLeft->exportInstancesJSON(speciesMap, jsonDirPath + "/Instances/", exportSettings.filenameLeft + "_instancesLeft", sceneLeft, transectLeft);

      // Export Scene JSON 
			cout << "- Export scene left" << endl;
			sceneLeft->exportSceneJSON(jsonDirPath, exportSettings.filenameLeft + "_cameraLeft", "Lights", exportSettings.filenameLeft + "_terrainLeft", exportSettings.filenameLeft + "_instancesLeft",
        exportSettings.filenameLeft, exportSettings.resolutionW, exportSettings.resolutionH, exportSettings.samples, exportSettings.threads);
		}

		if (exportSettings.sceneRight)
		{
      const int right = 1;
      // Data
			Scene* sceneRight = this->scenes[right];
			Transect* transectRight = this->transectControls[right];

      // Export Cameras JSON
			cout << "- Export camera right" << endl;
			this->perspectiveViews[right]->getView()->exportCameraJSON(jsonDirPath + "/Cameras/", exportSettings.filenameRight + "_cameraRight");

			// Export Terrain JSON
			cout << "- Export terrain right" << endl;
			sceneRight->exportTerrainJSON(jsonDirPath + "/Terrain/", exportSettings.filenameRight + "_terrainRight");

      // Export Instances
      cout << "- Export vegetation instances" << endl;
      sceneRight->exportInstancesJSON(speciesMap, jsonDirPath + "/Instances/", exportSettings.filenameRight+"_instancesRight", sceneRight, transectRight);

			// Export Scene JSON
			cout << "- Export scene right" << endl;
			sceneRight->exportSceneJSON(jsonDirPath, exportSettings.filenameRight + "_cameraRight", "Lights", exportSettings.filenameRight + "_terrainRight", exportSettings.filenameRight + "_instancesRight",
				exportSettings.filenameRight, exportSettings.resolutionW, exportSettings.resolutionH, exportSettings.samples, exportSettings.threads);

		}
    cout << "Export finished !" << endl;
  }
}
