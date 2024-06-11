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

#include "gltransect.h"
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
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QRunnable>
#include <QThreadPool>

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

GLTransect::GLTransect(const QGLFormat& format, Window * wp, Scene * scn, Transect * trans, QWidget *parent)
    : QGLWidget(format, parent)
{
    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    glformat = format;

    view = nullptr;

    renderer = new PMrender::TRenderer(nullptr, "../viz/shaders/");

    setParent(wp);

    trx = trans;
    setScene(scn);
    viewlock = false;
    decalsbound = false;
    focuschange = false;
    timeron = false;
    active = false;
    rebindplants = true;
    forceRebindPlants = true;

    scf = 10000.0f;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    resize(sizeHint());
    setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void GLTransect::switchTransectScene(Scene *newScene, Transect *newTransect)
{
    trx = newTransect;
    viewlock = false;
    decalsbound = false;
    timeron = false;
    rebindplants = true;
    scf = 10000.0f;

    setScene(newScene);

    focuschange = true;
    forceRebindPlants = true;
}

GLTransect::~GLTransect()
{
    if (renderer) delete renderer;
}

QSize GLTransect::minimumSizeHint() const
{
    return QSize(80, 15);
}

QSize GLTransect::sizeHint() const
{
    return QSize(800, 150);
}

PMrender::TRenderer * GLTransect::getRenderer()
{
    return renderer;
}

void GLTransect::updateTransectView()
{
    float tx, ty;
    vpPoint basePlaneOrigin;

    // extract planes for world space culling and retrieve origin on one of the planes
    // (which will map to the near plane for camera)

    std::pair<Plane, Plane> planes  = trx->getTransectPlanes(basePlaneOrigin);
    transectPlanes.clear();
    transectPlanes.push_back(planes.first);
    transectPlanes.push_back(planes.second);

    view->setOrthoViewExtent(trx->getExtent());
    view->setOrthoViewDepth(trx->getThickness());
    // scene->getTerrain()->setMidFocus();
    // view->setForcedFocus(trx->getCenter());
    view->setForcedFocus(basePlaneOrigin);
    // view->setForcedFocus(scene->getTerrain()->getFocus());
    view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));

    Vector v, n;
    v = Vector(0.0f, 0.0f, 1.0f);
    n = trx->getNormal();
    float r = atan2(n.k, n.i) - atan2(v.k, v.i);

    // cerr << "rotation angle = " << -r << endl;
    view->flatview(-r);

    // cerr << "extent = " << trx->getExtent() << endl;
    // cerr << "thickness = " << trx->getThickness() << endl;
    // cerr << "zoomdist = " << view->getZoom() << endl;


}

void GLTransect::setScene(Scene * s)
{
    scene = s;
    view = new View();

    float tx, ty;

    // orthogonal rendering
    view->setViewType(ViewState::ORTHOGONAL);
    view->setZoomdist(0.0f);

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

    updateTransectView();

    // PCM: should these be here? this is called from constructor.
    winparent->rendercount++;
    signalRepaintAllGL();
}

void GLTransect::unlockView(Transect * imposedTrx)
{
    View * preview = view; // PCM: likely a leak of Viewe object at some point
    view = new View();
    (* view) = (* preview);

    trx = imposedTrx; // pointer managed externally so no need to delete previous
}

void GLTransect::lockView(View * imposedView, Transect * imposedTrx)
{
    delete view;
    view = imposedView;
    trx = imposedTrx; // pointer managed externally so no need to delete
    updateTransectView();
}

void GLTransect::loadDecals()
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
}

void GLTransect::initializeGL()
{
    // get context opengl-version
    qDebug() << "\nGLTransect initialize....\n";
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid() << "; Address: " << context();
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

    //PMrender::TRenderer::terrainShadingModel sMod = PMrender::TRenderer::BASIC;

    // PMrender::TRenderer::terrainShadingModel sMod = PMrender::TRenderer::FLAT_TRANSECT; // flat shading transect

PMrender::TRenderer::terrainShadingModel sMod = PMrender::TRenderer::RADIANCE_SCALING_TRANSECT;

    // set terrain shading model
    renderer->setTerrShadeModel(sMod);

    // set up light
    Vector dl = Vector(0.6f, 1.0f, 0.6f);
    dl.normalize();

  //  GLfloat pointLight[3] = {0.5, 5.0, 7.0}; // side panel + BASIC lighting
     GLfloat pointLight[3] = {1000.0, 2000.0, 1000.0}; // side panel + BASIC lighting
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
    glEnable(GL_TEXTURE_2D);

    loadDecals();
    paintGL();
}

void GLTransect::paintCyl(vpPoint p, GLfloat * col, std::vector<ShapeDrawData> &drawParams)
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

    shape.genCappedCylinder(scale*arad, 1.5f*scale*arad, scale*(mheight-mrad), 40, 10, tfm, false);
    if(shape.bindInstances(&translInstance, &scaleInstance, &cinst)) // passing in an empty instance will lead to one being created at the origin
    {
        sdd = shape.getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}

