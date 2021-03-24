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
#include <QMessageBox>
#include <QInputDialog>

#include <fstream>
#include "data_importer/data_importer.h"
#include "data_importer/map_procs.h"

using namespace std;

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

////
// Scene
////

Scene::Scene()
{
    view = new View();
    terrain = new Terrain();
    terrain->initGrid(1024, 1024, 10000.0f, 10000.0f);
    view->setForcedFocus(terrain->getFocus());
    view->setViewScale(terrain->longEdgeDist());
    eco = new EcoSystem();
    biome = new Biome();

    int dx, dy;
    terrain->getGridDim(dx, dy);

    // instantiate typemaps for all possible typemaps		(XXX: this could lead to memory issues for larger landscapes?)
    for (int t = 0; t < int(TypeMapType::TMTEND); t++)
        maps[t] = new TypeMap(dx, dy, (TypeMapType)t);
    maps[2]->setRegion(terrain->coverRegion());		// this is for the 'TypeMapType::CATEGORY' typemap? Any reason why this one is special?

    for(int m = 0; m < 12; m++)
    {
        moisture.push_back(new MapFloat());
        sunlight.push_back(new MapFloat());
        temperature.push_back(0.0f);
    }
    slope = new MapFloat();
    chm = new MapFloat();
    cdm = new MapFloat();
    overlay = TypeMapType::EMPTY;
}

Scene::~Scene()
{
    delete view;
    delete terrain;

    // cycle through all typemaps, and if exists, delete and assign nullptr to indicate empty
    for (int t = 0; t < int(TypeMapType::TMTEND); t++)
    {
        if (maps[int(t)] != nullptr)
        {
            delete maps[int(t)];
            maps[int(t)] = nullptr;
        }
    }

    // delete sim;
    delete eco;
    delete biome;
    for(int m = 0; m < 12; m++)
    {
        delete sunlight[static_cast<int>(m)];
        delete moisture[static_cast<int>(m)];
    }
    temperature.clear();
    delete chm;
    delete cdm;
}


////
// GLWidget
////

static int curr_cohortmap = 0;

GLWidget::GLWidget(const QGLFormat& format, string datadir, QWidget *parent)
    : QGLWidget(format, parent)
{
    this->datadir = datadir;

    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    vizpopup = new QLabel();
    atimer = new QTimer(this);
    connect(atimer, SIGNAL(timeout()), this, SLOT(animUpdate()));

    rtimer = new QTimer(this);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(rotateUpdate()));
    glformat = format;

    // main design scene
    addScene();

    // database display and picking scene
    addScene();

    currscene = 0;

    renderer = new PMrender::TRenderer(nullptr, "../viz/shaders/");
    cmode = ControlMode::VIEW;
    viewing = false;
    viewlock = false;
    decalsbound = false;
    focuschange = false;
    focusviz = false;
    timeron = false;
    dbloaded = false;
    ecoloaded = false;
    // inclcanopy = true;
    active = true;
    scf = 10000.0f;
    decalTexture = 0;

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

    // delete views
    for(int i = 0; i < static_cast<int>(scenes.size()); i++)
        delete scenes[i];

    if (decalTexture != 0)	glDeleteTextures(1, &decalTexture);
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(1000, 800);
}


