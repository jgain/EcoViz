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

#ifndef WINDOW_H
#define WINDOW_H

#include "glwidget.h"
#include "gltransect.h"
#include "chartwindow.h"
#include "mitsuba_model.h"
#include <QWidget>
#include <QtWidgets>
#include <string>

class QAction;
class QMenu;
class QLineEdit;
class GLWidget;

enum LockState
{
    UNLOCKED,           //< panels are independent
    LOCKEDFROMLEFT,     //< left panel dictates behaviour of right panel
    LOCKEDFROMRIGHT     //< right panel dictates behaviour of left panel
};

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(std::string datadir);

    ~Window();

    QSize sizeHint() const;

    /// Adjust rendering parameters, grid and contours, to accommodate current scale
    void scaleRenderParams(float scale);
    void loadSceneData(void); // load data from disk for each scene
    void run_viewer();

public slots:
    void repaintAllGL();
    void transectSyncPlace(bool firstplace);
    void timelineSync(int t);

    // menu items
    void showRenderOptions();
    void showPlantOptions();
    void showContours(int show);
    void showGridLines(int show);
    void exportMitsuba();

    // render panel
    void lineEditChange();
    void mapChange(bool on);
    void cameraChange(int idx);

    // plant panel
    void plantChange(int show);
    void allPlantsOn();
    void allPlantsOff();

    // locking
    void lockViewsFromLeft();
    void lockViewsFromRight();
    void lockTransectFromLeft();
    void lockTransectFromRight();
    void lockTimelineFromLeft();
    void lockTimelineFromRight();

    void showTransectViews();
    void clearTransects();

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void optionsChanged();

private:
    std::vector<Scene *> scenes;                ///< scenes for each half
    std::vector<Transect *> transectControls;   ///< Transect controls for each viewing pair
    std::vector<GLWidget *> perspectiveViews;   ///< OpenGL perspective rendering views
    std::vector<GLTransect *> transectViews;    ///< OpenGL transect views
    std::vector<TimeWindow *> timelineViews;    ///< widget for timeline control
    std::vector<ChartWindow *> chartViews;      ///< widget for displaying graphs
    std::vector<std::vector< TimelineGraph *> > graphModels;   ///< Underlying graph data associated with scene, multiple graphs per scene
    QWidget * vizPanel;                         ///< Central panel with visualization subwidgets
    QWidget * renderPanel;                      ///< Side panel to adjust various rendering style parameters
    QWidget * plantPanel;                       ///< Side panel to adjust various plant visualization parameters
    QGridLayout * vizLayout;

    // rendering parameters
    float gridSepX, numGridX, gridSepZ, numGridZ, gridWidth, gridIntensity; ///< grid params
    float contourSep, numContours, contourWidth, contourIntensity; ///< contour params
    float radianceTransition, radianceEnhance; ///< radiance scaling params

    // map parameters
    int sunMonth, wetMonth, tempMonth;

    // render panel widgets
    QLineEdit * gridSepXEdit, * gridSepZEdit, * gridWidthEdit, * gridIntensityEdit, * contourSepEdit, * contourWidthEdit, * contourIntensityEdit, * radianceEnhanceEdit;
    QComboBox * cameraDropDown;

    // plant viz panel widgets
    QLineEdit * sunMapEdit, * wetMapEdit;
    QRadioButton * sunMapRadio, * wetMapRadio, * chmMapRadio, * noMapRadio;
    QLineEdit * smoothEdit;

    // menu widgets and actions
    QMenu *viewMenu;
    QAction *showRenderAct;
    QAction *showPlantAct;
    QAction *exportMitsubaAct;
    QAction *fromLeftTransectAct, *fromRightTransectAct;
    QAction *clearTransectsAct;
    bool transectsValid;

    // file management
    std::string scenedirname;

    // locking management
    LockState viewLock, transectLock, timelineLock;
    QPushButton * lockT1, * lockT2, * lockV1, * lockV2, * lockG1, * lockG2;
    QGroupBox *lockTGroup;
     QIcon * lockleftIcon, * unlockleftIcon, * lockrightIcon, * unlockrightIcon;

    // Map containing the different export profiles (Mitsuba)
    map<string, map<string, vector<MitsubaModel>>> profileToSpeciesMap;

    /**
     * @brief setupRenderPanel  Initialize GUI layout of side render panel for controlling various rendering parameters
     */
    void setupRenderPanel();

    /**
     * @brief setupPlantPanel  Initialize GUI layout of side plant viz panel for controlling various plant display parameters
     */
    void setupPlantPanel();

    /**
     * @brief setupVizPanel  Initialize GUI layout of central visualization
     */
    void setupVizPanel();

    /**
     * @brief setupGraphModels  set up the data structures for dynamic graphs
     */
    void setupGraphModels(int scene_index);

    /**
     * @brief setSmoothing Set smoothing distance to soften the underlying plant grid
     * @param d smoothing distance value
     */
    void setSmoothing(int d);

    // init menu
    void createActions();
    void createMenus();

    /**
      * @brief unlockTimelines Unlink timelines so that graphs and play controls are no longer synchronized
      */
    void unlockTimelines();

    /**
     * @brief unlockViews Unlink views so that perspective renderings are no longer synchronized
     */
    void unlockViews();

    /**
     * @brief unlockTransects Unlink transacts so that they are no longer synchronized
     */
    void unlockTransects();

    /**
     * @brief readMitsubaExportProfiles Read the CSV files in the specified folder to fill profiles map
     * @param profilesDirPath Path of the folder containing the export profiles
     */
    void readMitsubaExportProfiles(string profilesDirPath);
};

#endif
