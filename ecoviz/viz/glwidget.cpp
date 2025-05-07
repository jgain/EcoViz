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

#include "glwidget.h"
#include "eco.h"

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <QGridLayout>
#include <QImage>
#include <QCoreApplication>
#include <QInputDialog>
#include <QLineEdit>


#include <fstream>
#include "data_importer/data_importer.h"
#include "data_importer/map_procs.h"
#include "cohortmaps.h"
#include "window.h"

using namespace std;

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

static int curr_cohortmap = 0;
static int curr_tstep = 1;

GLWidget::GLWidget(const QSurfaceFormat& format, Window * wp, Scene * scn, Transect * trans, const std::string &widName, mapScene *mScene, QWidget *parent)
    : QOpenGLWidget(parent)
{
    wname = widName;
    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    glformat = format;
    setFormat(glformat);
    vizpopup = new QLabel();
    atimer = new QTimer(this);
    connect(atimer, SIGNAL(timeout()), this, SLOT(animUpdate()));

    rtimer = new QTimer(this);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(rotateUpdate()));


    setParent(wp);

    // setup transect creation state
    trc = new TransectCreation();
    trc->trx = trans;
    trc->trxstate = -1;
    trc->showtransect = true;

    view = nullptr;
    mapView = new overviewWindow(mScene);

    setScene(scn);
    renderer = new PMrender::TRenderer(nullptr, ":/resources/shaders/");

    viewlock = false;
    focuschange = false;
    focusviz = false;
    timeron = false;
    active = false;
    persRotating = false;
    painted = false;
    rebindplants = true;
    overviewEnabled = true; //enabled by default - need for selection of sub-terrain in main window; can be disabled/enabled
    scf = 10000.0f;
    decalTexture = 0;
    overlay = TypeMapType::EMPTY;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    resize(sizeHint());
    setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
}

GLWidget::~GLWidget()
{
    delete atimer;
    delete rtimer;
    if(vizpopup) delete vizpopup;

    if (renderer) delete renderer;

    if (mapView) delete mapView;

    // PCM removed - this seems like it should not be here?
   // if (decalTexture != 0)	glDeleteTextures(1, &decalTexture);
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(80, 60);
}

QSize GLWidget::sizeHint() const
{
    return QSize(800, 600);
}


void GLWidget::screenCapture(QImage * capImg, QSize capSize)
{
    paintGL();
    glFlush();

    (* capImg) = grabFramebuffer();
    (* capImg) = capImg->scaled(capSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

PMrender::TRenderer * GLWidget::getRenderer()
{
    return renderer;
}

void GLWidget::refreshOverlay()
{
    renderer->updateTypeMapTexture(scene->getTypeMap(overlay), PMrender::TRenderer::typeMapInfo::PAINT, false);
}

void GLWidget::setOverlay(TypeMapType purpose)
{
    overlay = purpose;
    renderer->updateTypeMapTexture(scene->getTypeMap(overlay), PMrender::TRenderer::typeMapInfo::PAINT, true);
    refreshViews();
}

TypeMapType GLWidget::getOverlay()
{
    return overlay;
}

void GLWidget::writePaintMap(std::string paintfile)
{
    scene->getTypeMap(TypeMapType::TRANSECT)->saveToPaintImage(paintfile);
}

void GLWidget::setScene(Scene * s)
{
    scene = s;

    if (view != nullptr) delete view;

    view = new View();
    view->setForcedFocus(scene->getTerrain()->getFocus());
    view->setViewScale(scene->getTerrain()->longEdgeDist()*2.0f);
    view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));
    scf = scene->getTerrain()->getMaxExtent();
    scene->getTerrain()->setBufferToDirty();

    // transect setup
    float rw, rh;
    scene->getTerrain()->getTerrainDim(rw, rh);

    plantvis.clear();
    plantvis.resize(scene->getBiome()->numPFTypes()*3);
    for(int t = 0; t < scene->getBiome()->numPFTypes(); t++)
        plantvis[t] = true;

    canopyvis = true;
    undervis = true;

    setAllPlantsVis();
    focuschange = !focuschange;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    active = true;

    loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);

    /*
    cerr << "Pre refreshOverlay" << endl;
    scene->getTerrain()->setBufferToDirty();
    refreshOverlay();
    cerr << "Post refreshOverlay" << endl;
    winparent->rendercount++;
    update();
    cerr << "Post update" << endl;*/

     refreshViews();
     signalRepaintAllGL();
}

void GLWidget::changeViewMode(ViewMode vm)
{
    view->setViewMode(vm);
    view->setForcedFocus(scene->getTerrain()->getFocus());
    view->setViewScale(scene->getTerrain()->longEdgeDist()*2.0f);
    view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));
}

Region GLWidget::getMapRegion()
{
    return mapView->getSelectionRegion();
}

void GLWidget::unlockView()
{
    View * preview = view;
    view = new View();
    (* view) = (* preview);
}

void GLWidget::lockView(View * imposedView)
{
    if (view) delete view;
    view = imposedView;
}

void GLWidget::lockMap(Region reg)
{
    mapView->lockMap(reg);
    Region currRegion = mapView->getSelectionRegion();
    signalExtractNewSubTerrain( (wname=="left" ? 0: 1), currRegion.x0, currRegion.y0, currRegion.x1, currRegion.y1);
}



