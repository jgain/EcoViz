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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glheaders.h" // Must be included before QT opengl headers
#include <QGLWidget>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <list>
#include "common/basic_types.h"
#include <memory>

#include "scene.h"
#include "view.h"
#include "timewindow.h"
#include "progressbar_window.h"
#include "gltransect.h"

//! [0]

const float manipradius = 75.0f;
const float manipheight = 750.0f;
const float armradius = manipradius / 2.5f;
const float tolzero = 0.01f;

const float transectradius = 50.0f;

const float seaval = 2000.0f;
const float initmint = 0.0f;
const float initmaxt = 40.0f;
const float mtoft = 3.28084f;

class Window;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:

    GLWidget(const QGLFormat& format, Scene * scn, Transect * trans, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /**
     * capture the framebuffer as an image
     * @param capImg    framebuffer is written to this image
     * @param capSize   the image is scaled by linear interpolation to this size
     */
    void screenCapture(QImage * capImg, QSize capSize);

    /// getters for currently active view, terrain, typemaps, renderer, ecosystem
    PMrender::TRenderer * getRenderer();

    /// getter and setter for brush radii
    float getRadius();
    void setRadius(float rad);

    /// getter, setter, refresher for overlay texture being displayed
    void refreshOverlay();
    void setOverlay(TypeMapType purpose);
    TypeMapType getOverlay();
    void setMap(TypeMapType type, int mth);

    /**
     * @brief bandCanopyHeightTexture   Recolour the canopy height texture according to a band of min and max tree heights
     * @param mint  Minimum tree height (below which heights are coloured black)
     * @param maxt  Maximum tree height (above which heights are coloured red)
     */
    void bandCanopyHeightTexture(float mint, float maxt);

    /**
     * @brief writePaintMap Output image file encoding the paint texture layer. Paint codes are converted to greyscale values
     * @param paintfile image file name
     */
    void writePaintMap(std::string paintfile);

    /**
     * @brief writeGrass Output terragen image files related to the grass layer
     * @param grassrootfile  name of root image file, all images use this as the prefix
     */
    void writeGrass(std::string grassrootfile);

    /**
     * @brief setScene Change the scene being displayed and initialize a new default view
     * @param s Scene to display
     */
    void setScene(Scene * s);

    /// getter for scene attached to glwidget
    Scene * getScene(){ return scene; }
    View * getView(){ return view; }

    /// Prepare decal texture
    void loadDecals();

    /// Load from file to appropriate TypeMap depending on purpose
    int loadTypeMap(basic_types::MapFloat * map, TypeMapType purpose);

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

    template<typename T>
    int loadTypeMap(const T &map, TypeMapType purpose);

signals:
    void signalRepaintAllGL();
    void signalShowTransectView();
    
public slots:
    void animUpdate(); // animation step for change of focus
    void rotateUpdate(); // animation step for rotating around terrain center
    void rebindPlants(); // set flag indicating that plants need to be re-bound

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * wheel);

private:

    QGLFormat glformat; //< format for OpenGL
    Scene * scene;      //< wrapper for terrain, various maps, and ecosystem
    View * view;        //< viewpoint controls
    std::string datadir;

    // render variables
    PMrender::TRenderer * renderer;
    bool decalsbound;
    GLuint decalTexture;

    // gui variables
    bool viewlock;
    bool focuschange;
    bool focusviz;
    bool timeron;
    bool active; //< scene only rendered if this is true
    std::vector<bool> plantvis;
    bool canopyvis; //< display the canopy plants if true
    bool undervis; //< display the understorey plants if true
    bool showtransect; //< display the widgets associated with the transect
    bool rebindplants; //< flag to indicate that plants have changed and need to be rebound
    float scf;
    int sun_mth; // which month to display in the sunlight texture
    int wet_mth; // which month to display in the moisture texture
    TypeMapType overlay; //< currently active overlay texture: CATEGORY, WATER, SUNLIGHT, TEMPERATURE, etc

    QPoint lastPos;
    QColor qtWhite;
    QTimer * atimer, * rtimer; // timers to control different types of animation
    QLabel * vizpopup;  //< for debug visualisation

    // transect parameters
    vpPoint t1, t2;
    Transect * trx;
    int trxstate;
    Shape trxshape[3]; //< geometry for transect line display

    /**
     * @brief createLine    Create sub-line for part of the transect
     * @param line          Vertices of line
     * @param start         Starting position for line
     * @param end           Ending position for line
     * @param hghtoffset    amount that line is lifted above the terrain for visibility
     */
    void createLine(vector<vpPoint> * line, vpPoint start, vpPoint end, float hghtoffset);

    /**
     * @brief createTransectShape Instantiate the geometry for a transect line that crosses the terrain
     * @param hghtoffset    amount that line is lifted above the terrain for visibility
     */
    void createTransectShape(float hghtoffset);

    /**
     * @brief paintCyl  called by PaintGL to display a cylinder
     * @param p     position of base of cylinder on terrain
     * @param col   colour of the cylinder
     * @param drawParams accumulated rendering state
     */
    void paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams);

    /**
     * @brief paintCyl  called by PaintGL to display a sphere
     * @param p     position of sphere on terrain
     * @param col   colour of the cylinder
     * @param drawParams accumulated rendering state
     */
    void paintSphere(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams);

    /**
     * @brief paintTransect called by PaintGL to display the transect line
     * @param col           colour of the line
     * @param drawParams    accumulated rendering state
     */
    void paintTransect(GLfloat * col, std::vector<ShapeDrawData> &drawParams);

    /**
     * @brief pickInfo  write information about a terrain cell to the console
     * @param x         x-coord on terrain grid
     * @param y         y-coord on terrain grid
     */
    void pickInfo(int x, int y);
};

#endif