void GLTransect::paintGL()
{
    vpPoint mo;
    //glm::mat4 tfm, idt;
    //glm::vec3 trs, rot;
    std::vector<ShapeDrawData> drawParams; // to be passed to terrain renderer
    Shape shape, planeshape;  // geometry for focus indicator
    std::vector<glm::mat4> sinst;
    std::vector<glm::vec4> cinst;

    Timer t;

    if(active)
    {
        t.start();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateTransectView();

            /*
            if(focuschange)
            {
                GLfloat manipCol1[] = {0.325f, 0.235f, 1.0f, 1.0f};
                paintCyl(trx->getInnerStart(), manipCol1, drawParams);

                GLfloat manipCol2[] = {0.725f, 0.235f, 1.0f, 1.0f};
                paintCyl(trx->getInnerEnd(), manipCol2, drawParams);
            }*/

            // prepare plants for rendering

        if(focuschange || forceRebindPlants)
        {
            scene->getEcoSys()->bindPlantsSimplified(scene->getTerrain(), drawParams, &plantvis, rebindplants, transectPlanes);
            scene->getTerrain()->setBufferToDirty();
            rebindplants = false;
            forceRebindPlants = false;
        }

        // pass in draw params for objects
        renderer->setConstraintDrawParams(drawParams);

        // draw terrain and plants
        //scene->getTerrain()->setBufferToDirty();
        scene->getTerrain()->updateBuffers(renderer);

        renderer->draw(view);

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
    }
}

void GLTransect::resizeGL(int width, int height)
{
    // TO DO: fix resizing
    // int side = qMin(width, height);
    // glViewport((width - side) / 2, (height - side) / 2, width, height);
    glViewport(0, 0, width, height);

    view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
    view->apply();
}


void GLTransect::keyPressEvent(QKeyEvent *event)
{
}

void GLTransect::setAllPlantsVis()
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = true;
}

void GLTransect::setCanopyVis(bool vis)
{
    setAllPlantsVis();
    canopyvis = vis; // toggle canopy visibility
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
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

void GLTransect::setUndergrowthVis(bool vis)
{
    setAllPlantsVis();
    undervis = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
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

void GLTransect::setAllSpecies(bool vis)
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = vis;
    scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
    rebindplants = true;
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

void GLTransect::setSinglePlantVis(int p)
{
    if(p < (int) plantvis.size())
    {
        for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
            plantvis[i] = false;
        plantvis[p] = true;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
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
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}

void GLTransect::toggleSpecies(int p, bool vis)
{
    if(p < static_cast<int>(plantvis.size()))
    {
        plantvis[p] = vis;
        scene->getEcoSys()->pickAllPlants(scene->getTerrain(), canopyvis, undervis);
        rebindplants = true;
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
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}

void GLTransect::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    vpPoint pnt;

    int x = event->x(); int y = event->y();
    float W = static_cast<float>(width()); float H = static_cast<float>(height());

    // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
    if(event->modifiers() == Qt::MetaModifier || event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton)
    {
        // TO DO: translate in orthogonal view

        // convert to [0,1] X [0,1] domain
        /*
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        lastPos = event->pos();
        view->startArcRotate(nx, ny);*/
    }

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


    lastPos = event->pos();

}

void GLTransect::mouseMoveEvent(QMouseEvent *event)
{
    float nx, ny, W, H;

    int x = event->x();
    int y = event->y();

    W = (float) width();
    H = (float) height();

    // control translation of viewpoint in the plane of the transect
    if((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton)
    {
        // TO DO translate 
        // screen to world scaling
        float scw = view->getOrthoViewExtent() / W;

        float delx = (float) (x - lastPos.x());
        float dely = (float) (y - lastPos.y());
        delx *= scw;
        dely *= scw;
        vpPoint center = trx->getCenter();

        center.x -= delx * trx->getHorizontal().i;
        center.z -= delx * trx->getHorizontal().k;
        center.y += dely;
        trx->setCenter(center);

        // apply the same transformation to the transect inner bounds
        vpPoint inner[2];
        inner[0] = trx->getInnerStart(); inner[1] = trx->getInnerEnd();
        for(int i = 0; i < 2; i++)
        {
            inner[i].x -= delx * trx->getHorizontal().i;
            inner[i].z -= delx * trx->getHorizontal().k;
            // reproject onto terrain
            scene->getTerrain()->drapePnt(inner[i], inner[i]);
        }
        trx->setInnerStart(inner[0], scene->getTerrain()); trx->setInnerEnd(inner[1], scene->getTerrain());

        updateTransectView();
        winparent->rendercount++;
        signalRepaintAllGL();
        lastPos = event->pos();
    }

}

void GLTransect::wheelEvent(QWheelEvent * wheel)
{
    float del, extent;

    QPoint pix = wheel->pixelDelta();
    QPoint deg = wheel->angleDelta();

    if(!pix.isNull()) // screen resolution tracking, e.g., from magic mouse
    {
        del = (float) pix.y() * 2.0f;
    }
    else if(!deg.isNull()) // mouse wheel instead
    {
        del = (float) -deg.y() * 0.5f;
    }

        /*
        extent = view->getOrthoViewExtent();
        extent += del;
        view->setOrthoViewExtent(extent);
        */

    // also adjust inner bounds relative to center
    trx->zoom(del, scene->getTerrain());
    updateTransectView();
    winparent->rendercount++;
    signalRepaintAllGL();
}

void GLTransect::rebindPlants()
{
    rebindplants = true;
    forceRebindPlants = true;
}