void GLWidget::loadTypeMap(basic_types::MapFloat * map, TypeMapType purpose, float range)
{
    switch(purpose)
    {
        case TypeMapType::EMPTY:
            break;
        case TypeMapType::TRANSECT:
            scene->getTypeMap(purpose)->convert(map, purpose);
            break;
        case TypeMapType::GREYRAMP:
        case TypeMapType::HEATRAMP:
        case TypeMapType::BLUERAMP:
            scene->getTypeMap(purpose)->convert(map, purpose, range);
            break;
        default:
            break;
    }
}

void GLWidget::setDataMap(int dataIdx, TypeMapType ramp, bool updatenow)
{
    if(ramp == TypeMapType::EMPTY || ramp == TypeMapType::TRANSECT) // no data texture
    {
        loadTypeMap(trc->trx->getTransectMap(), ramp);
        if(updatenow) // assuming texture state is initialized
            setOverlay(ramp);
        else // defer texture push until after first render
             overlay = ramp;
    }
    else
    {
        if(dataIdx > 0)
        {
            basic_types::MapFloat * tmpMap = new basic_types::MapFloat();

            int year = getScene()->getTimeline()->getNow()-1;
            // selected sub region compared to the whole
            Region subRegion = mapView->getSelectionRegion();
            Region superRegion = mapView->getEntireRegion();

            getScene()->getDataMaps()->extractRegion(year, dataIdx-1, superRegion, subRegion, tmpMap);
            loadTypeMap(tmpMap, ramp, getScene()->getDataMaps()->getRange(dataIdx-1));
            if(updatenow) // assuming texture state is initialized
                setOverlay(ramp);
            else // defer texture push until after first render
                overlay = ramp;
            delete tmpMap;
        }
    }
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // get context opengl-version
    qDebug() << "GL initialize....";
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid() << "; Address: " << context();;
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." <<
              context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // QSurfaceFormat glFormat = QOpenGLWidget::format();
    // if ( !glFormat.sampleBuffers() )
    //    qWarning() << "Could not enable sample buffers";

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    int mu;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mu);
    cerr << "max texture units = " << mu << endl;

    // *** PM Render code - start ***

    // To use basic shading: PMrender::TRenderer::BASIC
    // To use radianvce scaling: PMrender::TRenderer::RADIANCE_SCALING

    PMrender::TRenderer::terrainShadingModel sMod = PMrender::TRenderer::RADIANCE_SCALING;

    // set terrain shading model
    renderer->setTerrShadeModel(sMod);

    // set up light
    Vector dl = Vector(0.6f, 1.0f, 0.6f);
    dl.normalize();

    GLfloat pointLight[3] = { 0.5, 5.0, 7.0}; // side panel + BASIC lighting - FIX FOR TERRAIN HEIGHT (PCM)?
    GLfloat dirLight0[3] = { dl.i, dl.j, dl.k}; // for radiance lighting
    GLfloat dirLight1[3] = { -dl.i, dl.j, -dl.k}; // for radiance lighting

    renderer->setPointLight(pointLight[0],pointLight[1],pointLight[2]);
    renderer->setDirectionalLight(0, dirLight0[0], dirLight0[1], dirLight0[2]);
    renderer->setDirectionalLight(1, dirLight1[0], dirLight1[1], dirLight1[2]);

    // initialise renderer/compile shaders
    renderer->initShaders();
    // NB: we need an active context to compile the shders...
    mapView->getRenderer()->initShaders();

    // set other render parameters
    // can set terrain colour for radiance scaling etc - check trenderer.h

    // terrain contours
    renderer->drawContours(false);
    renderer->drawGridlines(false);

    // turn on terrain type overlay (off by default); NB you can stil call methods to update terrain type,
    renderer->useTerrainTypeTexture(true);
    renderer->useConstraintTypeTexture(false);

    // use manipulator textures (decal'd)
    renderer->textureManipulators(false);

    // *** PM Render code - end ***

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_TEXTURE_2D);

    paintGL();
}

void GLWidget::paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
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
    shape.genCappedCylinder(scale*armradius, 1.5f*scale*armradius, scale*(manipheight-manipradius), 40, 10, tfm, false);
    if(shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}

