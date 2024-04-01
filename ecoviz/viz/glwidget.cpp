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
#include <QGLFramebufferObject>
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

GLWidget::GLWidget(const QGLFormat& format, Window * wp, Scene * scn, Transect * trans, const std::string &widName, QWidget *parent)
    : QGLWidget(format, parent)
{
    wname = widName;
    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    vizpopup = new QLabel();
    atimer = new QTimer(this);
    connect(atimer, SIGNAL(timeout()), this, SLOT(animUpdate()));

    rtimer = new QTimer(this);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(rotateUpdate()));
    glformat = format;

    setParent(wp);

    // setup transect creation state
    trc = new TransectCreation();
    trc->trx = trans;
    trc->trxstate = -1;
    trc->showtransect = true;

    view = nullptr;

    setScene(scn);
    renderer = new PMrender::TRenderer(nullptr, "../viz/shaders/");

    viewlock = false;
    decalsbound = false;
    focuschange = false;
    focusviz = false;
    timeron = false;
    active = true;
    rebindplants = true;
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

    if (decalTexture != 0)	glDeleteTextures(1, &decalTexture);
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

    (* capImg) = grabFrameBuffer();
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

void GLWidget::bandCanopyHeightTexture(float mint, float maxt)
{
    scene->getTypeMap(TypeMapType::CHM)->bandCHMMap(scene->getCanopyHeightModel(), mint*mtoft, maxt*mtoft);
    focuschange = true;
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
    view->setViewScale(scene->getTerrain()->longEdgeDist());
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

    loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
    // loadTypeMap(scene->getSlope(), TypeMapType::SLOPE);

    /*
    cerr << "Pre refreshOverlay" << endl;
    scene->getTerrain()->setBufferToDirty();
    refreshOverlay();
    cerr << "Post refreshOverlay" << endl;
    winparent->rendercount++;
    updateGL();
    cerr << "Post update" << endl;*/

     refreshViews();
}

void GLWidget::changeViewMode(ViewMode vm)
{
    view->setViewMode(vm);
    view->setForcedFocus(scene->getTerrain()->getFocus());
    view->setViewScale(scene->getTerrain()->longEdgeDist());
    view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));
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

void GLWidget::loadDecals()
{
    QImage decalImg, t;

    // load image
    if(!decalImg.load(QCoreApplication::applicationDirPath() + "/../../../common/Icons/manipDecals.png"))
        cerr << QCoreApplication::applicationDirPath().toUtf8().constData() << "/../../../common/Icons/manipDecals.png" << " not found" << endl;

    // Qt prep image for OpenGL
    QImage fixedImage(decalImg.width(), decalImg.height(), QImage::Format_ARGB32);
    QPainter painter(&fixedImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(fixedImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage( 0, 0, decalImg);
    painter.end();

    t = QGLWidget::convertToGLFormat( fixedImage );

    renderer->bindDecals(t.width(), t.height(), t.bits());
    decalsbound = true;
    cerr << "decals bound" << endl;
}

int GLWidget::loadTypeMap(basic_types::MapFloat * map, TypeMapType purpose)
{
    int numClusters = 0;

    switch(purpose)
    {
        case TypeMapType::EMPTY:
            break;
        case TypeMapType::TRANSECT:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 1.0f);
            break;
        case TypeMapType::CATEGORY:
            break;
        case TypeMapType::SLOPE:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 90.0f);
            break;
        case TypeMapType::WATER:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 100.0); // 1000.0f);
            break;
        case TypeMapType::SUNLIGHT:
             numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 13.0f);
             break;
        case TypeMapType::TEMPERATURE:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 20.0f);
            break;
        case TypeMapType::CHM:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, mtoft*initmaxt);
            break;
        case TypeMapType::CDM:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 1.0f);
            break;
        default:
            break;
    }
    return numClusters;
}

void GLWidget::setMap(TypeMapType type, int mth)
{
    if(type == TypeMapType::SUNLIGHT)
        loadTypeMap(scene->getSunlight(mth), type);
    if(type == TypeMapType::WATER)
        loadTypeMap(scene->getMoisture(mth), type);
    setOverlay(type);
}

