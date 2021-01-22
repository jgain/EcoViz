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

/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems
 * Copyright (C) 2020 J.E. Gain (jgain@cs.uct.ac.za) and K.P. Kapp  (konrad.p.kapp@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/

#include "ConfigReader.h"
#include "glwidget.h"
#include "window.h"
#include "vecpnt.h"
#include "common/str.h"
#include "specselect_window.h"
#include "convertpaintingdialog.h"

#include <cuda_runtime.h>

#include <cmath>
#include <string>
#include <functional>

#include <QProgressBar>
#include <QImage>

using namespace std;

////
// SunWindow
///


QSize SunWindow::sizeHint() const
{
    return QSize(800, 800);
}

void SunWindow::setOrthoView(GLWidget * ortho)
{
    QWidget *mainWidget = new QWidget;
    QGridLayout *mainLayout = new QGridLayout;

    orthoView = ortho;

    setCentralWidget(mainWidget);
    mainLayout->setColumnStretch(0, 1);

    // signal to slot connections
    connect(orthoView->getGLSun(), SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));

    mainLayout->addWidget(orthoView->getGLSun(), 0, 0);
    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("EcoSun"));
}

void SunWindow::repaintAllGL()
{
    orthoView->getGLSun()->repaint();
}

////
// Window
///


void Window::showLoadLandscapeDialog()
{
    QMessageBox mbox;
    mbox.setText("Scene not loaded yet. Load one first at File -> Load Scene");
    mbox.exec();
}

bool Window::checkAndLoadLandscapeDialog()
{
    if (!perspectiveView->hasSceneLoaded())
    {
        showLoadLandscapeDialog();
        return false;
    }
    return true;
}


QSize Window::sizeHint() const
{
    return QSize(1000, 800);
}