void GLWidget::paintSphere(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
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
    scale = view->getScaleFactor() * 0.5f;
    idt = glm::mat4(1.0f);
    trs = glm::vec3(p.x, p.y, p.z);
    rot = glm::vec3(1.0f, 0.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    // tfm = glm::rotate(tfm, glm::radians(-90.0f), rot);
    shape.genSphere(scale * transectradius, 40, 40, tfm);
    if(shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}

void GLWidget::createLine(vector<vpPoint> * line, vpPoint start, vpPoint end, float hghtoffset)
{
    vpPoint pnt;
    Vector del;
    float tx, ty;

    scene->getTerrain()->getTerrainDim(tx, ty);
    int steps = 200;
    del.diff(start, end);

    del.mult(1.0f / (float) steps);
    pnt = start;
    line->push_back(pnt);
    for(int j = 0; j < steps; j++)
    {
        del.pntplusvec(pnt, &pnt);
        if(pnt.x >= ty-tolzero) pnt.x = ty-tolzero;
        if(pnt.x <= tolzero) pnt.x = tolzero;
        pnt.y = 1.0f;
        if(pnt.z >= tx-tolzero) pnt.z = tx-tolzero;
        if(pnt.z <= tolzero) pnt.z = tolzero;
        line->push_back(pnt);
    }

    drapeProject(line, line, scene->getTerrain());

    // add height offset to all line positions
    for(int j = 0; j < (int) line->size(); j++)
        (* line)[j].y += hghtoffset;
}

void GLWidget::createTransectShape(float hghtoffset)
{
    vector<vpPoint> line[3];
    float tol, tx, ty;

    for(int i = 0; i < 3; i++)
        trc->trxshape[i].clear();
    scene->getTerrain()->getTerrainDim(tx, ty);
    tol = 0.001f * std::max(tx, ty);

    // generate vertices for the line and drop onto terrain
    if(active)
    {
        createLine(&line[0], trc->trx->getBoundStart(), trc->trx->getClampedInnerStart(), hghtoffset);
        trc->trxshape[0].genDashedCylinderCurve(line[0], transectradius * 0.25f * view->getScaleFactor(), tol, transectradius * view->getScaleFactor(), 10);
        createLine(&line[1], trc->trx->getClampedInnerStart(), trc->trx->getClampedInnerEnd(), hghtoffset);
        trc->trxshape[1].genCylinderCurve(line[1], transectradius * 0.25f * view->getScaleFactor(), tol, 10);
        createLine(&line[2], trc->trx->getClampedInnerEnd(), trc->trx->getBoundEnd(), hghtoffset);
        trc->trxshape[2].genDashedCylinderCurve(line[2], transectradius * 0.25f * view->getScaleFactor(), tol, transectradius * view->getScaleFactor(), 10);
    }
}

void GLWidget::paintTransect(GLfloat * col, std::vector<ShapeDrawData> &drawParams)
{
    // assumes that the transect shape has already been created
    ShapeDrawData sdd[3];
    std::vector<glm::vec3> translInstance;
    std::vector<glm::vec2> scaleInstance;
    std::vector<float> cinst;

    // update and bind shapes
    for(int i = 0; i < 3; i++)
    {
        trc->trxshape[i].setColour(col);

        if(trc->trxshape[i].bindInstances(&translInstance,&scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
        {
            sdd[i] = trc->trxshape[i].getDrawParameters();
            sdd[i].current = false;
            drawParams.push_back(sdd[i]);
        }
    }
}

void GLWidget::paintGL()
{
    std::vector<ShapeDrawData> drawParams; // to be passed to terrain renderer

    Timer t;

    // if(winparent->rendercount > 1)
    //     cerr << "Queued rendering count = " << winparent->rendercount << endl;
    winparent->rendercount = 0;

    if(active)
    {
        drawParams.clear();
        t.start();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // note: bindinstances will not work on the first render pass because setup time is needed

        if(focuschange && focusviz)
        {
            GLfloat manipCol[] = {0.325f, 0.235f, 1.0f, 1.0f};
            paintCyl(view->getFocus(), manipCol, drawParams);
        }

        if (focuschange && trc->showtransect)
        {
            //ShapeDrawData sdd;

            GLfloat transectCol[] = {0.9f, 0.1f, 0.1f, 0.2f};

            if(trc->trx->getChangeFlag()) // only update the transect line when inner point positions or thickness has changed
            {
                createTransectShape(0.0f);
                loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
                refreshOverlay();
                trc->trx->clearChangeFlag();
                signalRebindTransectPlants();
            }

            if(trc->trxstate == 0)
            {
                // paintCyl(t1, transectCol, drawParams);
                // paintCyl(t2, transectCol, drawParams);
                paintSphere(trc->trx->getClampedInnerStart(), transectCol, drawParams);
                paintSphere(trc->trx->getClampedInnerEnd(), transectCol, drawParams);
                paintTransect(transectCol, drawParams);
                // paintCyl(trx->getCenter(), transectCol, drawParams);
                signalRebindTransectPlants(); // PCM...see if this works
            }
            if(trc->trxstate == 1)
            {
                // paintCyl(t1, transectCol, drawParams);
                paintSphere(trc->t1, transectCol, drawParams);
            }


            /*
            planeshape.clear();
            planeshape.setColour(planeCol);

            std::cout << "Transect vec: " << transect_vec.x << ", " << transect_vec.y << ", " << transect_vec.z << std::endl;

            idt = glm::mat4(1.0f);
            planeshape.genPlane(transect_vec, transect_pos, transect_thickness, transect_length, idt);
            if (planeshape.bindInstances(&sinst, &cinst))
            {
                sdd = planeshape.getDrawParameters();
                sdd.current = false;
                drawParams.push_back(sdd);
            }*/
        }


        // prepare plants for rendering
        if(focuschange)
        {
            scene->getEcoSys()->bindPlantsSimplified(scene->getTerrain(), drawParams, &plantvis, rebindplants);
            rebindplants = false;
        }

        // pass in draw params for objects
        renderer->setConstraintDrawParams(drawParams);

        // draw terrain and plants
        renderer->forceTextureRebind();
        //scene->getTerrain()->setBufferToDirty();

        if (drawParams.size() > 0) // DEBUG: PCM
        {
            scene->getTerrain()->updateBuffers(renderer);
            // bind textures
            //renderer->updateTypeMapTexture(scene->getTypeMap(overlay), PMrender::TRenderer::typeMapInfo::PAINT, true);
        }

        if(focuschange)
            renderer->updateTypeMapTexture(scene->getTypeMap(getOverlay())); // only necessary if the texture is changing dynamically

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer->draw(view);

        // ** overview map draw : draw on resrtricted viewport:
        // ** can be turned off - but no terrain selection until re-enabled.

        if (overviewEnabled)
        {
            int wd, ht;
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            // PCM fix inital aspect mismatch in overview
            if (mapView->isTerrainReady() == false)
            {
                mapView->setWindowSize();
                mapView->resetViewDims();
                mapView->setTerrainReady(true);
            }

            // mapView->getWindowSize(wd,ht); // JG - viewport and window coordinates are different on Apple
            mapView->getViewSize(viewport[2], wd, ht);
            GLint newport[4];

            newport[0] = (GLint)(viewport[2] - wd);
            newport[1] = (GLint)(viewport[3] - ht);
            newport[2] = (GLint)wd;
            newport[3] = (GLint)ht;

            /*
        cerr << "newport = " << newport[0] << ", " << newport[1] << ", " << newport[2] << ", " << newport[3] << endl;
        cerr << "oldport = " << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " << viewport[3] << endl;
        cerr << "olddim = " << this->width() << ", " << this->height() << endl;
        cerr << "newdim = " << wd << ", " << ht << endl;
        */

            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glEnable(GL_SCISSOR_TEST);
            //glViewport(viewport[0], viewport[1], viewport[2]-500, viewport[3]-500);
            //glScissor(viewport[0], viewport[1], viewport[2]-300, viewport[3]-300);
            //GLint wdim = viewport[2]-600, hdim = viewport[3]-600;
            //GLint wpos = viewport[0], hpos = viewport[1];
            //glViewport(wpos, hpos,  wdim, hdim);
            glViewport(newport[0], newport[1], newport[2], newport[3]);
            mapView->draw();
            //glDisable(GL_SCISSOR_TEST);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            //QThread::msleep(1000);
            // ** end overview map draw **
        }

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
        /*
        if(!painted)
            cerr << "first paint" << endl;
        if(!painted)
        {
            painted = true;
            signalUpdateOverviews();
            cerr << "update overview signalled" << endl;
        }
       */
    }
}

void GLWidget::resizeGL(int width, int height)
{
    // TO DO: fix resizing
    // int side = qMin(width, height);
    // glViewport((width - side) / 2, (height - side) / 2, width, height);
    glViewport(0, 0, width, height);

    view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
    view->apply();
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    /*
    if(event->key() == Qt::Key_Right)
    {
    }
    if(event->key() == Qt::Key_Left)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
    }
    if(event->key() == Qt::Key_Down)
    {
    }*/
    if(event->key() == Qt::Key_A || event->key() == Qt::Key_Left) // 'A' fly left
    {
        view->incrSideFly(-10.0f);
        refreshViews();
    }
    if(event->key() == Qt::Key_D || event->key() == Qt::Key_Right) // 'D' fly right
    {
        view->incrSideFly(10.0f);
        refreshViews();
    }
    if(event->key() == Qt::Key_E) // 'E' to remove all texture overlays
    {
        setOverlay(TypeMapType::TRANSECT);
    }
    if(event->key() == Qt::Key_F) // 'F' to toggle focus stick visibility
    {
        focusviz = !focusviz;
        winparent->rendercount++;
        update();
    }

    if(event->key() == Qt::Key_N) // 'N' to save overview map selection
    {
         Region savereg = mapView->getSelectionRegion();
         View saveview = (* view);
         viewScene viewscene(savereg, saveview);
         viewscene.save("viewscene.txt");

         cerr << "Overview region saved" << endl;
         cerr << " Subregion stored -  [x0,y0,x1,y1] = [" << savereg.x0 << "," << savereg.y0 << "," <<
                savereg.x1 << "," << savereg.y1 << "]" << endl;
    }

    if(event->key() == Qt::Key_M) // 'M' to toggle ovewviewmap
    {
        overviewEnabled = ! overviewEnabled; // toggle ovewviewmap
        winparent->rendercount++;
        update();
    }

    /*
    if(event->key() == Qt::Key_N) // 'N' to toggle display of canopy trees on or off
    {
        cerr << "canopy visibility toggled" << endl;
        setAllPlantsVis();
        canopyvis = !canopyvis; // toggle canopy visibility
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        update();
    }
    if(event->key() == Qt::Key_P) // 'P' to toggle plant visibility
    {
        cerr << "plant visibility toggled" << endl;
        setAllPlantsVis();
        focuschange = !focuschange;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        update();
    }
    */

    if(event->key() == Qt::Key_S || event->key() == Qt::Key_Down) // 'S' fly backwards
    {
        view->incrFly(40.0f);
        refreshViews();
    }/*
    if(event->key() == Qt::Key_T) // 'T' to toggle transect display on/off
    {
        trc->showtransect = !trc->showtransect;
        if(trc->showtransect && trc->trxstate == 0)
        {
            loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
            setOverlay(TypeMapType::TRANSECT);
        }
        else
        {
            setOverlay(TypeMapType::EMPTY);
        }
    }*/
    if(event->key() == Qt::Key_T) // 'T' to toggle texture on
    {
        basic_types::MapFloat * tmpMap = new basic_types::MapFloat();

        // extract map
        // get current year
        int year = 0;
        Region subRegion = mapView->getSelectionRegion();
        Region superRegion = mapView->getEntireRegion();

        // cerr << "superRegion = " << superRegion.x0 << ", " << superRegion.y0 << " -> " << superRegion.x1 << ", " << superRegion.y1 << endl;
        // cerr << "subRegion = " << subRegion.x0 << ", " << subRegion.y0 << " -> " << subRegion.x1 << ", " << subRegion.y1 << endl;

        getScene()->getDataMaps()->extractRegion(getScene()->getTimeline()->getNow()-1, 0, superRegion, subRegion, tmpMap);
        loadTypeMap(tmpMap, TypeMapType::HEATRAMP, getScene()->getDataMaps()->getRange(0));
        setOverlay(TypeMapType::HEATRAMP);
        delete tmpMap;
    }
    /*
    if(event->key() == Qt::Key_U) // 'U' toggle undergrowth display on/off
    {
        setAllPlantsVis();
        undervis = !undervis; // toggle canopy visibility
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        update();
    }*/

    if(event->key() == Qt::Key_V) // 'V' for top-down view
    {
        scene->getTerrain()->setMidFocus();
        view->setForcedFocus(scene->getTerrain()->getFocus());
        view->topdown();
        refreshViews();
    }
    /*

    if(event->key() == Qt::Key_V) // 'V' for switching view modes
    {
        view->switchViewMode();
        refreshViews();
    }
   */

    /*
    if(event->key() == Qt::Key_M) // 'M' save camera matrices (view, projection, and product)
    {
        Region src;
        float sx, sy, ex, ey, pdimX, pdimY;
        bool flag = scene->getTerrain()->getSourceRegion(src, sx, sy, ex, ey, pdimX, pdimY);
        if (!flag)
        {
            std::cerr << "keyPressEvent (m) - source region undefined, camera matrix unreliable.\n";
        }

        view->saveCameraMatrices(wname, sx, sy);
        std::cerr << "\nCamera matrix saved for " << wname << "\n";
    }
    */

    if(event->key() == Qt::Key_W || event->key() == Qt::Key_Up) // 'W' fly forward
    {
        view->incrFly(-40.0f);
        refreshViews();
    }
}

void GLWidget::setAllPlantsVis()
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = true;
}

void GLWidget::setCanopyVis(bool vis)
{
    setAllPlantsVis();
    canopyvis = vis; // toggle canopy visibility
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    winparent->rendercount++;
    update();
}

void GLWidget::setUndergrowthVis(bool vis)
{
    setAllPlantsVis();
    undervis = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    winparent->rendercount++;
    update();
}

void GLWidget::setAllSpecies(bool vis)
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    winparent->rendercount++;
    update();
}

void GLWidget::setSinglePlantVis(int p)
{
    if(p < (int) plantvis.size())
    {
        for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
            plantvis[i] = false;
        plantvis[p] = true;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        update();
    }
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}

void GLWidget::toggleSpecies(int p, bool vis)
{
    if(p < static_cast<int>(plantvis.size()))
    {
        plantvis[p] = vis;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        update();
    }
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}

template<typename T> void GLWidget::loadTypeMap(const T &map, TypeMapType purpose, float range)
{
    switch(purpose)
    {
        case TypeMapType::EMPTY:
            break;
        case TypeMapType::TRANSECT:
        case TypeMapType::GREYRAMP:
        case TypeMapType::HEATRAMP:
        case TypeMapType::BLUERAMP:
            scene->getTypeMap(purpose)->convert(map, purpose, range);
            break;
        default:
            break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    vpPoint pnt;
    
    int sx = event->x(); int sy = event->y();
    float W = static_cast<float>(width()); float H = static_cast<float>(height());

    // ensure this viewport is current for unproject
    refreshViews(); // should not be necessary

    if(mapView->mouseInOverView(width(), height(), sx, sy) && overviewEnabled)
    {
        int ox, oy;

        // transform mouse position to overview window coordinates
        mapView->mouseCoordTransform(width(), height(), sx, sy, ox, oy);

        // for specifying a new region from scratch, right click and drag
        if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
            mapView->startRegionDemarcation(ox, oy);

        // for translating an existing region, left click in the region and drag
        if(event->buttons() == Qt::LeftButton)
            mapView->startRegionTranslate(ox, oy);

        if(mapView->getPickOnTerrain())
            update();
    }
    else
    {
        // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
        if(event->modifiers() == Qt::MetaModifier || event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton)
        {
            // arc rotate in perspective mode
            // convert to [0,1] X [0,1] domain
            nx = (2.0f * (float) sx - W) / W;
            ny = (H - 2.0f * (float) sy) / H;
            lastPos = event->pos();
            view->startArcRotate(nx, ny);
            persRotating = true;
        }
    }

    lastPos = event->pos();
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // set the focus for arcball rotation
    // pick point on terrain or zero plane if outside the terrain bounds
    vpPoint pnt;
    int sx, sy;
    
    sx = event->x(); sy = event->y();

    // double click in overview should do nothing
    if(mapView->mouseInOverView(width(), height(), sx, sy) && overviewEnabled)
        return;

    if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
    {
        view->apply();
        if(scene->getTerrain()->pick(sx, sy, view, pnt))
        {
            vpPoint pickpnt = pnt;
            view->setAnimFocus(pickpnt);
            scene->getTerrain()->setFocus(pickpnt);
            cerr << "Pick Point = " << pickpnt.x << ", " << pickpnt.y << ", " << pickpnt.z << endl;
            focuschange = true; focusviz = true;
            atimer->start(10);
        }
        else
        {
            cerr << "Terrain missed" << endl;
        }
        // ignores pick if terrain not intersected, should possibly provide error message to user
    }
}

void GLWidget::pickInfo(int x, int y)
{
   std::string catName;

   cerr << endl;
   cerr << "*** PICK INFO ***" << endl;
   cerr << "location: " << x << ", " << y << endl;
   cerr << "Elevation: " << scene->getTerrain()->getHeight(y, x) << endl;
   // getSim()->pickInfo(x, y);
   //cerr << "Canopy Height (m): " << getCanopyHeightModel()->get(x, y) * 0.3048f  << endl;
   //cerr << "Canopy Density: " << getCanopyDensityModel()->get(x, y) << endl;
   //cerr << "Sunlight: " << getSunlight(sun_mth)->get(x,y) << endl;
   //cerr << "Moisture: " << getMoisture(wet_mth)->get(x,y) << endl;
}

void GLWidget::setTransectCreate(TransectCreation * newtrc)
{
    if(trc != nullptr)
        delete trc;
    trc = newtrc;

    // align rendering
    if(trc->trxstate == 0)
    {
        trc->trx->derive(trc->t1, trc->t2, scene->getTerrain());
        createTransectShape(0.0f);
        loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
        setOverlay(TypeMapType::TRANSECT);
    }
    if(trc->trxstate == 1 || trc->trxstate == -1)
    {
        setOverlay(TypeMapType::EMPTY);
        trc->trx->setValidFlag(false);
    }
    signalShowTransectView();
    winparent->rendercount++;
    signalRepaintAllGL(); // need to also update transect view
}

void GLWidget::seperateTransectCreate(Transect * trx)
{
    TransectCreation * newtrc = new TransectCreation;
    (* newtrc) = (* trc);
    newtrc->trx = trx;
    trc = newtrc;
}

/*
void GLWidget::forceTransect(Transect *newTrans)
{
    if (trc) delete trc;
    // clear existing texture??
    trc = new TransectCreation;
    trc->trx = newTrans;
    trc->trxstate = -1;
    trc->showtransect = true;
}
*/

void GLWidget::pointPlaceTransect(bool firstPoint)
{
    if(firstPoint)
    {
       setOverlay(TypeMapType::EMPTY);
    }
    else
    {
        createTransectShape(0.0f);
        loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
        setOverlay(TypeMapType::TRANSECT);
    }
    signalSyncDataMap();
    signalShowTransectView();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int sx, sy;
    sx = event->x(); sy = event->y();

    persRotating = false;
    if( (mapView->mouseInOverView(width(), height(), sx, sy) || mapView->getPickOnTerrain() ) && overviewEnabled)
    {
        persRotating = false;
        if(mapView->endRegionChange())
        {
            int i;
            // now apply change
            Region currRegion = mapView->getSelectionRegion();
            if(viewlock)
                i = 2; // signal change to both perspective views
            else
                i = (wname=="left" ? 0: 1);
            cerr << "REGION = " << currRegion.x0 << ", " << currRegion.y0 << " - " << currRegion.x1 << ", " << currRegion.y1 << endl;
            cerr << "SIGNAL = " << i << endl;
            signalExtractNewSubTerrain(i, currRegion.x0, currRegion.y0, currRegion.x1, currRegion.y1);
        }
    }
    else
    {
        if(event->button() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) // place transect point
        {
            vpPoint pnt;

            trc->showtransect = true;
            switch(trc->trxstate)
            {
            case -1:
            case 0: // placement of initial point
                if(scene->getTerrain()->pick(sx, sy, view, pnt))
                {
                    trc->t1 = pnt;
                    trc->trxstate = 1;
                    trc->trx->setValidFlag(false);
                    pointPlaceTransect(true);
                    signalSyncPlace(true);
                    winparent->rendercount++;
                    update();
                }
                break;
            case 1: // placement of final point
                if(scene->getTerrain()->pick(sx, sy, view, pnt))
                {
                    trc->t2 = pnt;
                    trc->trxstate = 0;
                    trc->trx->derive(trc->t1, trc->t2, scene->getTerrain());
                    pointPlaceTransect(false);
                    signalSyncPlace(false);
                    winparent->rendercount++;
                    signalRebindTransectPlants();
                    signalRepaintAllGL(); // need to also update transect view
                }
                break;
            }
        }
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float nx, ny, W, H;

    int x = event->x();
    int y = event->y();

    W = (float) width();
    H = (float) height();

    // cerr << "mouse pos = " << x << ", " << y << endl;
    // cerr << "in overview = " << mapView->mouseInOverView(W, H, x, y);
    // control view orientation with right mouse button or ctrl modifier key and left mouse

    if(mapView->mouseInOverView(width(), height(), x, y) && overviewEnabled)
    {
        int ox, oy;

        // transform mouse position to overview window coordinates
        mapView->mouseCoordTransform(width(), height(), x, y, ox, oy);

        // adjust shape of the region selection
        if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
            mapView->continueRegionDemarcation(ox, oy);

        // adjust position of the region selection
        if(event->buttons() == Qt::LeftButton)
            mapView->continueRegionTranslate(ox, oy);

        if(mapView->getPickOnTerrain())
            update();
    }
    else
    {
        if(persRotating && event->buttons() == Qt::RightButton)
        {
            // convert to [0,1] X [0,1] domain
            nx = (2.0f * (float) x - W) / W;
            ny = (H - 2.0f * (float) y) / H;
            view->arcRotate(nx, ny);

            refreshViews();
        }
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent * wheel)
{
    float del;
 
    QPoint pix = wheel->pixelDelta();
    QPoint deg = wheel->angleDelta();

    if(!pix.isNull()) // screen resolution tracking, e.g., from magic mouse
    {
        del = (float) pix.y() * 10.0f;
    }
    else if(!deg.isNull()) // mouse wheel instead
    {
        del = (float) -deg.y() * 5.0f;
    }
    // cerr << "del = " << del << endl;
    if(wheel->modifiers() == Qt::ControlModifier) // adjust transect width
    {
        del /= 60.0f;
        trc->trx->setThickness(trc->trx->getThickness()+del, scene->getTerrain());
        signalRebindTransectPlants(); // PCM...see if this works
        signalRepaintAllGL();
    }
    else // otherwise adjust view zoom
    {
        view->incrZoom(del);
        trc->trx->setChangeFlag(); // render thickness of transects depends on zoom
    }
    refreshViews();
}

void GLWidget::animUpdate()
{
    if(view->animate())
         refreshViews();
}

void GLWidget::rotateUpdate()
{
    if(view->spin())
        refreshViews();
}

void GLWidget::rebindPlants()
{
    rebindplants = true;
}

void GLWidget::refreshViews()
{
    if(viewlock)
    {
        winparent->rendercount++;
        signalRepaintAllGL();
    }
    else
    {
        winparent->rendercount++;
        update();
    }
}

/// overviewmap methods: these methods refer to the part of the main viewport on which the map is overdrawn

overviewWindow::overviewWindow(mapScene * scn)
{
    currRegion = Region(0,0,0,0);
    mapWidth = mapHeight = 0;

    mview = nullptr;

    ovw = 50; ovh = 50;

    setScene(scn);
    active = true;
    timeron = false;
    pickOnTerrain = false;
    perscale = 0.3f;
    terrainReady = false;

    mrenderer = new PMrender::TRenderer(nullptr, ":/resources/shaders/");


    initializeMapRenderer();
}

overviewWindow::~overviewWindow()
{
    if (mrenderer) delete mrenderer;
    if (mview) delete mview;
}

void overviewWindow::setWindowSize(void)
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
}


void overviewWindow::setScene(mapScene * s)
{
    scene = s;

    if (mview != nullptr) delete mview;

    mview = new View();

    setWindowSize();

    scene->getLowResTerrain()->setMidFocus();
    mview->setViewType(ViewState::ORTHOGONAL);
    mview->setForcedFocus(scene->getLowResTerrain()->getFocus());
    mview->setDim(0.0f, 0.0f, ovw, ovh);
    scf = scene->getLowResTerrain()->getMaxExtent();

    mview->setOrthoViewExtent(scene->getLowResTerrain()->longEdgeDist());

    std::cout << "***** setScene(mapScene) - data:\n";
    vpPoint pt = scene->getLowResTerrain()->getFocus();
    std::cout << " ** focus.x: " << pt.x;
    std::cout << " ** focus.y: " << pt.y;
    std::cout << " ** focus.z: " << pt.z;
    std::cout << " ** scf: " << scf << "\n";
    std::cout << " ** wd: " << ovw << "\n";
    std::cout << " ** ht: " << ovh << "\n";
    std::cout << " ** terrain scale: " << scene->getLowResTerrain()->longEdgeDist() << "\n";

    // scene->getLowResTerrain()->setMidFocus();
    mview->topdown();

    scene->getLowResTerrain()->setBufferToDirty();

    //winparent->rendercount++;
    //signalRepaintAllGL();
    //update();
}

// the heightfield will change after initial dummy creation (and possible edits later)
// make sure View params are correct.

void overviewWindow::updateViewParams(void)
{
    scene->getLowResTerrain()->setMidFocus();
    mview->setForcedFocus(scene->getLowResTerrain()->getFocus());
    // view->setViewScale(scene->getLowResTerrain()->longEdgeDist());
    scf = scene->getLowResTerrain()->getMaxExtent();

    // orthogonal rendering
    mview->setViewType(ViewState::ORTHOGONAL);
    float minHt, maxHt;
    scene->getHighResTerrain()->getHeightBounds(minHt, maxHt); // need *global* max height

    vpPoint orthoFocusTop;
    orthoFocusTop = scene->getLowResTerrain()->getFocus();
    orthoFocusTop.y = 1.1* maxHt + 100.0f; // PCM: for some reason the camera near plane clips even when using maxHt
    // could be the slight near plane offset e=0.01, but that is small and an addiive offset should have fixed that.
    // Possible issue?

    // std::cerr << "overviewWindow: Camera front clipping plane height: " << orthoFocusTop.y << std::endl;
    mview->setForcedFocus(orthoFocusTop);
    mview->setOrthoViewDepth(100000.0f); // make this large to avoid issues!
    mview->setOrthoViewExtent(scene->getLowResTerrain()->longEdgeDist()); // this scales to fit in window

    mview->topdown();
    mview->setZoomdist(0.0f);

    mview->apply();
}

void overviewWindow::unlockMap()
{
    /*
    View * preview = view; // PCM: likely a leak of Viewe object at some point
    view = new View();
    (* view) = (* preview);

    trx = imposedTrx; // pointer managed externally so no need to delete previous
    */
}

void overviewWindow::lockMap(Region reg)
{
    // copy accross inset region and signal redraw
    currRegion.x0 = reg.x0; currRegion.x1 = reg.x1;
    currRegion.y0 = reg.y0; currRegion.y1 = reg.y1;
}

// this just sets renderer state - not VBOs, textures etc - those need to be set from glwidget,
// at initialization.Both the renderers share the current graphics context (glwidget in the case)
void overviewWindow::initializeMapRenderer(void)
{

    // *** PM Render code - start ***

    PMrender::TRenderer::terrainShadingModel  sMod = PMrender::TRenderer::RADIANCE_SCALING_OVERVIEW;

    // set terrain shading model
    mrenderer->setTerrShadeModel(sMod);

    // set up light
    Vector dl = Vector(0.6f, 1.0f, 0.6f);
    dl.normalize();

    //GLfloat pointLight[3] = {0.5, 5.0, 7.0}; // side panel + BASIC lighting
    GLfloat pointLight[3] = {1000.0, 3000.0, 1000.0}; // side panel + BASIC lighting
    GLfloat dirLight0[3] = { dl.i, dl.j, dl.k}; // for radiance lighting
    GLfloat dirLight1[3] = { -dl.i, dl.j, -dl.k}; // for radiance lighting

    mrenderer->setPointLight(pointLight[0],pointLight[1],pointLight[2]);
    mrenderer->setDirectionalLight(0, dirLight0[0], dirLight0[1], dirLight0[2]);
    mrenderer->setDirectionalLight(1, dirLight1[0], dirLight1[1], dirLight1[2]);

    // initialise renderer/compile shaders
    // renderer->initShaders();

    // set other render parameters
    // can set terrain colour for radiance scaling etc - check trenderer.h

    // terrain contours
    mrenderer->drawContours(false);
    mrenderer->drawGridlines(false);

    // turn on terrain type overlay (off by default); NB you can stil call methods to update terrain type,
    mrenderer->useTerrainTypeTexture(false);
    mrenderer->useConstraintTypeTexture(false);

    // use manipulator textures (decal'd)
    mrenderer->textureManipulators(false);

    // overlay - load??
    //paintGL();
}

void overviewWindow::paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
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
    scale = mview->getScaleFactor();
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

void overviewWindow::paintSelectionPlane(GLfloat *col, std::vector<ShapeDrawData> &drawparams)
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
    //               "," << region.x1 << "," << region.y1 << "]\n";

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

void overviewWindow::paintSphere(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
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
    scale = mview->getScaleFactor();
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

void overviewWindow::draw(void)
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
    //GLfloat pickCol[] = {1.0f, 0.2f, 0.2f, 1.0f};

    Timer t;

    if(active)
    {
        drawParams.clear();
        t.start();
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // viewport is incorrect on creation, this will ensure current is used to match View
        updateViewParams();

        // build selection plane 'manipulator'
        if(pickOnTerrain)
            paintSelectionPlane(purpleish, drawParams);
        else
            paintSelectionPlane(blueish, drawParams);

        // pass in draw params for objects
        mrenderer->setConstraintDrawParams(drawParams);

        // draw terrain

        mrenderer->forceTextureRebind(); // because we have two renderers looking at this openGl context
        //scene->getLowResTerrain()->setBufferToDirty();
        // draw terrain  with selection plane
        if (drawParams.size() > 0) // DEBUG: PCM
            scene->getLowResTerrain()->updateBuffers(mrenderer);

        mrenderer->draw(mview);

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
    }
}

void overviewWindow::mouseCoordTransform(int w, int h, int sx, int sy, int &ox, int &oy)
{
    int ix, iy, ow, oh;

    // transform to independent [0,1] coordinates for x and y
    getViewSize(w, ow, oh);
    ix = sx - (w - ow);
    iy = sy;

    ox = (int) ((float) ovw * (float) ix / (float) ow);
    oy = (int) ((float) ovh * (float) iy / (float) oh);
}

void overviewWindow::startRegionDemarcation(int x, int y)
{
    vpPoint pnt;

    mview->apply();
    if(scene->getHighResTerrain()->pick(x, y, mview, pnt))
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

void overviewWindow::startRegionTranslate(int x, int y)
{
    vpPoint pnt;

    mview->apply();
    if(scene->getHighResTerrain()->pick(x, y, mview, pnt))
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

void overviewWindow::continueRegionDemarcation(int x, int y)
{
    if(pickOnTerrain)
    {
        vpPoint pnt;
        mview->apply();
        scene->getHighResTerrain()->pick(x, y, mview, pnt); // okay to move out of bounds, with edge clipping
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

void overviewWindow::continueRegionTranslate(int x, int y)
{
    if(pickOnTerrain)
    {
        vpPoint pnt;
        int delx, dely, dx, dy;
        mview->apply();
        scene->getHighResTerrain()->pick(x, y, mview, pnt); // okay to move out of bounds, with edge clipping
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


bool overviewWindow::endRegionChange()
{
    bool updateRegion = false;

    if(pickOnTerrain)
    {
        int Xwd = currRegion.x1 - currRegion.x0 + 1;
        int Ywd = currRegion.y1 - currRegion.y0 + 1;

        // restore previous state if window is not valid or sides less than 150 samples
        if (!isSelectionValid(currRegion) || Xwd < 150 || Ywd < 150)
            currRegion = prevRegion;
        else
            updateRegion = true;

        pickOnTerrain = false;
    }
    return updateRegion;
}