void GLWidget::screenCapture(QImage * capImg, QSize capSize)
{
    paintGL();
    glFlush();

    (* capImg) = grabFrameBuffer();
    (* capImg) = capImg->scaled(capSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

View * GLWidget::getView()
{
     if(!scenes.empty())
        return scenes[currscene]->view;
    else
        return nullptr;
}

Terrain * GLWidget::getTerrain()
{
    if(!scenes.empty())
        return scenes[currscene]->terrain;
    else
        return nullptr;
}

TypeMap * GLWidget::getTypeMap(TypeMapType purpose)
{
    if(!scenes.empty())
        return scenes[currscene]->maps[static_cast<int>(purpose)];
    else
        return nullptr;
}

MapFloat * GLWidget::getSunlight(int month)
{
     if(!scenes.empty())
        return scenes[currscene]->sunlight[month];
    else
        return nullptr;
}

MapFloat * GLWidget::getSlope()
{
    if(!scenes.empty())
        return scenes[currscene]->slope;
    else
        return nullptr;
}

MapFloat * GLWidget::getMoisture(int month)
{
   if(!scenes.empty())
        return scenes[currscene]->moisture[month];
    else
        return nullptr;
}

MapFloat * GLWidget::getCanopyHeightModel()
{
    if(!scenes.empty())
        return scenes[currscene]->chm;
    else
        return nullptr;
}

MapFloat * GLWidget::getCanopyDensityModel()
{
    if(!scenes.empty())
        return scenes[currscene]->cdm;
    else
        return nullptr;
}

PMrender::TRenderer * GLWidget::getRenderer()
{
    return renderer;
}

EcoSystem * GLWidget::getEcoSys()
{
    if(!scenes.empty())
        return scenes[currscene]->eco;
    else
        return nullptr;
}

Biome * GLWidget::getBiome()
{
    if(!scenes.empty())
        return scenes[currscene]->biome;
    else
        return nullptr;
}

bool GLWidget::readMonthlyMap(std::string filename, std::vector<MapFloat *> &monthly)
{
    float val;
    ifstream infile;
    int gx, gy, dx, dy;

    infile.open((char *) filename.c_str(), ios_base::in);
    if(infile.is_open())
    {
        infile >> gx >> gy;
#ifdef STEPFILE
        float step;
        infile >> step; // new format
#endif
        getTerrain()->getGridDim(dx, dy);
        if((gx != dx) || (gy != dy))
            cerr << "Error Simulation::readMonthlyMap: map dimensions do not match terrain" << endl;

        for(int m = 0; m < 12; m++)
            monthly[m]->setDim(gx, gy);

        for (int y = 0; y < gy; y++)
            for (int x = 0; x < gx; x++)
                for(int m = 0; m < 12; m++)
                {
                    infile >> val;
                    monthly[m]->set(x, y, val);
                }
        infile.close();
        return true;
    }
    else
    {
        cerr << "Error Simulation::readMonthlyMap: unable to open file" << filename << endl;
        return false;
    }
}

bool GLWidget::writeMonthlyMap(std::string filename, std::vector<MapFloat *> &monthly)
{
    int gx, gy;
    ofstream outfile;
    monthly[0]->getDim(gx, gy);

    outfile.open((char *) filename.c_str(), ios_base::out);
    if(outfile.is_open())
    {
        outfile << gx << " " << gy;
#ifdef STEPFILE
        outfile << " 0.9144"; // hardcoded step
#endif
        outfile << endl;
        for (int y = 0; y < gy; y++)
            for (int x = 0; x < gx; x++)
                for(int m = 0; m < 12; m++)
                    outfile << monthly[m]->get(x, y) << " ";

        outfile << endl;
        outfile.close();
        return true;
    }
    else
    {
        cerr << "Error Simulation::writeMonthlyMap:unable to open file " << filename << endl;
        return true;
    }

}

bool GLWidget::readSun(std::string filename)
{
    return readMonthlyMap(filename, scenes[currscene]->sunlight);
}

bool GLWidget::writeSun(std::string filename)
{
    return writeMonthlyMap(filename, scenes[currscene]->sunlight);
}

bool GLWidget::readMoisture(std::string filename)
{
    return readMonthlyMap(filename, scenes[currscene]->moisture);
}

bool GLWidget::writeMoisture(std::string filename)
{
    return writeMonthlyMap(filename, scenes[currscene]->moisture);
}

void GLWidget::calcSlope()
{
    int dx, dy;
    Vector up, n;

    // slope is dot product of terrain normal and up vector
    up = Vector(0.0f, 1.0f, 0.0f);
    getTerrain()->getGridDim(dx, dy);
    getSlope()->setDim(dx, dy);
    getSlope()->fill(0.0f);
    for(int x = 0; x < dx; x++)
        for(int y = 0; y < dy; y++)
        {
            getTerrain()->getNormal(x, y, n);
            float rad = acos(up.dot(n));
            float deg = RAD2DEG * rad;
            getSlope()->set(y, x, deg);
        }
}

void GLWidget::refreshOverlay()
{
    renderer->updateTypeMapTexture(getTypeMap(scenes[currscene]->overlay), PMrender::TRenderer::typeMapInfo::PAINT, false);
    update();
}

void GLWidget::setOverlay(TypeMapType purpose)
{
    scenes[currscene]->overlay = purpose;
    renderer->updateTypeMapTexture(getTypeMap(scenes[currscene]->overlay), PMrender::TRenderer::typeMapInfo::PAINT, true);
    update();
}

TypeMapType GLWidget::getOverlay()
{
    return scenes[currscene]->overlay;
}

void GLWidget::bandCanopyHeightTexture(float mint, float maxt)
{
    getTypeMap(TypeMapType::CHM)->bandCHMMap(getCanopyHeightModel(), mint*mtoft, maxt*mtoft);
    focuschange = true;
}

std::string GLWidget::get_dirprefix()
{
    std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();

    std::cout << "Datadir after fixing: " << datadir << std::endl;

    int slash_idx = datadir.find_last_of("/");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = datadir + "/" + setname;
    return dirprefix;
}

void GLWidget::loadFinScene(int timestep_start, int timestep_end)
{
    tstep_scrollwindow = new scrollwindow(this, timestep_start, timestep_end, 300, 100);
    tstep_scrollwindow->setVisible(false);

    std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();

    std::cout << "Datadir after fixing: " << datadir << std::endl;

    int slash_idx = datadir.find_last_of("/");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = get_dirprefix();

    loadFinScene(dirprefix, timestep_start, timestep_end);
}

void GLWidget::loadFinScene(std::string dirprefix, int timestep_start, int timestep_end)
{
    using namespace data_importer;

    std::vector<std::string> timestep_files;
    std::string terfile = datadir+"/dem.elv";

    initstep = timestep_start;
    for (int ts = timestep_start; ts <= timestep_end; ts++)
    {
        timestep_files.push_back(datadir + "/ecoviz_" + std::to_string(ts) + ".pdb");
    }


    /*
    std::string cpdbfile = dirprefix + "_canopy";
    std::string updbfile = dirprefix + "_undergrowth";
    cpdbfile += std::to_string(curr_canopy) + ".pdb";
    updbfile += std::to_string(curr_canopy) + ".pdb";
    */

    // load terrain
    currscene = 0;
    getTerrain()->loadElv(terfile);
    cerr << "Elevation file loaded" << endl;
    scf = getTerrain()->getMaxExtent();
    getView()->setForcedFocus(getTerrain()->getFocus());
    getView()->setViewScale(getTerrain()->longEdgeDist());
    getView()->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));
    getTerrain()->calcMeanHeight();

    // match dimensions for empty overlay
    int dx, dy;
    getTerrain()->getGridDim(dx, dy);
    getTypeMap(TypeMapType::PAINT)->matchDim(dx, dy);
    getTypeMap(TypeMapType::PAINT)->fill(0);
    getTypeMap(TypeMapType::EMPTY)->matchDim(dx, dy);
    getTypeMap(TypeMapType::EMPTY)->clear();

    float rw, rh;
    getTerrain()->getTerrainDim(rw, rh);

    // import cohorts

    for (auto &tsfname : timestep_files)
    {
        auto fdata = ilanddata::read(tsfname, "2.0");
        ilanddata::trim_filedata_spatial(fdata, int(rw), int(rh));

        cohortmaps.push_back(ValueMap<std::vector<ilanddata::cohort> >());
        cohortmaps.back().setDim(int(ceil(rw / 2)), int(ceil(rh / 2)));

        for (auto &crt : fdata.cohorts)
        {
            int gx = (crt.xs - 1) / 2;
            int gy = (crt.ys - 1) / 2;
            cohortmaps.back().get(gx, gy).push_back(crt);
        }
    }

    int gw, gh;
    if (cohortmaps.size() > 0)
        cohortmaps[0].getDim(gw, gh);
    for (auto &tscmap : cohortmaps)
    {

        int tgw = ceil(rw), tgh = ceil(rh);

        cohort_plantcountmaps.emplace_back(tgw, tgh);

        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                auto &crts = tscmap.get(x, y);
                float count = 0.0f;
                for (auto &crt : crts)
                {
                    count += float(crt.nplants);		// nplants should be float...?
                }
                for (int i = 0; i < 2; i++)
                    for (int j = 0; j < 2; j++)
                    {
                        int xloc = x * 2 + 1 + i;
                        int yloc = y * 2 + 1 + j;
                        if (xloc < tgw && yloc < tgh)
                            cohort_plantcountmaps.back().set(yloc, xloc, count);
                    }
            }
        }
    }

    if (cohortmaps.size() > 0)
    {
        sampler = std::unique_ptr<cohortsampler>(new cohortsampler(rw, rh, gw, gh, 10, 3));

        //std::vector<basic_tree> trees = sampler->sample(cohortmaps[0]);
        //data_importer::write_pdb("testsample.pdb", trees.data(), trees.data() + trees.size());
    }

    if (getBiome()->read_dataimporter(SONOMA_DB_FILEPATH))
    {
        if (static_cast<int>(plantvis.size()) < getBiome()->numPFTypes())
            plantvis.resize(getBiome()->numPFTypes());
        cerr << "Biome file load" << endl;
        for(int t = 0; t < getBiome()->numPFTypes(); t++)
            plantvis[t] = true;

        canopyvis = true;
        undervis = true;

        // loading plant distribution
        getEcoSys()->setBiome(getBiome());

        /*
        if(!getEcoSys()->loadNichePDB(cpdbfile, getTerrain()))
             std::cerr << "Plant distribution file " << cpdbfile << "does not exist" << endl; // just report but not really an issue
        else
            std::cerr << "Plant canopy distribution file loaded" << std::endl;


        if(!getEcoSys()->loadNichePDB(updbfile, getTerrain(), 1))
             std::cerr << "Undergrowth distribution file " << updbfile << " does not exist" << endl; // just report but not really an issue
        else
            std::cerr << "Plant undergrowth distribution file loaded" << std::endl;
        */

        setAllPlantsVis();
        focuschange = !focuschange;
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
        getEcoSys()->redrawPlants();
        update();

        loadTypeMap(getSlope(), TypeMapType::SLOPE);
    }
    else
    {
        focuschange = false;
    }
}

