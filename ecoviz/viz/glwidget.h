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
class overviewWindow;

class GLWidgetAdrien : public QGLWidget
{
    Q_OBJECT

public:

    GLWidgetAdrien(const QGLFormat& format, Window * wp, Scene * scn, Transect * trans, const std::string &widgetName, mapScene *mapScene, QWidget *parent = 0);

    ~GLWidgetAdrien();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setParent(Window * wp){ winparent = wp; }

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
    bool getActive(){ return active; }
    bool getPainted(){ return painted; }
    bool getTextureActive(){ return decalsbound; }

    /// get internal state object that manages map window
    overviewWindow *getOverviewWindow(void) { return mapView; }

    /// alter the mode of the camera view to either ARBALL or FLY
    void changeViewMode(ViewMode vm);

    /// create an independent view object with the same parameter
    void unlockView();

    /// create a locked view object by overwriting the current view
    void lockView(View * imposedView);

    /// toggle lock flag
    void setViewLockState(bool state){ viewlock = state; }

    /// control the texture overlay

    /**
     * @brief setDataMap  Load a data map into the texture
     * @param dataIdx     Which data map to use (indexed from 1, with 0 meaning none)
     * @param ramp        Which colour map to use
     * @param updatenow   Push the texture update immediately if true
     */
    void setDataMap(int dataIdx, TypeMapType ramp, bool updatenow);

    /// get Transect
    TransectCreation * getTransectCreate(){ return trc; }

    /// set Transect
    void setTransectCreate(TransectCreation * newtrc);

    // make transect independent
    void seperateTransectCreate(Transect * newtrx);

    /// PCM: force new transect data on existing and reset - needs to free existing texture; only
    /// used when a sub-terrain has been extracted since this invalidates existing information
    //  void forceTransect(Transect *newTrans);

    /// Prepare decal texture
    void loadDecals();

    /// Load from file to appropriate TypeMap depending on purpose
    void loadTypeMap(basic_types::MapFloat * map, TypeMapType purpose, float range = 1.0f);

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
    void loadTypeMap(const T &map, TypeMapType purpose, float range);

    /**
     * @brief pointPlaceTransect    GUI actions related to transect point placement
     * @param firstPoint true if this is the placement of the first transect point, false if the second
     */
    void pointPlaceTransect(bool firstPoint);

    void resetTransectState()
    {
        if(trc != nullptr)
        {
            trc->trxstate = -1;
            loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
            refreshOverlay();
        }
    }

    // replace current FloatImage for the overlay with newly sized version from newTerr
    // PCM: is excisting GL texture is correctly released?
    /*
    void replaceTransectImageMap(Terrain *newTerr)
    {
        if (trc != nullptr && trc->trx != nullptr)
        {
           int dx, dy, tdx, tdy;
           newTerr->getGridDim(dx, dy);
           trc->trx->reset(newTerr);
           trc->trx->getTransectMap()->getDim(tdx, tdy);
           std::cout << " ^^^ transect reset: terr dims [" << dx << "," << dy << "]; mapviz size [" <<
                        tdx << "," << tdy << "]\n";
           trc->trxstate = -1;
           trc->trx->setValidFlag(false);
           loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
           setOverlay(TypeMapType::EMPTY);
        }
        else
            throw  std::runtime_error("replaceTransectImageMap - applied to a null pointer!");
    }
    */

signals:
    void signalRepaintAllGL();
    void signalShowTransectView();
    void signalSyncPlace(bool firstPoint);
    void signalRebindTransectPlants();
    //void signalUpdateOverviews();
    void signalExtractNewSubTerrain(int, int, int, int, int); // window (left/right) + region corners
    void signalSyncDataMap();
    
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
    Window * winparent;
    Scene * scene;      //< wrapper for terrain, various maps, and ecosystem
    View * view;        //< viewpoint controls
    overviewWindow *mapView; //< manage state and render state for overview map attached to this widget
    std::string datadir;
    std::string wname; //< name for this widget

    // transect parameters
    TransectCreation * trc;

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
    bool painted; //< set after first successful paint
    bool persRotating; // if arcball rotation of main perspective view is active
    bool overviewPick; // if a mouse press occurs on the overview map
    std::vector<bool> plantvis;
    bool canopyvis; //< display the canopy plants if true
    bool undervis; //< display the understorey plants if true
    bool rebindplants; //< flag to indicate that plants have changed and need to be rebound
    float scf;
    int sun_mth; // which month to display in the sunlight texture
    int wet_mth; // which month to display in the moisture texture
    TypeMapType overlay; //< currently active overlay texture: CATEGORY, WATER, SUNLIGHT, TEMPERATURE, etc

    QPoint lastPos;
    QColor qtWhite;
    QTimer * atimer, * rtimer; // timers to control different types of animation
    QLabel * vizpopup;  //< for debug visualisation

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

    /**
     * @brief refreshViews Signal update to either this view or all views depending on lock state
     */
    void refreshViews();
};

class overviewWindow {
    public:

    overviewWindow(mapScene * scn);
    ~overviewWindow();

