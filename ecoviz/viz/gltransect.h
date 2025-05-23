/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  J.E. Gain  (jgain@cs.uct.ac.za)
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

#ifndef GLTRANSECT_H
#define GLTRANSECT_H

#include "glheaders.h" // Must be included before QT opengl headers
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <list>
#include <memory>

#include "scene.h"
#include "view.h"
#include "timewindow.h"
#include "progressbar_window.h"

//! [0]

class Window;

class GLTransect : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:

    GLTransect(const QSurfaceFormat& format, Window * wp, Scene * scn, Transect * trans, QWidget *parent = 0);
    ~GLTransect();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setParent(Window * wp){ winparent = wp; }

    /// getters for currently active view, terrain, typemaps, renderer, ecosystem
    PMrender::TRenderer * getRenderer();

    /**
     * @brief updateTransectView Change the view to match the current transect parameters
     */
    void updateTransectView();

    /**
     * @brief setScene Change the scene being displayed and initialize a new default view
     * @param s Scene to display
     */
    void setScene(Scene * s);

    /// set active status of transect (i.e. should this view be painted)
    void setActive(bool v) { active = v; }

    /// getter for various viewing controls
    Scene * getScene(){ return scene; }
    View * getView(){ return view; }
    Transect * getTransect(){ return trx; }

    /// create an independent view object with the same parameter
    void unlockView(Transect * imposedTrx);

    /// create a locked view object by overwriting the current view
    void lockView(View * imposedView, Transect * imposedTrx);

    /// toggle lock flag
    void setViewLockState(bool state){ viewlock = state; }

    /// Respond to key press events
    void keyPressEvent(QKeyEvent *event);

    /// set scaling value for all terrains
    void setScales(float sc);

    /// Make all plants visible
    void setAllPlantsVis();

    /// Toggle canopy plant visibility
    void setCanopyVis(bool vis);

    /// Toggle undergrowth plant visibility
    void setUndergrowthVis(bool vis);

    /// Turn all species either visible or invisible
    void setAllSpecies(bool vis);

    /// Turn on visibility for a single plant species only (all others off)
    void setSinglePlantVis(int p);

    /// Toggle visibility of an individual species on or off
    void toggleSpecies(int p, bool vis);

    /// switch scene - should only be called when a new sub-terrainis extracted
    void switchTransectScene(Scene *newScene, Transect *newTransect);

signals:
    void signalRepaintAllGL();

public slots:
    void rebindPlants(); // set flag indicating that plants need to be re-bound

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * wheel);

private:

    QSurfaceFormat glformat; //< format for OpenGL
    Window * winparent;
    Scene * scene;      //< wrapper for terrain, various maps, and ecosystem
    View * view;        //< viewpoint controls
    std::string datadir;

    // render variables
    PMrender::TRenderer * renderer;

    // gui variables
    bool viewlock;
    bool focuschange;
    bool timeron;
    bool active; //< scene only rendered if this is true
    bool rebindplants; //< flag to indicate that plants have changed and need to be rebound
    bool forceRebindPlants; //< flag to override lack of 'focus' (which never seems to be used?)
    std::vector<bool> plantvis;
    bool canopyvis; //< display the canopy plants if true
    bool undervis; //< display the understorey plants if true
    float scf;
    Transect * trx; //< transect control parameters

    // PCM 2023 - can be used to test geometry against transect
    std::vector<Plane> transectPlanes;

    QPoint lastPos;
    QColor qtWhite;
    QLabel * vizpopup;  //< for debug visualisation

    /**
     * @brief paintCyl  called by PaintGL to display a cylinder
     * @param p     position of base of cylinder on terrain
     * @param col   colour of the cylinder
     * @param drawParams accumulated rendering state
     */
    void paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams);
};

#endif