void GLWidget::saveScene(std::string dirprefix)
{
    std::string terfile = dirprefix+".elv";
    std::string pdbfile = dirprefix+".pdb";

    // load terrain
    getTerrain()->saveElv(terfile);

    if(!getEcoSys()->saveNichePDB(pdbfile))
        cerr << "Error GLWidget::saveScene: saving plane file " << pdbfile << " failed" << endl;
}

void GLWidget::writePaintMap(std::string paintfile)
{
    getTypeMap(TypeMapType::PAINT)->saveToPaintImage(paintfile);
}

void GLWidget::addScene()
{
    Scene * scene = new Scene();
    scene->view->setDim(0.0f, 0.0f, static_cast<float>(this->width()), static_cast<float>(this->height()));

    plantvis.clear();
    scenes.push_back(scene);
    currscene = static_cast<int>(scenes.size()) - 1;
}

void GLWidget::setScene(int s)
{
    if(s >= 0 && s < static_cast<int>(scenes.size()))
    {
        currscene = s;
        getTerrain()->setBufferToDirty();
        refreshOverlay();
        update();
    }
}


void GLWidget::loadDecals()
{
    QImage decalImg, t;

    // load image
    if(!decalImg.load(QCoreApplication::applicationDirPath() + "/../../common/Icons/manipDecals.png"))
        cerr << QCoreApplication::applicationDirPath().toUtf8().constData() << "/../../common/Icons/manipDecals.png" << " not found" << endl;

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

int GLWidget::loadTypeMap(MapFloat * map, TypeMapType purpose)
{
    int numClusters = 0;

    switch(purpose)
    {
        case TypeMapType::EMPTY:
            break;
        case TypeMapType::PAINT:
            break;
        case TypeMapType::CATEGORY:
            break;
        case TypeMapType::SLOPE:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 90.0f);
            break;
        case TypeMapType::WATER:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 100.0); // 1000.0f);
            break;
        case TypeMapType::SUNLIGHT:
             numClusters = getTypeMap(purpose)->convert(map, purpose, 13.0f);
             break;
        case TypeMapType::TEMPERATURE:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 20.0f);
            break;
        case TypeMapType::CHM:
            numClusters = getTypeMap(purpose)->convert(map, purpose, mtoft*initmaxt);
            break;
        case TypeMapType::CDM:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 1.0f);
            break;
        default:
            break;
    }
    return numClusters;
}