    /// getters for currently active view, terrain, typemaps, renderer, ecosystem
    PMrender::TRenderer * getRenderer() { return mrenderer; }


    /**
     * @brief setScene Change the scene being displayed and initialize a new default view
     * @param s Scene to display
     */
    void setScene(mapScene * s);

    /// set the selecton region (this will then be updated internally)
    void setSelectionRegion(Region reg)
    {
        currRegion = reg;
        // store dimensions of full terrain grid for bounds checking
        scene->getHighResTerrain()->getGridDim(mapWidth, mapHeight);
    }

    bool isSelectionValid(Region subwindow)
    {
        if (subwindow.x0 < 0 || subwindow.x0 > mapWidth-1)
            return false;
        if (subwindow.x1 < 0 || subwindow.x1 > mapWidth-1)
            return false;
        if (subwindow.y0 < 0 || subwindow.y0 > mapHeight-1)
            return false;
        if (subwindow.y1 < 0 || subwindow.y1 > mapHeight-1)
            return false;

      return true;
    }

    Region getSelectionRegion(void) const
    {
        return currRegion;
    }

    Region getEntireRegion(void)
    {
        return getScene()->getHighResTerrain()->getEntireRegion();
    }

    /// getter for various viewing controls
    mapScene * getScene(){ return scene; }
    View * getView(){ return mview; }
    //bool getActive(){ return active; }
    bool isTerrainReady(void) const { return terrainReady; }
    void setTerrainReady(bool v) { terrainReady = v; }

    void getWindowSize(int & width, int & height)
    {
        width = ovw; height = ovh;
    }

    void getViewSize(int perswidth, int & width, int & height)
    {
        float aspect = (float) ovh / (float) ovw;
        width = (int) ((float) perswidth * perscale); height = (int) ((float) width * aspect);
    }

    bool getPickOnTerrain(){ return pickOnTerrain; }

    bool mouseInOverView(int winwd, int winht, int mx, int my)
    {
        int wd, ht;

        getViewSize(winwd, wd, ht);
        return (mx > winwd - wd && my < ht);
    }

    /// Transform perspective window coordinates (sx, sy) to overview map coordinates (ox, oy)
    void mouseCoordTransform(int w, int h, int sx, int sy, int &ox, int &oy);

    /**
     * @brief startRegionDemarcation On mouse down in overview window, start marking out a new sub-region
     * @param x mouse x-coord
     * @param y mouse y-coord
     */
    void startRegionDemarcation(int x, int y);

    /**
     * @brief startRegionTranslate On mouse down in overview window over existing sub-region, begin translation
     * @param x mouse x-coord
     * @param y mouse y-coord
     */
    void startRegionTranslate(int x, int y);

    /**
     * @brief continueRegionDemarcation Process mouse movement in sub-region demarcation mode
     * @param x mouse x-coord
     * @param y mouse y-coord
     */
    void continueRegionDemarcation(int x, int y);

    /**
     * @brief continueRegionTranslate Process mouse movement in sub-region translation mode
     * @param x mouse x-coord
     * @param y mouse y-coord
     */
    void continueRegionTranslate(int x, int y);

    /**
     * @brief endRegionChange On mouse up finalise the new sub-region position and dimensions if necessary
     * @return true if a new terrain needs to be extracted, otherwise false
     */
    bool endRegionChange();

    /// recalculate View params for viewport
    void updateViewParams(void);

    /// force height/normal map recompute
    void forceUpdate(void)
    {
        scene->getLowResTerrain()->setBufferToDirty();
    }

    /// draw
    void draw(void);

    /// set size based on aspect ratio of terrain
    void setWindowSize(void);

    /// set view dim
    void resetViewDims(void)
    {
        mview->setDim(0.0f, 0.0f, ovw, ovh);
    }

private:

    mapScene * scene;      //<overview scene info
    View * mview;        //< viewpoint controls
    //std::string datadir;
    //int widgetId; // left render winsow =0, right=1 in main window
    Region currRegion; // relative to full resolution input (stored in mapScene)
    Region prevRegion; // fallback if a new subregion creation fails
    int pick0x, pick0y, pick1x, pick1y; // region bounds during mouse movement
    int mapWidth; // entire map for bounds checking extracted sub-regions
    int mapHeight;
    vpPoint pickPos;
    bool pickOnTerrain; // signals manipulation of selection region
    int ovw, ovh; // width and height of overview window based on terrain aspect ratio
    float perscale; // what horizontal proportion of the perspective view should be occupied by overview

    // set to true once terrain is actually loaded (default dims used when widget created)
    bool terrainReady;

    // render variables
    PMrender::TRenderer * mrenderer;

    // gui variables
    bool active;
    bool timeron;
    float scf;

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

    // paint on the overview selection window (size and position obtained from Terrain)
    void paintSelectionPlane(GLfloat *col, std::vector<ShapeDrawData> & drawparams);

    void paintSphere(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams);


    // set up state for renderer for overview - this will use the context for glwidget for resources
    void initializeMapRenderer(void);
};

#endif
