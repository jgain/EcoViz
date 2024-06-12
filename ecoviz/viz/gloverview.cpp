/*******************************************************************************
 *
 * EcoViz - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2021  J.E. Gain  (jgain@cs.uct.ac.za)
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


#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "gloverview.h"

#include <QGridLayout>
#include <QImage>
#include <QCoreApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QRunnable>
#include <QThreadPool>
#include <fstream>

#include "eco.h"
#include "data_importer/data_importer.h"
#include "data_importer/map_procs.h"
#include "cohortmaps.h"
#include "window.h"

using namespace std;

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLOverview::GLOverview(const QSurfaceFormat& format, Window * wp, mapScene * scn, int Id, QWidget *parent)
    : QOpenGLWidget(parent)
{
    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    glformat = format;
    setFormat(glformat);
    timeron = false;
    widgetId = Id;

    currRegion = Region(0,0,0,0);
    mapWidth = mapHeight = 0;

    setParent(wp);

    view = nullptr;

    setScene(scn);
    active = true;
    pickOnTerrain = false;

    renderer = new PMrender::TRenderer(nullptr, "../viz/shaders/");
   
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    ovw = 50; ovh = 50;
    resize(sizeHint());
    setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
    signalRepaintAllGL();
}

GLOverview::~GLOverview()
{
    if (renderer) delete renderer;
}

QSize GLOverview::minimumSizeHint()
{
    return QSize(50, 50);
}

QSize GLOverview::sizeHint()
{
    int w, h;
    if(scene != nullptr)
    {

        scene->getLowResTerrain()->getGridDim(w, h);
        // reset window to match aspect ratio of the terrain

        if(h > w)
        {
            ovh = (int) (200.0 / (float) h * (float) w);
            ovw = 200;
        }
        else
        {
            ovh = 200;
            ovw = (int) (200.0 / (float) w * (float) h);
        }
    }
    return QSize(ovw, ovh);
}

PMrender::TRenderer * GLOverview::getRenderer()
{
    return renderer;
}

void GLOverview::setScene(mapScene * s)
{ 
    int h, w;
    scene = s;

    if (view != nullptr) delete view;

    this->resize(sizeHint());
    this->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
    view = new View();

    scene->getLowResTerrain()->setMidFocus();
    view->setViewType(ViewState::ORTHOGONAL);
    view->setForcedFocus(scene->getLowResTerrain()->getFocus());
    // view->setViewScale(scene->getLowResTerrain()->longEdgeDist());
    view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));
    scf = scene->getLowResTerrain()->getMaxExtent();

    // orthogonal rendering
    //view->setViewType(ViewState::ORTHOGONAL);
    //view->setZoomdist(0.0f);
    //view->setOrthoViewDepth(2000.0f);
    view->setOrthoViewExtent(scene->getLowResTerrain()->longEdgeDist());

    std::cout << "***** setScene(mapScene) - data:\n";
    vpPoint pt = scene->getLowResTerrain()->getFocus();
    std::cout << " ** focus.x: " << pt.x;
    std::cout << " ** focus.y: " << pt.y;
    std::cout << " ** focus.z: " << pt.z;
    std::cout << " ** scf: " << scf << "\n";
    std::cout << " ** wd: " << this->width() << "\n";
    std::cout << " ** ht: " << this->height() << "\n";
    std::cout << " ** terrain scale: " << scene->getLowResTerrain()->longEdgeDist() << "\n";

    // scene->getLowResTerrain()->setMidFocus();
    view->topdown();

    scene->getLowResTerrain()->setBufferToDirty();

    //winparent->rendercount++;
    //signalRepaintAllGL();
    //update();
}

// the heightfield will change after initial dummy creation (and possible edits later)
// make sure View params are correct.

void GLOverview::updateViewParams(void)
{
    scene->getLowResTerrain()->setMidFocus();
    view->setForcedFocus(scene->getLowResTerrain()->getFocus());
    // view->setViewScale(scene->getLowResTerrain()->longEdgeDist());
    scf = scene->getLowResTerrain()->getMaxExtent();

    // orthogonal rendering
    view->setViewType(ViewState::ORTHOGONAL);
    float minHt, maxHt;
    scene->getHighResTerrain()->getHeightBounds(minHt, maxHt); // need *global* max height

    vpPoint orthoFocusTop;
    orthoFocusTop = scene->getLowResTerrain()->getFocus();
    orthoFocusTop.y = 1.1* maxHt + 100.0f; // PCM: for some reason the camera near plane clips even when using maxHt
    // could be the slight near plane offset e=0.01, but that is small and an addiive offset should have fixed that.
    // Possible issue?

    // std::cerr << "GLOverview: Camera front clipping plane height: " << orthoFocusTop.y << std::endl;
    view->setForcedFocus(orthoFocusTop);
    view->setOrthoViewDepth(100000.0f); // make this large to avoid issues!
    view->setOrthoViewExtent(scene->getLowResTerrain()->longEdgeDist()); // this scales to fit in window

    view->topdown();
    view->setZoomdist(0.0f);
    //view->canyonview();

    view->apply();
}

void GLOverview::initializeGL()
{
    // get context opengl-version
    qDebug() << "\nGLOverview initialize....\n";
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid() << "; Address: " << context();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." <<
              context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    QSurfaceFormat glFormat = QOpenGLWidget::format();
    // if ( !glFormat.sampleBuffers() )
    //     qWarning() << "Could not enable sample buffers";

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    int mu;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mu);
    cerr << "max texture units = " << mu << endl;

    // *** PM Render code - start ***

    // To use basic shading: PMrender::TRenderer::BASIC
    // To use radiance scaling: PMrender::TRenderer::RADIANCE_SCALING
    // PMrender::TRenderer::RADIANCE_SCALING;
    // PMrender::TRenderer::RADIANCE_SCALING_TRANSECT;
    PMrender::TRenderer::terrainShadingModel  sMod = PMrender::TRenderer::RADIANCE_SCALING_OVERVIEW;

    // set terrain shading model
    renderer->setTerrShadeModel(sMod);

    // set up light
    Vector dl = Vector(0.6f, 1.0f, 0.6f);
    dl.normalize();

    //GLfloat pointLight[3] = {0.5, 5.0, 7.0}; // side panel + BASIC lighting
    GLfloat pointLight[3] = {1000.0, 3000.0, 1000.0}; // side panel + BASIC lighting
    GLfloat dirLight0[3] = { dl.i, dl.j, dl.k}; // for radiance lighting
    GLfloat dirLight1[3] = { -dl.i, dl.j, -dl.k}; // for radiance lighting

    renderer->setPointLight(pointLight[0],pointLight[1],pointLight[2]);
    renderer->setDirectionalLight(0, dirLight0[0], dirLight0[1], dirLight0[2]);
    renderer->setDirectionalLight(1, dirLight1[0], dirLight1[1], dirLight1[2]);

    // initialise renderer/compile shaders
    renderer->initShaders();

    // set other render parameters
    // can set terrain colour for radiance scaling etc - check trenderer.h

    // terrain contours
    renderer->drawContours(false);
    renderer->drawGridlines(false);

    // turn on terrain type overlay (off by default); NB you can stil call methods to update terrain type,
    renderer->useTerrainTypeTexture(false);
    renderer->useConstraintTypeTexture(false);

    // use manipulator textures (decal'd)
    renderer->textureManipulators(false);

    // *** PM Render code - end ***

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_CLAMP);
    //glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);

    // overlay - load??
    //paintGL();
}

void GLOverview::paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
{
    ShapeDrawData sdd;
    float scale;
    Shape shape;
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    std::vector<glm::vec3> translInstance;
    std::vector<glm::vec2> scaleInstance;
    std::vector<float> cinst;

    // create shape
    shape.clear();
    shape.setColour(col);

    // place vertical cylinder
    scale = view->getScaleFactor();
    idt = glm::mat4(1.0f);
    trs = glm::vec3(p.x, p.y, p.z);
    rot = glm::vec3(1.0f, 0.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(-90.0f), rot);

    float mrad = 75.0f;
    float mheight = 750.0f;
    float arad = mrad / 2.5f;

    shape.genCappedCylinder(scale*arad*50.0f, 1.5f*scale*arad*50.0f, scale*(mheight-mrad)*50.0f, 40, 10, tfm, false);
    if(shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}

void GLOverview::paintSelectionPlane(GLfloat *col, std::vector<ShapeDrawData> &drawparams)
{
    ShapeDrawData sdd;
    Shape shape;
    glm::mat4 tfm, idt;
    std::vector<glm::vec3> translInstance;
    std::vector<glm::vec2> scaleInstance;
    std::vector<float> cinst;

    // create shape
    shape.clear();
    shape.setColour(col);

    Region region;
    vpPoint centre;
    float planeHeight, planeWidth;
    region = currRegion; //  getScene()->getSelectedRegion(); //  (region, startx, starty, endx, endy);
    float minHt, maxHt;
    scene->getHighResTerrain()->getHeightBounds(minHt, maxHt); // need global maxHt
    float pointStep = scene->getHighResTerrain()->getPointStep();

    // std::cout << "@@@@ paintSelectionPlane: input region [" << region.x0 << "," << region.y0 <<
    //              "," << region.x1 << "," << region.y1 << "]\n";

    centre.y = 1.1*maxHt + 10.0f; // PCM: hack **** I think the quat rotation is inaccurate and camera is off angle slightly
    // JG - the quaternion rotation is now fixed
    centre.x = (pointStep*region.y0 + pointStep*region.y1)/2.0; // PCM: again, flip since drawgrid is flipped
    centre.z = (pointStep*region.x0 + pointStep*region.x1)/2.0;
    idt = glm::mat4(1.0f);
    tfm = glm::translate(idt, glm::vec3(centre.x, centre.y, centre.z));

    // PCM: this seems to be implied by way plane is defined?
    planeWidth  = pointStep*(region.y1 - region.y0 + 1)/2.0f;
    planeHeight = pointStep*(region.x1 - region.x0 + 1);

    /*
    std::cout << "\nSelection plane data:\n";
    std::cout << "centre = (" << centre.x << "," << centre.y << "," << centre.z << ")\n";
    std::cout << "point step: " << pointStep << std::endl;
    std::cout << "(planeWidth, planeHeight) = " << planeWidth << "," << planeHeight << ")\n";
    */

    shape.genPlane(vpPoint(0.0,0.0,1.0), vpPoint(0.0,0.0,0.0), planeWidth, planeHeight, tfm);
    if (shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawparams.push_back(sdd);
    }
}