void GLWidget::setMap(TypeMapType type, int mth)
{
    if(type == TypeMapType::SUNLIGHT)
        loadTypeMap(getSunlight(mth), type);
    if(type == TypeMapType::WATER)
        loadTypeMap(getMoisture(mth), type);
    setOverlay(type);
}

void GLWidget::initializeGL()
{
    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
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

    GLfloat pointLight[3] = { 0.5, 5.0, 7.0}; // side panel + BASIC lighting
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

void GLWidget::paintGL()
{
    vpPoint mo;
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    uts::vector<ShapeDrawData> drawParams; // to be passed to terrain renderer
    Shape shape;  // geometry for focus indicator
    std::vector<glm::mat4> sinst;
    std::vector<glm::vec4> cinst;

    Timer t;

    if(active)
    {
        t.start();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // note: bindinstances will not work on the first render pass because setup time is needed

        if(focuschange && focusviz)
        {
            ShapeDrawData sdd;
            float scale;

            GLfloat manipCol[] = {0.325f, 0.235f, 1.0f, 1.0f};

            // create shape
            shape.clear();
            shape.setColour(manipCol);

            // place vertical cylinder at view focus
            mo = getView()->getFocus();
            scale = getView()->getScaleFactor();
            idt = glm::mat4(1.0f);
            trs = glm::vec3(mo.x, mo.y, mo.z);
            rot = glm::vec3(1.0f, 0.0f, 0.0f);
            tfm = glm::translate(idt, trs);
            tfm = glm::rotate(tfm, glm::radians(-90.0f), rot);
            shape.genCappedCylinder(scale*armradius, 1.5f*scale*armradius, scale*(manipheight-manipradius), 40, 10, tfm, false);
            if(shape.bindInstances(getView(), &sinst, &cinst)) // passing in an empty instance will lead to one being created at the origin
            {
                sdd = shape.getDrawParameters();
                sdd.current = false;
                drawParams.push_back(sdd);
            }
        }

        // prepare plants for rendering
        if(focuschange)
            getEcoSys()->bindPlantsSimplified(getTerrain(), drawParams, &plantvis);

        // pass in draw params for objects
        renderer->setConstraintDrawParams(drawParams);

        // draw terrain and plants
        getTerrain()->updateBuffers(renderer); 

        if(focuschange)
            renderer->updateTypeMapTexture(getTypeMap(getOverlay())); // only necessary if the texture is changing dynamically
        renderer->draw(getView());

        t.stop();

        if(timeron)
            cerr << "rendering = " << t.peek() << " fps = " << 1.0f / t.peek() << endl;
    }
}

void GLWidget::resizeGL(int width, int height)
{
    // TO DO: fix resizing
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, width, height);

    // apply to all views
    for(int i = 0; i < static_cast<int>(scenes.size()); i++)
    {
        scenes[i]->view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
        scenes[i]->view->apply();
    }
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_A) // 'A' for animated spin around center point of terrain
    {
        getView()->startSpin();
        rtimer->start(20);
    }
    if(event->key() == Qt::Key_C) // 'C' to show canopy height model texture overlay
    {
        setOverlay(TypeMapType::CHM);
    }
    if(event->key() == Qt::Key_E) // 'E' to remove all texture overlays
    {
        setOverlay(TypeMapType::EMPTY);
    }
    if(event->key() == Qt::Key_F) // 'F' to toggle focus stick visibility
    {
        if(focusviz)
            focusviz = false;
        else
            focusviz = true;
        update();
    }
    if(event->key() == Qt::Key_N) // 'N' to toggle display of canopy trees on or off
    {
        cerr << "canopy visibility toggled" << endl;
        setAllPlantsVis();
        canopyvis = !canopyvis; // toggle canopy visibility
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
        update();
    }
    if(event->key() == Qt::Key_O) // 'O' to toggle timestep slider
    {
        if (getTypeMap(TypeMapType::COHORT)->getNumSamples() == -1)
        {
            QMessageBox(QMessageBox::Warning, "Typemap Error", "Type map for cohorts does not have a valid colour table").exec();
        }
        else if (cohort_plantcountmaps.size() > 0)
        {
            tstep_scrollwindow->setVisible(true);
            set_timestep(initstep);
            //curr_cohortmap++;
            //getTypeMap(TypeMapType::COHORT)->save("/home/konrad/cohorttypemap.txt");
        }
        else
            QMessageBox(QMessageBox::Warning, "Typemap Error", "No cohort plant count maps available").exec();

        /*
        cerr << "unit test on ascii grid load" << endl;
        ValueGridMap<float> map;
        map = data_importer::load_esri<ValueGridMap<float>>("./test.txt");
        cerr << map.get(0, 0) << " " << map.get(1, 2) << endl;
        // possible order issue on reads
        */
    }
    if(event->key() == Qt::Key_P) // 'P' to toggle plant visibility
    {
        cerr << "plant visibility toggled" << endl;
        setAllPlantsVis();
        focuschange = !focuschange;
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
        update();
    }
    if(event->key() == Qt::Key_R) // 'R' to show temperature texture overlay
    {
        setOverlay(TypeMapType::TEMPERATURE);
    }
    if(event->key() == Qt::Key_S) // 'S' to show sunlight texture overlay
    {
        sun_mth++;
        if(sun_mth >= 12)
            sun_mth = 0;
        loadTypeMap(getSunlight(sun_mth), TypeMapType::SUNLIGHT);
        setOverlay(TypeMapType::SUNLIGHT);
    }
    if(event->key() == Qt::Key_T) // 'T' to show slope texture overlay
    {
        loadTypeMap(getSlope(), TypeMapType::SLOPE);
        setOverlay(TypeMapType::SLOPE);
    }
    if(event->key() == Qt::Key_U) // 'U' toggle undergrowth display on/off
    {
        setAllPlantsVis();
        undervis = !undervis; // toggle canopy visibility
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
        update();
    }
    if(event->key() == Qt::Key_V) // 'V' for top-down view
    {
        getTerrain()->setMidFocus();
        getView()->setForcedFocus(getTerrain()->getFocus());
        getView()->topdown();
        update();
    }
    if(event->key() == Qt::Key_W) // 'W' to show water texture overlay
    {
        wet_mth++;

        if(wet_mth >= 12)
            wet_mth = 0;
        loadTypeMap(getMoisture(wet_mth), TypeMapType::WATER);
        setOverlay(TypeMapType::WATER);
    }
    // '1'-'9' make it so that only plants of that functional type are visible
    if(event->key() == Qt::Key_0)
    {
        cerr << "KEY 0" << endl;
        int p = 0;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9)
    {
        int p = static_cast<int>(event->key()) - static_cast<int>(Qt::Key_1) + 1;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_ParenRight)
    {
         cerr << "KEY )" << endl;
        int p = 10;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_Exclam)
    {
         cerr << "KEY !" << endl;
        int p = 11;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_At)
    {
         cerr << "KEY @" << endl;
        int p = 12;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_NumberSign)
    {
         cerr << "KEY #" << endl;
        int p = 13;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_Dollar)
    {
         cerr << "KEY $" << endl;
        int p = 14;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_Percent)
    {
         cerr << "KEY %" << endl;
        int p = 15;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
    }
    if(event->key() == Qt::Key_Ampersand)
    {
         cerr << "KEY &" << endl;
        int p = 16;
        setSinglePlantVis(p);
        cerr << "single species visibility " << p << endl;
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
    getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
    update();
}