Window::Window(int scale_size)
{
    mainWidget = new QWidget;
    mainLayout = new QGridLayout;
    int dx, dy;
    float sx, sy;

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

    setCentralWidget(mainWidget);
    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 0);

    // render panel
    renderPanel = new QWidget;
    renderLayout = new QVBoxLayout;

    // Grid Line Widgets
    QGroupBox *gridGroup = new QGroupBox(tr("Grid Lines"));
    QCheckBox * checkGridLines = new QCheckBox(tr("Show Grid Lines"));
    checkGridLines->setChecked(false);
    QLabel *gridSepXLabel = new QLabel(tr("Grid Sep X:"));
    gridSepXEdit = new QLineEdit;
    // gridSepXEdit->setValidator(new QDoubleValidator(0.0, 500000.0, 2, gridSepXEdit));
    gridSepXEdit->setInputMask("0000.0");
    QLabel *gridSepZLabel = new QLabel(tr("Grid Sep Z:"));
    gridSepZEdit = new QLineEdit;
    // gridSepZEdit->setValidator(new QDoubleValidator(0.0, 500000.0, 2, gridSepZEdit));
    gridSepZEdit->setInputMask("0000.0");
    QLabel *gridWidthLabel = new QLabel(tr("Grid Line Width:"));
    gridWidthEdit = new QLineEdit;
    // gridWidthEdit->setValidator(new QDoubleValidator(0.0, 10.0, 2, gridWidthEdit));
    gridWidthEdit->setInputMask("0.0");
    QLabel *gridIntensityLabel = new QLabel(tr("Grid Intensity:"));
    gridIntensityEdit = new QLineEdit;
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
    //contourSepEdit->setValidator(new QDoubleValidator(0.0, 10000.0, 2, contourSepEdit));
    contourSepEdit->setInputMask("000.0");
    QLabel *contourWidthLabel = new QLabel(tr("Contour Line Width:"));
    contourWidthEdit = new QLineEdit;
    // contourWidthEdit->setValidator(new QDoubleValidator(0.0, 10.0, 2, contourWidthEdit));
    contourWidthEdit->setInputMask("0.0");
    QLabel *contourIntensityLabel = new QLabel(tr("Contour Intensity:"));
    contourIntensityEdit = new QLineEdit;
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
    radianceEnhanceEdit->setInputMask("0.0");

    // set initial radiance values
    radianceEnhanceEdit->setText(QString::number(radianceEnhance, 'g', 2));

    QGridLayout *radianceLayout = new QGridLayout;
    radianceLayout->addWidget(radianceEnhanceLabel, 0, 0);
    radianceLayout->addWidget(radianceEnhanceEdit, 0, 1);
    radianceGroup->setLayout(radianceLayout);

    renderLayout->addWidget(gridGroup);
    renderLayout->addWidget(contourGroup);
    renderLayout->addWidget(radianceGroup);

    // OpenGL widget
    // Specify an OpenGL 3.2 format.

    QGLFormat glFormat;
    glFormat.setVersion( 4, 1 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( false );

    perspectiveView = new GLWidget(glFormat, scale_size);
    getView().setForcedFocus(getTerrain().getFocus());
    getView().setViewScale(getTerrain().longEdgeDist());

    getTerrain().getGridDim(dx, dy);
    getTerrain().getTerrainDim(sx, sy);

    perspectiveView->getGLSun()->setScene(&getTerrain(), NULL, NULL);

    std::cerr << "done" << std::endl;

    numGridX = 1.0f / gridSepX;
    numGridZ = 1.0f / gridSepZ;

    QVBoxLayout *cplace_layout = new QVBoxLayout();
    QVBoxLayout *qundergrowth_layout = new QVBoxLayout();
    QVBoxLayout *undersynth_layout = new QVBoxLayout();

    QLabel *cplace_label = new QLabel("Canopy Placement");
    cplace_layout->addWidget(cplace_label);
    QLabel *qundergrowth_label = new QLabel("Quick Undergrowth Synthesis");
    qundergrowth_layout->addWidget(qundergrowth_label);
    QLabel *undersynth_label = new QLabel("Undergrowth Synthesis");
    undersynth_layout->addWidget(undersynth_label);


    // signal to slot connections
    connect(perspectiveView, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(gridSepXEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridSepZEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridWidthEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(gridIntensityEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourSepEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourWidthEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(contourIntensityEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(radianceEnhanceEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(radianceEnhanceEdit, SIGNAL(returnPressed()), this, SLOT(lineEditChange()));

    //connect(minTreeEdit, SIGNAL(editingFinished()), this, SLOT(treeEditChange()));
    //connect(maxTreeEdit, SIGNAL(editingFinished()), this, SLOT(treeEditChange()));

    // display switches
    connect(checkContours, SIGNAL(stateChanged(int)), this, SLOT(showContours(int)));
    connect(checkGridLines, SIGNAL(stateChanged(int)), this, SLOT(showGridLines(int)));

    renderPanel->setLayout(renderLayout);

    mainLayout->addWidget(renderPanel, 0, 0, Qt::AlignTop);
    mainLayout->addWidget(perspectiveView, 0, 1, 10, 1);

    createActions();
    createMenus();

    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("EcoLearn"));
    mainWidget->setMouseTracking(true);
    setMouseTracking(true);

    renderPanel->hide();

    perspectiveView->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
    perspectiveView->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
    perspectiveView->getRenderer()->setRadianceScalingParams(radianceEnhance);

    std::cerr << "Window construction done" << std::endl;
    //connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
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

    perspectiveView->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
    perspectiveView->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
    perspectiveView->getRenderer()->setRadianceScalingParams(radianceEnhance);
}

void Window::keyPressEvent(QKeyEvent *e)
{
    /*
    if (e->key() == Qt::Key_Escape)
        close();
    else

        QWidget::keyPressEvent(e);
     */
    
    // pass to render window
    perspectiveView->keyPressEvent(e);
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
    perspectiveView->repaint();
}

void Window::cleanup()
{
}

void Window::openTerrain()
{
    std::string valid_files = "16-bit PNG image (*.png);;Terragen File (*.ter);;Ascii Elevation File (*.elv)";
    //valid_files += ";;16-bit PNG image (*.png)";
    bool valid = false;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Terrain File"),
                                                    "~/",
                                                    tr(valid_files.c_str()));
    if (!fileName.isEmpty())
    {
        std::string infile = fileName.toUtf8().constData();

        // use file extension to determine action
        if(endsWith(infile, ".ter")) // terragen file format
        {
            getTerrain().loadTer(infile); valid = true;
        }
        else if(endsWith(infile, ".elv")) // simple ascii heightfield
        {
            getTerrain().loadElv(infile); valid = true;
        }
        else if (endsWith(infile, ".png"))	// load terrain from 16-bit PNG image
        {
            getTerrain().loadPng(infile); valid = true;
            int w, h;
            getTerrain().getGridDim(w, h);
            getGLWidget()->initCHM(w, h);
        }

        if(valid)
        {
            getView().setForcedFocus(getTerrain().getFocus());
            getView().setViewScale(getTerrain().longEdgeDist());
            getTerrain().calcMeanHeight();
            getTerrain().updateBuffers(perspectiveView->getRenderer()); // NB - sets width and height for terrain, otherwise crash
            repaintAllGL();
        }
        else
        {
            cerr << "Error Window::open: attempt to open unrecognized file format" << endl;
        }
    }
}

void Window::openScene()
{
    // Ecosystem files are bundled in a directory, which the user specifies.
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Landscape Scene"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    openScene(dirName.toStdString(), true);

    // if scene load was successful, enable other function of interface
    if (perspectiveView->hasSceneLoaded())
    {
        for (auto ptr : enableAtLandscapeLoad)
            ptr->setEnabled(true);
    }
}

void Window::openScene(std::string dirName, bool import_cluster_dialog)
{
    if (dirName.size() > 0)
    {
        QDir indir(dirName.c_str());
        QString lastseg = indir.dirName();
        std::string dirstr = dirName;
        std::string segstr = lastseg.toUtf8().constData();
        // use last component of directory structure as ecosystem name
        scenedirname = dirstr + "/" + segstr;
#ifndef PAINTCONTROL
        sunwindow->show();
        sunwindow->repaint();
        sunwindow->hide();
#endif
        bool firstscene = !perspectiveView->hasSceneLoaded();
        perspectiveView->sunwindow = sunwindow;
        perspectiveView->loadScene(scenedirname);
        auto ter = perspectiveView->getTerrain();
        float tx, ty;
        ter->getTerrainDim(tx, ty);
        float tm = std::min(tx, ty);

        float startval = tm / 40.0f;
        float endval = tm / 4.0f;

        repaintAllGL();
    }

}


void Window::saveScene()
{
    if(!scenedirname.empty()) // save directly if we already have a file name
    {
        perspectiveView->saveScene(scenedirname);
    }
    else
        saveAsScene();
}

void Window::saveAsScene()
{
    if (!perspectiveView->hasSceneLoaded())
    {
        QMessageBox mbox;
        mbox.setText("Cannot save scene - no scene loaded yet");
        mbox.exec();
        return;
    }
    QFileDialog::Options options;
    QString selectedFilter;
    // use file open dialog but convert to a directory
    QString scenedir = QFileDialog::getSaveFileName(this,
                                    tr("Save Scene"),
                                    "~/",
                                    tr(""),
                                    &selectedFilter,
                                    options);
    if (!scenedir.isEmpty())
    {
        scenedirname = scenedir.toUtf8().constData();
        QDir dir(scenedir);
        if (!dir.exists()) // create directory if it doesn't already exist
        {
            dir.mkpath(".");
        }
        scenedirname += "/" + dir.dirName().toStdString();

        saveScene();
    }
}

void Window::saveAsPaint()
{

    QFileDialog::Options options;
    QString selectedFilter;
    QString paintfile = QFileDialog::getSaveFileName(this,
                                    tr("Save PaintMap As"),
                                    "~/",
                                    tr("Image Files (*.png)"),
                                    &selectedFilter,
                                    options);
    if (!paintfile.isEmpty())
    {
        std::string paintfilename = paintfile.toStdString();

        if (!endsWith(paintfilename, ".png"))
        {
            paintfilename += ".png";
        }
        perspectiveView->writePaintMap(paintfilename);
    }
}

void Window::saveAsCHM()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString chmfile = QFileDialog::getSaveFileName(this,
                                    tr("Save CHM As"),
                                    "~/",
                                    tr("Image Files (*.png)"),
                                    &selectedFilter,
                                    options);
    if (!chmfile.isEmpty())
    {
        std::string chmfilename = chmfile.toStdString();

        if (!endsWith(chmfilename, ".png"))
        {
            chmfilename += ".png";
        }
        perspectiveView->writePaintMap(chmfilename);
    }

}

void Window::showRenderOptions()
{
    renderPanel->setVisible(showRenderAct->isChecked());
}

void Window::showContours(int show)
{
    perspectiveView->getRenderer()->drawContours(show == Qt::Checked);
    repaintAllGL();
}

void Window::showGridLines(int show)
{
    perspectiveView->getRenderer()->drawGridlines(show == Qt::Checked);
    repaintAllGL();
}

void Window::lineEditChange()
{
    bool ok;
    float val;
    float tx, ty, hr;
    //QLineEdit* sender = dynamic_cast<QLineEdit*> sender();

    //getTerrain().getTerrainDim(tx, ty);
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
    cerr << "val entered " << val << endl;

    // without this the renderer defaults back to factory settings at certain stages - very wierd bug
    perspectiveView->getRenderer()->setGridParams(numGridX, numGridZ, gridWidth, gridIntensity);
    perspectiveView->getRenderer()->setContourParams(numContours, contourWidth, contourIntensity);
    perspectiveView->getRenderer()->setRadianceScalingParams(radianceEnhance);
    repaintAllGL();
}

void Window::treeEditChange()
{
    bool ok;
    float val;

    if(sender() == minTreeEdit)
    {
        val = minTreeEdit->text().toFloat(&ok);
        if(ok)
        {
            minTree = val;
        }
    }
    if(sender() == maxTreeEdit)
    {
        val = maxTreeEdit->text().toFloat(&ok);
        if(ok)
        {
            maxTree = val;
        }
    }
    cerr << "val entered " << val << endl;

    // adjust canopy height texture render
    perspectiveView->bandCanopyHeightTexture(minTree, maxTree);
    repaintAllGL();
}

void Window::createActions()
{
    openSceneAct = new QAction(tr("&OpenScene"), this);
    openSceneAct->setShortcuts(QKeySequence::Open);
    openSceneAct->setStatusTip(tr("Open an ecosystem scene directory"));
    connect(openSceneAct, SIGNAL(triggered()), this, SLOT(openScene()));

    /*
     * // Removing import of only terrain, not scene. Will make interface more complicated.
     * // Can be added at a later stage if necessary
    openTerrainAct = new QAction(tr("OpenTerrain"), this);
    openTerrainAct->setStatusTip(tr("Open an existing terrain file"));
    connect(openTerrainAct, SIGNAL(triggered()), this, SLOT(openTerrain()));
    */

    /*
     * // Removing normal save without specifying directory, because it can accidentally overwrite
     * // an existing scene easily. Safer to use "Save Scene as", which creates a new directory or overwrites
     * // if explicitly specified
    saveSceneAct = new QAction(tr("&Save Scene"), this);
    saveSceneAct->setShortcuts(QKeySequence::Save);
    saveSceneAct->setStatusTip(tr("Save the ecosystem scene"));
    connect(saveSceneAct, SIGNAL(triggered()), this, SLOT(saveScene()));
    */

    showRenderAct = new QAction(tr("Show Render Options"), this);
    showRenderAct->setCheckable(true);
    showRenderAct->setChecked(false);
    showRenderAct->setStatusTip(tr("Hide/Show Rendering Options"));
    connect(showRenderAct, SIGNAL(triggered()), this, SLOT(showRenderOptions()));
    showRenderAct->setEnabled(true);	// this option can be triggered without a scene being loaded

    importCanopyAct = new QAction(tr("Import canopy"), this);
    connect(importCanopyAct, SIGNAL(triggered()), this, SLOT(showImportCanopy()));
    importCanopyAct->setEnabled(false);

    importUndergrowthAct = new QAction(tr("Import undergrowth"), this);
    connect(importUndergrowthAct, SIGNAL(triggered()), this, SLOT(showImportUndergrowth()));
    importUndergrowthAct->setEnabled(false);

    viewSpeciesColoursAct = new QAction(tr("Species colours"), this);
    connect(viewSpeciesColoursAct, SIGNAL(triggered()), this, SLOT(showSpeciesColours()));
    viewSpeciesColoursAct->setEnabled(true);		// this option can be triggered without a scene being loaded

    // we create a vector of actions to be enabled when a landscape is loaded
    std::vector<QAction *> tempvec = {
        importCanopyAct,
        importUndergrowthAct
    };

    enableAtLandscapeLoad = tempvec;
}

void Window::showImportCanopy()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import canopy from PDB file"), scenedirname.c_str(), tr("*.pdb"));

    if (!filename.isEmpty())
    {
        perspectiveView->read_pdb_canopy(filename.toStdString());
    }
}

void Window::convertPainting()
{
    ConvertPaintingDialog d;
    if (d.exec() == QDialog::Accepted)
    {
        int from, to;
        d.get_values(from, to);
        BrushType tp_from, tp_to;
        switch (from)
        {
            case 0:
                tp_from = BrushType::FREE;
                break;
            case 1:
                tp_from = BrushType::SPARSETALL;
                break;
            case 2:
                tp_from = BrushType::DENSETALL;
                break;
            default:
                return;
        }
        switch (to)
        {
            case 0:
                tp_to = BrushType::FREE;
                break;
            case 1:
                tp_to = BrushType::SPARSETALL;
                break;
            case 2:
                tp_to = BrushType::DENSETALL;
                break;
            default:
                return;
        }
        perspectiveView->convert_painting(tp_from, tp_to);
    }
    else
    {
    }
}

void Window::hide_all_ctrlwindows()
{
}

void Window::showImportUndergrowth()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import undergrowth from PDB file"), scenedirname.c_str(), tr("*.pdb"));

    if (!filename.isEmpty())
    {
        perspectiveView->read_pdb_undergrowth(filename.toStdString());
    }
}

void Window::showSpeciesColours()
{

    const auto &cdata = perspectiveView->get_cdata();

    if (specColoursWindow)
        delete specColoursWindow;

    specColoursWindow = new SpeciesColoursWindow(this, cdata);
    specColoursWindow->display();
}

void Window::createMenus()
{
    // File menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openSceneAct);

    // View menu
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showRenderAct);
    viewMenu->addAction(viewSpeciesColoursAct);

    // Import menu
    importMenu = menuBar()->addMenu(tr("&Import"));
    importMenu->addAction(importCanopyAct);
    importMenu->addAction(importUndergrowthAct);
}