void GLWidget::initializeGL()
{
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

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
        qWarning() << "Could not enable sample buffers";

    qglClearColor(qtWhite.light());

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

    // set other render parameters
    // can set terrain colour for radiance scaling etc - check trenderer.h

    // terrain contours
    renderer->drawContours(false);
    renderer->drawGridlines(false);

    // turn on terrain type overlay (off by default); NB you can stil call methods to update terrain type,
    renderer->useTerrainTypeTexture(true);
    renderer->useConstraintTypeTexture(false);

    // use manipulator textures (decal'd)
    renderer->textureManipulators(true);

    // *** PM Render code - end ***

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_TEXTURE_2D);

    loadDecals();
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
    scale = view->getScaleFactor();
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
        trc->trxshape[0].genDashedCylinderCurve(line[0], transectradius * 0.5f * view->getScaleFactor(), tol, transectradius * view->getScaleFactor(), 10);
        createLine(&line[1], trc->trx->getClampedInnerStart(), trc->trx->getClampedInnerEnd(), hghtoffset);
        trc->trxshape[1].genCylinderCurve(line[1], transectradius * 0.5f * view->getScaleFactor(), tol, 10);
        createLine(&line[2], trc->trx->getClampedInnerEnd(), trc->trx->getBoundEnd(), hghtoffset);
        trc->trxshape[2].genDashedCylinderCurve(line[2], transectradius * 0.5f * view->getScaleFactor(), tol, transectradius * view->getScaleFactor(), 10);
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

    if(winparent->rendercount > 1)
        cerr << "Queued rendering count = " << winparent->rendercount << endl;
    winparent->rendercount = 0;

    if(active)
    {
        drawParams.clear();
        t.start();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // note: bindinstances will not work on the first render pass because setup time is needed

        if(focuschange && focusviz)
        {
            GLfloat manipCol[] = {0.325f, 0.235f, 1.0f, 1.0f};
            paintCyl(view->getFocus(), manipCol, drawParams);
        }

        if (focuschange && trc->showtransect)
        {
            ShapeDrawData sdd;

            GLfloat transectCol[] = {0.9f, 0.1f, 0.1f, 0.2f};

            if(trc->trx->getChangeFlag()) // only update the transect line when inner point positions or thickness has changed
            {
                createTransectShape(0.0f);
                loadTypeMap(trc->trx->getTransectMap(), TypeMapType::TRANSECT);
                refreshOverlay();
                trc->trx->clearChangeFlag();
                signalRebindTransectPlants(); // PCM...see if thsi works
            }

            if(trc->trxstate == 0)
            {
                // paintCyl(t1, transectCol, drawParams);
                // paintCyl(t2, transectCol, drawParams);
                paintSphere(trc->trx->getClampedInnerStart(), transectCol, drawParams);
                paintSphere(trc->trx->getClampedInnerEnd(), transectCol, drawParams);
                paintTransect(transectCol, drawParams);
                // paintCyl(trx->getCenter(), transectCol, drawParams);
                signalRebindTransectPlants(); // PCM...see if thsi works
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
        if (drawParams.size() > 0) // DEBUG: PCM
            scene->getTerrain()->updateBuffers(renderer);

        if(focuschange)
            renderer->updateTypeMapTexture(scene->getTypeMap(getOverlay())); // only necessary if the texture is changing dynamically

        renderer->draw(view);

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
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
    /*
    if(event->key() == Qt::Key_C) // 'C' to show canopy height model texture overlay
    {
        setOverlay(TypeMapType::CHM);
    }*/
    if(event->key() == Qt::Key_D || event->key() == Qt::Key_Right) // 'D' fly right
    {
        view->incrSideFly(10.0f);
        refreshViews();
    }
    /*
    if(event->key() == Qt::Key_E) // 'E' to remove all texture overlays
    {
        setOverlay(TypeMapType::EMPTY);
    }*/
    if(event->key() == Qt::Key_F) // 'F' to toggle focus stick visibility
    {
        focusviz = !focusviz;
        winparent->rendercount++;
        updateGL();
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
        updateGL();
    }
    if(event->key() == Qt::Key_P) // 'P' to toggle plant visibility
    {
        cerr << "plant visibility toggled" << endl;
        setAllPlantsVis();
        focuschange = !focuschange;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        updateGL();
    }
    */
    /*
    if(event->key() == Qt::Key_R) // 'R' to show temperature texture overlay
    {
        setOverlay(TypeMapType::TEMPERATURE);
    }
    if(event->key() == Qt::Key_S) // 'S' to show sunlight texture overlay
    {
        sun_mth++;
        if(sun_mth >= 12)
            sun_mth = 0;
        loadTypeMap(scene->getSunlight(sun_mth), TypeMapType::SUNLIGHT);
        setOverlay(TypeMapType::SUNLIGHT);
    }*/

    if(event->key() == Qt::Key_S || event->key() == Qt::Key_Down) // 'S' fly backwards
    {
        view->incrFly(40.0f);
        refreshViews();
    }
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
    }
    /*
    if(event->key() == Qt::Key_U) // 'U' toggle undergrowth display on/off
    {
        setAllPlantsVis();
        undervis = !undervis; // toggle canopy visibility
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
        winparent->rendercount++;
        updateGL();
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

    if(event->key() == Qt::Key_W || event->key() == Qt::Key_Up) // 'W' fly forward
    {
        view->incrFly(-40.0f);
        refreshViews();
    }
    /*
    if(event->key() == Qt::Key_W) // 'W' to show water texture overlay
    {
        wet_mth++;

        if(wet_mth >= 12)
            wet_mth = 0;
        loadTypeMap(scene->getMoisture(wet_mth), TypeMapType::WATER);
        setOverlay(TypeMapType::WATER);
    }*/
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
    updateGL();
}

void GLWidget::setUndergrowthVis(bool vis)
{
    setAllPlantsVis();
    undervis = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    winparent->rendercount++;
    updateGL();
}

void GLWidget::setAllSpecies(bool vis)
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
    winparent->rendercount++;
    updateGL();
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
        updateGL();
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
        updateGL();
    }
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}