void GLWidget::setUndergrowthVis(bool vis)
{
    setAllPlantsVis();
    undervis = vis;
    getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
    update();
}

void GLWidget::setAllSpecies(bool vis)
{
    for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
        plantvis[i] = vis;
    getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
    update();
}

void GLWidget::setSinglePlantVis(int p)
{
    if(p < (int) plantvis.size())
    {
        for(int i = 0; i < static_cast<int>(plantvis.size()); i++)
            plantvis[i] = false;
        plantvis[p] = true;
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
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
        getEcoSys()->pickAllPlants(getTerrain(), canopyvis, undervis);
        update();
    }
    else
    {
        cerr << "non-valid pft and so unable to toggle visibility" << endl;
    }
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    vpPoint pnt;
    
    int x = event->x(); int y = event->y();
    float W = static_cast<float>(width()); float H = static_cast<float>(height());

    update(); // ensure this viewport is current for unproject

    // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
    if(!viewlock && (event->modifiers() == Qt::MetaModifier || event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton))
    {
        // arc rotate in perspective mode
  
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        lastPos = event->pos();
        getView()->startArcRotate(nx, ny);
        viewing = true;
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
    if(!viewlock && ((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton))
    {
        getView()->apply();
        if(getTerrain()->pick(sx, sy, getView(), pnt))
        {
            if(!decalsbound)
                loadDecals();
            vpPoint pickpnt = pnt;
            getView()->setAnimFocus(pickpnt);
            getTerrain()->setFocus(pickpnt);
            cerr << "Pick Point = " << pickpnt.x << ", " << pickpnt.y << ", " << pickpnt.z << endl;
            focuschange = true; focusviz = true;
            atimer->start(10);
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
   // getSim()->pickInfo(x, y);
   cerr << "Canopy Height (m): " << getCanopyHeightModel()->get(x, y) * 0.3048f  << endl;
   cerr << "Canopy Density: " << getCanopyDensityModel()->get(x, y) << endl;
   cerr << "Sunlight: " << getSunlight(sun_mth)->get(x,y) << endl;
   cerr << "Moisture: " << getMoisture(wet_mth)->get(x,y) << endl;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    viewing = false;

    if(event->button() == Qt::LeftButton && cmode == ControlMode::VIEW) // info on terrain cell
    {
        vpPoint pnt;
        int sx, sy;

        sx = event->x(); sy = event->y();

        if(getTerrain()->pick(sx, sy, getView(), pnt))
        {
            int x, y;
            getTerrain()->toGrid(pnt, x, y);
            pickInfo(x, y);
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

    // control view orientation with right mouse button or ctrl modifier key and left mouse
    if(!viewlock && ((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton))
    {
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        getView()->arcRotate(nx, ny);
        update();
        lastPos = event->pos();
    }
}

void GLWidget::wheelEvent(QWheelEvent * wheel)
{
    float del;
 
    QPoint pix = wheel->pixelDelta();
    QPoint deg = wheel->angleDelta();

    if(!viewlock)
    {
        if(!pix.isNull()) // screen resolution tracking, e.g., from magic mouse
        {
            del = (float) pix.y() * 10.0f;
            getView()->incrZoom(del);
            update();

        }
        else if(!deg.isNull()) // mouse wheel instead
        {
            del = (float) -deg.y() * 2.5f;
            getView()->incrZoom(del);
            update();
        }
    }
}

void GLWidget::animUpdate()
{
    if(getView()->animate())
        update();
}

void GLWidget::rotateUpdate()
{
    if(getView()->spin())
        update();
}

void GLWidget::set_timestep(int tstep)
{
    curr_cohortmap = tstep - initstep;
    if (curr_cohortmap >= cohort_plantcountmaps.size())
        curr_cohortmap = cohort_plantcountmaps.size() - 1;
    loadTypeMap(cohort_plantcountmaps.at(curr_cohortmap), TypeMapType::COHORT);
    setOverlay(TypeMapType::COHORT);

    tstep_scrollwindow->set_labelvalue(tstep);

    auto trees = sampler->sample(cohortmaps.at(curr_cohortmap));
    for (auto &t : trees)
        t.species = t.species % 6;
    getEcoSys()->clear();
    getEcoSys()->placeManyPlants(getTerrain(), trees);
    getEcoSys()->redrawPlants();

    std::cout << "Timestep changed to " << tstep << std::endl;
}