void GLOverview::paintSphere(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
{
    ShapeDrawData sdd;
    float scale;
    Shape shape;
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    std::vector<glm::vec3> translInstance;
    std::vector<glm::vec2> scaleInstance;
    std::vector<float> cinst;

    // create shape
    shape.clear();
    shape.setColour(col);

    // place vertical cylinder
    scale = view->getScaleFactor();
    idt = glm::mat4(1.0f);
    trs = glm::vec3(p.x, p.y, p.z);
    rot = glm::vec3(1.0f, 0.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    // tfm = glm::rotate(tfm, glm::radians(-90.0f), rot);
    shape.genSphere(scale * transectradius*5.0f, 40, 40, tfm);
    if(shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}


void GLOverview::paintGL()
{
    vpPoint mo;
    //glm::mat4 tfm, idt;
    //glm::vec3 trs, rot;
    std::vector<ShapeDrawData> drawParams; // to be passed to terrain renderer
    Shape shape, planeshape;  // geometry for focus indicator
    std::vector<glm::mat4> sinst;
    std::vector<glm::vec4> cinst;

    GLfloat blueish[] = {0.325f, 0.235f, 1.0f, 1.0f};
    GLfloat purpleish[] = {0.6f, 0.2f, 0.8f, 1.0f};
    GLfloat pickCol[] = {1.0f, 0.2f, 0.2f, 1.0f};

    Timer t;

    if(active)
    {
        // std::cout << " ------- GLOverview::paintGL() called ----------\n";
        drawParams.clear();
        t.start();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // viewport is incorrect on creation, this will ensure current is used to match View
        updateViewParams();

        // build selection plane 'manipulator'
        if(pickOnTerrain)
            paintSelectionPlane(purpleish, drawParams);
        else
            paintSelectionPlane(blueish, drawParams);

        // paintSphere(pickPos, pickCol, drawParams);

        // pass in draw params for objects
        renderer->setConstraintDrawParams(drawParams);

        // draw terrain

        //scene->getLowResTerrain()->setBufferToDirty();
        // draw terrain  with selection plane
        if (drawParams.size() > 0) // DEBUG: PCM
            scene->getLowResTerrain()->updateBuffers(renderer);

        renderer->draw(view);

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
    }
}

void GLOverview::resizeGL(int width, int height)
{
    // TO DO: fix resizing
    // int side = qMin(width, height);
    // glViewport((width - side) / 2, (height - side) / 2, width, height);
    glViewport(0, 0, width, height);

    view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
    view->apply();
}


void GLOverview::keyPressEvent(QKeyEvent *event)
{
}


void GLOverview::mousePressEvent(QMouseEvent *event)
{
    vpPoint pnt;

    int x = event->x(); int y = event->y();
    float W = static_cast<float>(width()); float H = static_cast<float>(height());
    float deltaX, deltaY;

    // for specifying a new region from scratch, right click and drag
    if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
    {
        view->apply();
        if(scene->getHighResTerrain()->pick(x, y, view, pnt))
        {
            pickPos = pnt;
            pickOnTerrain = true;
            // convert to grid coordinates
            scene->getHighResTerrain()->toGrid(pickPos, pick0y, pick0x);
            prevRegion = currRegion;
            currRegion.x0 = pick0x; currRegion.x1 = pick0x;
            currRegion.y0 = pick0y; currRegion.y1 = pick0y;
        }
    }

    // for translating an existing region, left click in the region and drag
    if(event->buttons() == Qt::LeftButton)
    {
        view->apply();
        if(scene->getHighResTerrain()->pick(x, y, view, pnt))
        {
            // in terrain bounds, now check region bounds
            // convert to grid coordinates
            pickPos = pnt;
            scene->getHighResTerrain()->toGrid(pickPos, pick0y, pick0x);

            if(pick0x < currRegion.x1 && pick0x >= currRegion.x0 && pick0y < currRegion.y1 && pick0y >= currRegion.y0)
            {
                 prevRegion = currRegion;
                 pickOnTerrain = true;
            }
        }
    }

    if(pickOnTerrain)
    {
        update();
        lastPos = event->pos();
    }
}

void GLOverview::mouseMoveEvent(QMouseEvent *event)
{
    float nx, ny, W, H;

    int x = event->x();
    int y = event->y();

    W = (float) width();
    H = (float) height();

    // adjust shape of the region selection
    if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
    {
        if(pickOnTerrain)
        {
            vpPoint pnt;
            view->apply();
            scene->getHighResTerrain()->pick(x, y, view, pnt); // okay to move out of bounds, with edge clipping
            pickPos = pnt;

            // convert to grid coordinates
            scene->getHighResTerrain()->toGrid(pickPos, pick1y, pick1x);
            if(pick1x < pick0x)
            {
                currRegion.x0 = pick1x; currRegion.x1 = pick0x;
            }
            else
            {
                currRegion.x0 = pick0x; currRegion.x1 = pick1x;
            }

            if(pick1y < pick0y)
            {
                currRegion.y0 = pick1y; currRegion.y1 = pick0y;
            }
            else
            {
                currRegion.y0 = pick0y; currRegion.y1 = pick1y;
            }
        }
    }

    // adjust position of the region selection
    if(event->buttons() == Qt::LeftButton)
    {
        if(pickOnTerrain)
        {
            vpPoint pnt;
            int delx, dely, dx, dy;
            view->apply();
            scene->getHighResTerrain()->pick(x, y, view, pnt); // okay to move out of bounds, with edge clipping
            pickPos = pnt;

            // convert to grid coordinates
            scene->getHighResTerrain()->toGrid(pickPos, pick1y, pick1x);
            scene->getHighResTerrain()->getGridDim(dx, dy);

            delx = pick1x - pick0x ;
            dely = pick1y - pick0y;

            // bound delta to prevent move off the terrain edge
            int newx0, newx1, newy0, newy1;
            newx0 = prevRegion.x0 + delx;
            newx1 = prevRegion.x1 + delx;
            newy0 = prevRegion.y0 + dely;
            newy1 = prevRegion.y1 + dely;
            if(newx0 < 0)
                delx += newx0 * -1;
            if(newx1 >= dx)
                delx -= newx1 - dx + 1;
            if(newy0 < 0)
                dely += newy0 * -1;
            if(newy1 >= dy)
                dely -= newy1 - dy + 1;

            currRegion.x0 = prevRegion.x0 + delx; currRegion.x1 = prevRegion.x1 + delx;
            currRegion.y0 = prevRegion.y0 + dely; currRegion.y1 = prevRegion.y1 + dely;
        }
    }

    if(pickOnTerrain)
    {
        update();
        lastPos = event->pos();
    }
}

void GLOverview::mouseReleaseEvent(QMouseEvent *event)
{
    if(pickOnTerrain)
    {
        int Xwd = currRegion.x1 - currRegion.x0 + 1;
        int Ywd = currRegion.y1 - currRegion.y0 + 1;

        // restore previous state if window is not valid or sides less than 150 samples
        if (!isSelectionValid(currRegion) || Xwd < 150 || Ywd < 150)
        {
            currRegion = prevRegion;
            update();
        }
        //else
        //    signalExtractNewSubTerrain(widgetId, currRegion.x0, currRegion.y0, currRegion.x1, currRegion.y1);

        pickOnTerrain = false;
        //update(); --- will cause crash since extract....() rebulds asynchronously and may be incomplete when paint() event fires.
    }
}

void GLOverview::wheelEvent(QWheelEvent * wheel)
{
    /*
    float del, aspect;

    QPoint pix = wheel->pixelDelta();
    QPoint deg = wheel->angleDelta();

    if(!pix.isNull()) // screen resolution tracking, e.g., from magic mouse
    {
        del = (float) pix.y() * 2.0f;
    }
    else if(!deg.isNull()) // mouse wheel instead
    {
        del = (float) deg.y()/8;
    }
    else
        del = 0.0;

    Region currentRegion;
    currentRegion = currRegion; // getScene()->getSelectedRegion(); //  (region: startx, starty, endx, endy);
    int X0, Y0, X1, Y1, Xwd,Ywd;
    int centreX, centreY;

    aspect = float(currentRegion.x1 - currentRegion.x0)/float(currentRegion.y1 - currentRegion.y0);

    centreX = int( (currentRegion.x0 + currentRegion.x1)/2);
    centreY = int( (currentRegion.y0 + currentRegion.y1)/2);

    //del /= 360; // scale this to allow slower expansion/contraction
    // prerserve aspect ratio
    del = (del/10);
    Ywd = centreY - currentRegion.y0;
    Ywd += del;
    if (Ywd < 50) Ywd = 50;
    Xwd = int(aspect*Ywd);
    // std::cerr << "@@@@@ - (XWd, Ywd, deg) =  (" << Xwd << "," << Ywd << "," << del << ")\n";
    X0 = centreX - Xwd;
    X1 = centreX + Xwd;
    Y0 = centreY - Ywd;
    Y1 = centreY + Ywd;

    // bounds check

    currentRegion.x0 = X0;
    currentRegion.y0 = Y0;
    currentRegion.x1 = X1;
    currentRegion.y1 = Y1;

    Xwd = X1-X0+1;
    Ywd = Y1-Y0+1;

    // valid window and no side can be less than 50 samples
    if (isSelectionValid(currentRegion) && Xwd > 50 && Ywd > 50)
    {
        //std::cerr << "---%%%%%-- mouse wheel - new region valid: [" << currentRegion.x0 << ","
        //          <<   currentRegion.y0 << "," << currentRegion.x1 << "," <<
        //               currentRegion.y1 << "]\n";
        currRegion = currentRegion;
        signalExtractNewSubTerrain(widgetId, currRegion.x0, currRegion.y0, currRegion.x1, currRegion.y1);
    }

    //signalRepaintAllGL();
    update();
    */
}