template<typename T> int GLWidget::loadTypeMap(const T &map, TypeMapType purpose)
{
    int numClusters = 0;

    switch(purpose)
    {
        case TypeMapType::EMPTY:
            break;
        case TypeMapType::TRANSECT:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 1.0f);
            break;
        case TypeMapType::CATEGORY:
            break;
        case TypeMapType::SLOPE:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 90.0f);
            break;
        case TypeMapType::WATER:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 100.0); // 1000.0f);
            break;
        case TypeMapType::SUNLIGHT:
             numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 13.0f);
             break;
        case TypeMapType::TEMPERATURE:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 20.0f);
            break;
        case TypeMapType::CHM:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, mtoft*initmaxt);
            break;
        case TypeMapType::CDM:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 1.0f);
            break;
        case TypeMapType::COHORT:
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 60.0f);
            break;
        case TypeMapType::SMOOTHING_ACTION:
            std::cout << "Loading typemap SMOOTHING_ACTION..." << std::endl;
            numClusters = scene->getTypeMap(purpose)->convert(map, purpose, 2.0f);
            break;
        default:
            break;
    }
    return numClusters;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    vpPoint pnt;
    
    int x = event->x(); int y = event->y();
    float W = static_cast<float>(width()); float H = static_cast<float>(height());

    // ensure this viewport is current for unproject
    refreshViews(); // should not be necessary

    // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
    if(event->modifiers() == Qt::MetaModifier || event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton)
    {
        // arc rotate in perspective mode
  
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        lastPos = event->pos();
        view->startArcRotate(nx, ny);
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
    if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
    {
        view->apply();
        if(scene->getTerrain()->pick(sx, sy, view, pnt))
        {
            if(!decalsbound)
                loadDecals();
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

void GLWidget::forceTransect(Transect *newTrans)
{
    if (trc) delete trc;
    // clear existing texture??
    trc = new TransectCreation;
    trc->trx = newTrans;
    trc->trxstate = -1;
    trc->showtransect = true;
}


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
    signalShowTransectView();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) // place transect point
    {
        vpPoint pnt;
        int sx, sy;

        sx = event->x(); sy = event->y();
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
                updateGL();
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
                signalRepaintAllGL(); // need to also update transect view
            }
            break;
        }
        /*
        if(scene->getTerrain()->pick(sx, sy, view, pnt))
        {
            int x, y;
            scene->getTerrain()->toGrid(pnt, x, y);
            pickInfo(x, y);
        }
        */
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float nx, ny, W, H;

    int x = event->x();
    int y = event->y();

    W = (float) width();
    H = (float) height();

    // control view orientation with right mouse button or ctrl modifier key and left mouse
    if(event->buttons() == Qt::RightButton)
    {
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        view->arcRotate(nx, ny);
        lastPos = event->pos();
        refreshViews();
    }
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
    }
    else // otherwise adjust view zoom
        view->incrZoom(del);
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
        updateGL();
    }
}
