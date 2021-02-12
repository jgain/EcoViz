// Authors: K.P. Kapp and J.E. Gain

/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems
 * Copyright (C) 2020  K.P. Kapp  (konrad.p.kapp@gmail.com) and J.E. Gain (jgain@cs.uct.ac.za)
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
#include "window.h"
#include "data_importer/extract_png.h"
#include "histcomp_window.h"
#include <data_importer/AbioticMapper.h>
#include <common/custom_exceptions.h>

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <fstream>

#include <QGridLayout>
#include <QGLFramebufferObject>
#include <QImage>
#include <QCoreApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

//using layerspec = ClusterMatrices::layerspec;

static std::string get_errstring(GLuint errcode)
{
    switch (errcode)
    {
        case (GL_NO_ERROR):
            return "no error";
            break;
        case (GL_INVALID_ENUM):
            return "INVALID_ENUM";
            break;
        case (GL_INVALID_VALUE):
            return "INVALID_VALUE";
            break;
        case (GL_INVALID_OPERATION):
            return "INVALID_OPERATION";
            break;
        case (GL_INVALID_FRAMEBUFFER_OPERATION):
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case (GL_OUT_OF_MEMORY):
            return "GL_OUT_OF_MEMORY";
            break;
        case (GL_STACK_OVERFLOW):
            return "GL_STACK_OVERFLOW";
            break;
        case (GL_STACK_UNDERFLOW):
            return "GL_STACK_UNDERFLOW";
            break;
        default:
            return "Unknown error";
            break;
    }
}

#define GL_ERRCHECK(show_noerr) \
    { \
        GLenum errcode = glGetError();	\
        if (errcode != GL_NO_ERROR) \
        { \
            std::cout << "GL error in file " << __FILE__ << ", line " << __LINE__ << ": " << get_errstring(errcode) << std::endl; \
        } \
        else if (show_noerr) \
        { \
            std::cout << "No GL errors on record in file " << __FILE__ << ", line " << __LINE__ << std::endl; \
        } \
    }


using namespace std;

static int nspacingsteps = 1;	// to illustrate the difference between different numbers of iterations for the canopy placement.
                                // Add a GUI control for this later?

std::vector<int> get_canopyspecs(std::string dbname)
{
    data_importer::common_data cdata(dbname);
    std::vector<int> specs;
    for (auto &p : cdata.all_species)
    {
        specs.push_back(p.first);
    }
    return specs;
}


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

    sim = new Simulation();
    eco = new EcoSystem();
    biome = new Biome();

    int dx, dy;
    terrain->getGridDim(dx, dy);

    for(TypeMapType t: all_typemaps)
        maps[(int) t] = new TypeMap(dx, dy, t);
    maps[2]->setRegion(terrain->coverRegion());
    grass = new GrassSim();
    moisture = new MapFloat();
    illumination = new MapFloat();
    temperature = new MapFloat();
    chm = new MapFloat();
    cdm = new MapFloat();
    chm_cpl = new basic_types::MapFloat();
}

Scene::~Scene()
{
    delete view;
    delete terrain;
    for(TypeMapType t: all_typemaps)
       if(maps[(int) t] != nullptr)
       {
           delete maps[(int) t];
           maps[(int) t] = nullptr;
       }
    delete sim;
    delete eco;
    delete biome;
    delete grass;
    delete illumination;
    delete moisture;
    delete temperature;
    delete chm;
    delete cdm;
}



////
// GLWidget
////

GLWidget::GLWidget(const QGLFormat& format, int scale_size, QWidget *parent)
    : QGLWidget(format, parent), prj_src_dir(PRJ_SRC_DIR), db_pathname(std::string(PRJ_SRC_DIR) + "/ecodata/sonoma.db"), plant_sqldb_name(db_pathname), cdata(plant_sqldb_name),
      scale_size(scale_size), sceneloaded(false), undergrowth_sampled(false)
{
    assign_times = 0;
    qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    vizpopup = new QLabel();
    atimer = new QTimer(this);
    connect(atimer, SIGNAL(timeout()), this, SLOT(animUpdate()));

    rtimer = new QTimer(this);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(rotateUpdate()));

    connect(this, SIGNAL(signalRepaintAllFromThread()), this, SLOT(repaint()));

    // main design scene
    addScene();

    // database display and picking scene
    addScene();

    currscene = 0;

    renderer = new PMrender::TRenderer(NULL, "../viewer/shaders/");

    all_possible_species = cdata.all_species;
    species_infomap = cdata.all_species;		// TODO: species_infomap starts as all species selected - need to do so accordingly for checkboxes

    int i = 0;
    for (auto &p : all_possible_species)
    {
        allcanopy_idx_to_id.push_back(p.first);
        allcanopy_id_to_idx[p.first] = i;
        i++;
    }

    // the values of specbrushes isn't being used currently - just the vector's size is being used
    std::vector<int> specbrushes;
    for (auto &p : all_possible_species)
    {
        specbrushes.push_back(p.second.idx);
    }

    setRadius(250.0f);
    cmode = ControlMode::VIEW;
    viewing = false;
    viewlock = false;
    decalsbound = false;
    focuschange = false;
    focusviz = false;
    timeron = false;
    dbloaded = false;
    ecoloaded = false;
    scf = 10000.0f;
    decalTexture = 0;
    //spacer = nullptr;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    resize(sizeHint());
    setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);

    glsun = new GLSun(format);
    ipc = new IPC();

    ipc_received_raw = new MapFloat();

    nspecies = 5;
    species_percentages = std::vector<float> (nspecies, 1.0f / nspecies);		// find a better way to initialize this!!

    //genOpenglTexturesForTrees();

    std::cerr << "GLWidget construction done" << std::endl;
}

void GLWidget::repaint()
{
    report_cudamem("GPU memory in use before repaint: ");
    QGLWidget::repaint();
    report_cudamem("GPU memory in use after repaint: ");
}

GLWidget::~GLWidget()
{
    delete atimer;
    delete rtimer;
    if(vizpopup) delete vizpopup;

    if (renderer) delete renderer;

    // delete views
    for(int i = 0; i < (int) scenes.size(); i++)
        delete scenes[i];

    if (decalTexture != 0)	glDeleteTextures(1, &decalTexture);
    delete ipc;
    delete ipc_received_raw;

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
    if((int) scenes.size() > 0)
        return scenes[currscene]->view;
    else
        return NULL;
}

Terrain * GLWidget::getTerrain()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->terrain;
    else
        return NULL;
}

TypeMap * GLWidget::getTypeMap(TypeMapType purpose)
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->maps[(int) purpose];
    else
        return NULL;
}

MapFloat * GLWidget::getSunlight(int month)
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->getSunlightMap(month);
    else
        return NULL;
}

MapFloat * GLWidget::getSunlight()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->get_average_sunlight_map();
    else
        return NULL;
}

MapFloat * GLWidget::getSlope()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->getSlopeMap();
    else
        return NULL;
}

MapFloat * GLWidget::getMoisture(int month)
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->getMoistureMap(month);
    else
        return NULL;
}

MapFloat * GLWidget::getMoisture()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->get_average_moisture_map();
    else
        return NULL;
}

MapFloat *GLWidget::getTemperature()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->get_temperature_map();
    else
        return NULL;
}

basic_types::MapFloat * GLWidget::getPlacerCanopyHeightModel()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->chm_cpl;
    else
        return NULL;
}

MapFloat * GLWidget::getCanopyHeightModel()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->chm;
    else
        return NULL;
}

MapFloat * GLWidget::getCanopyDensityModel()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->cdm;
    else
        return NULL;
}

PMrender::TRenderer * GLWidget::getRenderer()
{
    return renderer;
}

void GLWidget::genOpenglTexturesForTrees()
{
    //getEcoSys()->genOpenglTextures();
}

EcoSystem * GLWidget::getEcoSys()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->eco;
    else
        return NULL;
}

Simulation * GLWidget::getSim()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim;
    else
        return NULL;
}

void GLWidget::initSceneSim()
{
    // assumes terrain and biome already loaded
    if((int) scenes.size() > 0)
    {
        scenes[currscene]->sim = new Simulation(getTerrain(), getBiome(), 5);
    }
}

Biome * GLWidget::getBiome()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->biome;
    else
        return NULL;
}

GLSun * GLWidget::getGLSun()
{
    return glsun;
}

GrassSim * GLWidget::getGrass()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->grass;
    else
        return NULL;
}

void GLWidget::init_underspacer()
{
    int gw, gh;
    float rw, rh;
    canopyshading_temp.getDim(gw, gh);
    canopyshading_temp.getDimReal(rw, rh);
    std::cout << "Sunlight map gw, gh: " << gw << ", " << gh << std::endl;
    std::cout << "Sunlight map rw, rh: " << rw << ", " << rh << std::endl;
    abiotic_maps_package amaps_temp(amaps_ptr->wet, canopyshading_temp, amaps_ptr->temp, amaps_ptr->slope);
}

void GLWidget::setCtrlMode(ControlMode mode)
{
    if(mode == ControlMode::PAINTECO)
    {
        if(dbloaded && ecoloaded)
            cmode = mode;
    }
    else
    {
        cmode = mode;
    }
}

void GLWidget::setMode(ControlMode mode)
{

    switch(mode)
    {
        case ControlMode::VIEW:
            cmode = mode;
            setOverlay(TypeMapType::CATEGORY);
            break;
        case ControlMode::PAINTLEARN:
            cmode = mode;
            setOverlay(TypeMapType::PAINT);
            break;
        case ControlMode::PAINTECO:
            if(dbloaded && ecoloaded) // can only change to this mode if database and ecosystem are loaded
            {
                cmode = mode;
                setOverlay(TypeMapType::PAINT);
            }
            break;
        case ControlMode::PAINTSPECIES:
            cmode = mode;
            setOverlay(TypeMapType::SPECIES);
            break;
        default:
            break;
    }
    update();
}

void GLWidget::convert_painting(BrushType from, BrushType to)
{
    TypeMap *tmap = getTypeMap(TypeMapType::PAINT);

    int fromi, toi;

    switch (from)
    {
        case BrushType::FREE:
            fromi = 0;
            break;
        case BrushType::SPARSETALL:
        case BrushType::SPARSEMED:
        case BrushType::SPARSESHRUB:
            fromi = 1;
            break;
        case BrushType::DENSETALL:
        case BrushType::DENSEMED:
        case BrushType::DENSESHRUB:
            fromi = 2;
            break;
        default:
            return;

    }

    switch (to)
    {
        case BrushType::FREE:
            toi = 0;
            break;
        case BrushType::SPARSETALL:
        case BrushType::SPARSEMED:
        case BrushType::SPARSESHRUB:
            toi = 1;
            break;
        case BrushType::DENSETALL:
        case BrushType::DENSEMED:
        case BrushType::DENSESHRUB:
            toi = 2;
            break;
        default:
            return;
    }

    tmap->replace_value(fromi, toi);

    //send_drawing();
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

void GLWidget::set_underspace_viabbase(float viabbase)
{
    std::cout << "Setting viability base to " << viabbase << std::endl;
}

void GLWidget::set_underspace_radmult(float radmult)
{
    std::cout << "Setting radius multiplier to " << radmult << std::endl;
}

void GLWidget::setSpecPerc(float perc)
{
    specperc = perc;
}

float GLWidget::getSpecPerc()
{
    return specperc;
}

void GLWidget::setRadius(float rad)
{
    float brushradius = rad;
    if (cmode == ControlMode::PAINTLEARN)
    {
        learnbrush_rad = brushradius;
    }
    else if (cmode == ControlMode::PAINTSPECIES)
    {
        specbrush_rad = brushradius;
    }
    brushcursor.setRadius(brushradius);
    update();
}

const data_importer::common_data &GLWidget::get_cdata()
{
    return cdata;
}

float GLWidget::getRadius()
{
    return brushcursor.getRadius();
}

float GLWidget::getLearnBrushRadius()
{
    return learnbrush_rad;
}

void GLWidget::bandCanopyHeightTexture(float mint, float maxt)
{
    getTypeMap(TypeMapType::CHM)->bandCHMMapEric(getCanopyHeightModel(), mint*mtoft, maxt*mtoft);
    focuschange = true;
}

void GLWidget::grow_grass()
{
    getGrass()->matchDim(getTerrain(), 10000.0f, 1);
    getGrass()->setConditions(getSim()->get_average_moisture_map(), getSim()->get_average_landsun_map(), getSim()->get_average_landsun_map(), getSim()->get_temperature_map());
    getGrass()->grow(getTerrain(), canopytrees, cdata, scf);
}

MapFloat *GLWidget::get_rocks()
{
    if((int) scenes.size() > 0)
        return scenes[currscene]->sim->get_rocks();
    else
        return NULL;
}

void GLWidget::set_pretty_map()
{
    MapFloat *grass = getGrass()->get_data();
    MapFloat *rocks = get_rocks();
    MapFloat *wet = getMoisture();

    int tx, ty;
    getTerrain()->getGridDim(tx, ty);
    int dx, dy;
    grass->getDim(dx, dy);
    if (dx != tx || dy != ty)
    {
        throw std::runtime_error("Grass map dimensions not equal to terrain dimensions. Terrain gw, gh: " + std::to_string(tx) + ", " + std::to_string(ty) + ". Grass map: " + std::to_string(dx) + ", " + std::to_string(dy));
    }
    rocks->getDim(dx, dy);
    if (dx != tx || dy != ty)
    {
        // rock dimensions not equal to terrain dimension, possibly due to rock map not being initialized yet.
        // so we initialize
        getSim()->set_rocks();
        rocks->getDim(dx, dy);

        // if it's still not equal in dimension, throw error
        if (dx != tx || dy != ty)
            throw std::runtime_error("Rock map dimensions not equal to terrain dimensions. Terrain gw, gh: " + std::to_string(tx) + ", " + std::to_string(ty) + ". Rock map: " + std::to_string(dx) + ", " + std::to_string(dy));
    }
    wet->getDim(dx, dy);
    if (dx != tx || dy != ty)
    {
        throw std::runtime_error("Moisture map dimensions not equal to terrain dimensions. Terrain gw, gh: " + std::to_string(tx) + ", " + std::to_string(ty) + ". Moisture map: " + std::to_string(dx) + ", " + std::to_string(dy));
    }

    pretty_map_painted.setDim(dx, dy);

    for (int y = 0; y < dy; y++)
    {
        for (int x = 0; x < dx; x++)
        {
            if (rocks->get(x, y) > 0.0f)
                pretty_map_painted.set(x, y, 1.0f);
            else if (grass->get(x, y) > 1.0f)
                pretty_map_painted.set(x, y, 2.0f + grass->get(x, y));
            else if (wet->get(x, y) > 100.0f)
                pretty_map_painted.set(x, y, 2.0f + MAXGRASSHGHT + 1.0f + wet->get(x, y));
            else
                pretty_map_painted.set(x, y, 0.0f);
        }
    }

    pretty_map = pretty_map_painted;

    const int add_const = 1000000;		// TODO: Define this constant somewhere, where it can be used also by the trenderer file

    for (int y = 0; y < dy; y++)
    {
        for (int x = 0; x < dx; x++)
        {
            float prevval = pretty_map_painted.get(x, y);
            auto tmap_ptr = getTypeMap(TypeMapType::PAINT);
            int painttype = tmap_ptr->get(x, y);		// this is a bit of a hard-coded value. Try to use proper enums?
            if (painttype == 1)
            {
                pretty_map_painted.set(x, y, prevval + add_const);
            }
            else if (painttype == 2)
            {
                pretty_map_painted.set(x, y, prevval + add_const * 2);
            }
        }
    }
}

void GLWidget::loadScene(std::string dirprefix)
{
    sceneloaded = false;
    bool simvalid = true;
    bool terloaded = false, wetloaded = false, sunloaded = false;
    QFileInfo finfo(QString(dirprefix.c_str()));
    base_dirname = finfo.absolutePath().toStdString();
    while (base_dirname.back() == '/')
        base_dirname.pop_back();

    int curr_counter = 0;
    pipeout_dirname = base_dirname + "/pipe_out" + std::to_string(curr_counter);
    QDir pipedir;
    while (!pipedir.mkdir(pipeout_dirname.c_str()))
    {
        curr_counter++;
        pipeout_dirname = base_dirname + "/pipe_out" + std::to_string(curr_counter);
    }

    logfile_ofs = std::ofstream(pipeout_dirname + "/" + "log");

    std::string terfile = dirprefix+".elv";
    std::string pdbfile = dirprefix+".pdb";
    std::string chmfile = dirprefix+".chm";
    std::string cdmfile = dirprefix+".cdm";
    std::string sunfile = dirprefix+"_sun.txt";
    std::string landsun_file = dirprefix+"_sun_landscape.txt";
    std::string wetfile = dirprefix+"_wet.txt";
    std::string climfile = dirprefix+"_clim.txt";
    std::string bmefile = dirprefix+"_biome.txt";
    std::string catfile = dirprefix+"_plt.png";
    std::string slopefile = dirprefix+"_slope.txt";
    std::string grassparamsfile = dirprefix + "_grass_params.txt";

    // load terrain
    currscene = 0;
    cerr << "Elevation file load" << endl;
    getTerrain()->loadElv(terfile);
    scf = getTerrain()->getMaxExtent();
    getView()->setForcedFocus(getTerrain()->getFocus());
    getView()->setViewScale(getTerrain()->longEdgeDist());
    getView()->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
    getTerrain()->calcMeanHeight();
    getTerrain()->updateBuffers(renderer); // NB - set terrain width and height in renderer.

    int scale_down = get_terscale_down(scale_size);

    // match dimensions for empty overlay
    int dx, dy;
    getTerrain()->getGridDim(dx, dy);
    ipc_received_raw->setDim(dx / scale_down, dy / scale_down);
    cerr << "terrain dimensions = " << dx << " " << dy << endl;
    getTypeMap(TypeMapType::EMPTY)->matchDim(dx, dy);
    getTypeMap(TypeMapType::PAINT)->matchDim(dx, dy);
    getTypeMap(TypeMapType::PAINT)->fill((int) BrushType::FREE);
    getTypeMap(TypeMapType::PRETTY_PAINTED)->matchDim(dx, dy);

    if (dx * dy > 0)
    {
        getSim()->set_terrain(getTerrain());
        terloaded = true;
    }
    else
        throw runtime_error("Could not import terrain, or imported terrain is invalid (either width or height is zero)");


    // Region R =  getTypeMap(TypeMapType::PAINT)->getRegion();
    // cerr << "R: " << R.x0 << " " << R.x1 << " " << R.y0 << " " << R.y1 << endl;
    std::cout << "Loading canopy density model..." << std::endl;
    if(getCanopyDensityModel()->read(cdmfile))
    {
        loadTypeMap(getCanopyDensityModel(), TypeMapType::CDM);
        cerr << "CDM file load" << endl;
    }
    else
    {
        cerr << "No Canopy Density Model found. Simulation invalidated." << endl;
        simvalid = false;
    }

    std::cout << "Loading canopy height model..." << std::endl;
    if(getCanopyHeightModel()->read(chmfile))
    {
        cerr << "CHM file load" << endl;
        loadTypeMap(getCanopyHeightModel(), TypeMapType::CHM);
    }
    else
    {
        cerr << "No Canopy Height Model found. Simulation invalidated." << endl;
        simvalid = false;
    }

    int chmw, chmh;
    getCanopyHeightModel()->getDim(chmw, chmh);
    getPlacerCanopyHeightModel()->setDim(chmw, chmh);


//#ifndef PAINTCONTROL
    //if(getBiome()->read(bmefile))
    if (getBiome()->read_dataimporter(prj_src_dir + "/ecodata/sonoma.db"))
    {
        amaps_ptr = unique_ptr<abiotic_maps_package>(new abiotic_maps_package(base_dirname, abiotic_maps_package::suntype::LANDSCAPE_ONLY, abiotic_maps_package::aggr_type::AVERAGE));

        if (plantvis.size() < getBiome()->numPFTypes())
            plantvis.resize(getBiome()->numPFTypes());
        cerr << "Biome file load" << endl;
        for(int t = 0; t < getBiome()->numPFTypes(); t++)
            plantvis[t] = true;

        std::cerr << "Running GLWidget::initSceneSim..." << std::endl;
        // initialize simulation
        //initSceneSim();

        std::cerr << "Reading climate parameters..." << std::endl;
        // read climate parameters
        if(!getSim()->readClimate(climfile))
        {
            simvalid = false;
            cerr << "No climate file " << climfile << " found. Simulation invalidated" << endl;
        }

        std::cerr << "Computing temperature map..." << std::endl;
        getSim()->calc_temperature_map();		// put this inside the readClimate function?

        getSim()->copy_map(amaps_ptr->sun, abiotic_factor::SUN);
        sunloaded = true;

        sun_mth = 0;
        //loadTypeMap(getSunlight(sun_mth), TypeMapType::SUNLIGHT);
        loadTypeMap(getSlope(), TypeMapType::SLOPE);

        getSim()->copy_map(amaps_ptr->wet, abiotic_factor::MOISTURE);
        wetloaded = true;

        std::cerr << "Calculating average moisture map..." << std::endl;
        wet_mth = 0;
        loadTypeMap(getMoisture(), TypeMapType::WATER);

        // read landscape category data
        if(getTypeMap(TypeMapType::CATEGORY)->loadCategoryImage(catfile))
        {
            cerr << "Region usage file load" << endl;
        }
        else
        {
            simvalid = false;
            cerr << "No land usage category image " << catfile << " found. Simulation invalidated" << endl;
        }

        // loading plant distribution
        getEcoSys()->setBiome(getBiome());


        getEcoSys()->pickAllPlants(getTerrain());
        getEcoSys()->redrawPlants();

        auto vparams = data_importer::read_grass_viability(grassparamsfile);
        getGrass()->set_viability_params(vparams);

        if (terloaded && sunloaded && wetloaded)
        {
            float rw, rh;
            getTerrain()->getGridDim(dx, dy);
            getTerrain()->getTerrainDim(rw, rh);

            std::cout << "Terrain grid dimensions: " << dx << ", " << dy << std::endl;
            std::cout << "Terrain real dimensions: " << rw << ", " << rh << std::endl;

            std::cout << "growing grass..." << std::endl;
            grow_grass();
            std::cout << "setting pretty map..." << std::endl;
            set_pretty_map();
            std::cout << "done" << std::endl;
        }

    }
    else
    {
        std::cerr << "Biome file " << bmefile << "does not exist. Simulation invalidated." << endl;
    }
//#endif
    // focuschange = true;
    setOverlay(TypeMapType::PAINT);

    getTerrain()->setBufferToDirty();	// force render reload, so that we have a valid initial texture

    sceneloaded = true;

    float tw, th;
    getTerrain()->getTerrainDim(tw, th);

    setPlantsVisibility(true);
}


void GLWidget::saveScene(std::string dirprefix)
{
    if (!sceneloaded)
        return;
    std::string terfile = dirprefix+".elv";
    std::string canopyfile = dirprefix+"_canopy.pdb";
    std::string undergrowthfile = dirprefix + "_undergrowth.pdb";
    std::string grassfile = dirprefix + "_grass.txt";
    std::string litfile = dirprefix + "_litterfall.txt";

    getTerrain()->saveElv(terfile, 0.3048);
    if (canopytrees.size() > 0)
    {
        data_importer::write_pdb(canopyfile, canopytrees.data(), canopytrees.data() + canopytrees.size());
        data_importer::write_txt<MapFloat>(grassfile, getGrass()->get_data());
        data_importer::write_txt<MapFloat>(litfile, getGrass()->get_litterfall_data());
    }
    if (underplants.size() > 0)
        data_importer::write_pdb(undergrowthfile, underplants.data(), underplants.data() + underplants.size());

}

void GLWidget::writePlants(std::string plantfile)
{
    if(!dbloaded || !ecoloaded) // an ecosystem database must be present
    {
        // display message to prompt user
        QMessageBox::information(
            this,
            tr("EcoSys"),
            tr("There are no plants to save.") );
    }
    else
    {
        getEcoSys()->getPlants()->writePDB(plantfile);
    }
}

void GLWidget::writePaintMap(std::string paintfile)
{
    getTypeMap(TypeMapType::PAINT)->saveToPaintImage(paintfile);
}

void GLWidget::writeCanopyHeightModel(std::string chmfile)
{
    getTypeMap(TypeMapType::CHM)->saveToGreyscaleImage(chmfile, 250.0f, true);
}

void GLWidget::addScene()
{
    Scene * scene = new Scene();
    scene->view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());

    std::fill(plantvis.begin(), plantvis.end(), false);
    //for(int t = 0; t < maxpftypes; t++)
    //    plantvis[t] = false;
    scenes.push_back(scene);
    currscene = (int) scenes.size() - 1;
}

void GLWidget::setScene(int s)
{
    if(s >= 0 && s < (int) scenes.size())
    {
        currscene = s;
        getTerrain()->setBufferToDirty();
        getTerrain()->setAccelInValid();
        refreshOverlay();
        update();
    }
}


void GLWidget::loadDecals()
{
    QDir src_basedir = QString(SRC_BASEDIR);
    QImage decalImg, t;

    if(!decalImg.load(src_basedir.filePath("Icons/manipDecals.png")))
        cerr << src_basedir.filePath("Icons/manipDecals.png").toStdString() << " not found" << endl;

    // Qt prep image for OpenGL
    QImage fixedImage(decalImg.width(), decalImg.height(), QImage::Format_ARGB32);
    QPainter painter(&fixedImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(fixedImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage( 0, 0, decalImg);
    painter.end();

    // probe fixedImage
    // cerr << "Decal alpha channel = " << fixedImage.hasAlphaChannel() << endl;
    // QRgb pix = fixedImage.pixel(84, 336);
    // cerr << "Decal[84][336] = " << (int) qRed(pix) << ", " << (int) qGreen(pix) << ", " << (int) qBlue(pix) << ", " << (int) qAlpha(pix) << endl;
    // t = QGLWidget::convertToGLFormat( fixedImage );
    t = QGLWidget::convertToGLFormat( fixedImage );

    renderer->bindDecals(t.width(), t.height(), t.bits());
    decalsbound = true;
}

void GLWidget::loadTypeMap(const QImage &img, TypeMapType purpose)
{
    getTypeMap(purpose)->load(img, purpose);
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
        case TypeMapType::GRASS:
            numClusters = getTypeMap(purpose)->convert(map, purpose, MAXGRASSHGHT);
            break;
        case TypeMapType::PRETTY_PAINTED:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 0);
            break;
        case TypeMapType::PRETTY:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 0);
            break;
        case TypeMapType::CLUSTER:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 0);
            break;
        case TypeMapType::CLUSTERDENSITY:
            numClusters = getTypeMap(purpose)->convert(map, purpose, 0);
            break;
        default:
            break;
    }
    return numClusters;
}

void GLWidget::floodSea(Terrain * ter, MapFloat * wet, float sealevel, float seaval)
{   char * cmap;
    int dx, dy, p = 0;

    const float * grid = ter->getGridData(dx, dy);

    for(int i = 0; i < dx; i++)
        for(int j = 0; j < dy; j++)
        {
            if(grid[p] <= sealevel)
                wet->set(j, i, seaval);
            p++;
        }
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

    // *** PM REnder code - start ***

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

    // *** PM REnder code - end ***

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_TEXTURE_2D);

    loadDecals();

    glsun->init_gl();
}

void GLWidget::paintGL()
{
    vpPoint mo;
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    uts::vector<ShapeDrawData> drawParams; // to be passed to terrain renderer
    Shape shape;  // geometry for focus indicator
    std::vector<Shape>::iterator sit;
    std::vector<glm::mat4> sinst;
    std::vector<glm::vec4> cinst;

    Timer t;
    t.start();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(focuschange && focusviz)
    {
        ShapeDrawData sdd;
        float scale;
   char * cmap;
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

    if(focuschange)
    {
        // visualization of random hemispheric sampling
        /*
        SunLight * sun = new SunLight();
        sun->bindDiffuseSun(getView(), getTerrain());
        sun->drawSun(drawParams);
        delete sun;
        */

        // visualization of occluding canopy
        /*
        CanopyShape * suncanopy = new CanopyShape(0.5, getTerrain()->getCellExtent());
        suncanopy->bindCanopy(getTerrain(), getView(), getCanopyHeightModel(), getCanopyDensityModel());
        suncanopy->drawCanopy(drawParams);
        delete suncanopy;*/
    }

    if (focuschange)
    {
        getEcoSys()->bindPlantsSimplified(getTerrain(), drawParams);
        int nonzero_count = 0;
        for (auto &dp : drawParams)
        {
            if (dp.VAO > 0)
                nonzero_count++;
        }
    }

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

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport(0, 0, width, height);
    // cerr << "GL: " << (int) width << " " << (int) height << " WINDOW " << this->width() << " " << this->height() << endl;

    // apply to all views
    for(int i = 0; i < (int) scenes.size(); i++)
    {
        // cerr << "VIEW RESIZED" << endl;
        //scenes[i]->view->setDim((float) ((width - side) / 2), (float) ((height - side) / 2), (float) side, (float) side);
        scenes[i]->view->setDim(0.0f, 0.0f, (float) this->width(), (float) this->height());
        scenes[i]->view->apply();
    }
}


void GLWidget::read_pdb_canopy(std::string pathname)
{
    canopytrees = data_importer::read_pdb(pathname);
    canopytrees_indices = false;

    // calculate quick and dirty canopy shading, based on just reducing grass height under each
    // tree based on its alpha value, then smoothing
    calc_fast_canopyshading();

    redrawPlants(false, layerspec::ALL, clearplants::YES);

    repaint();
}

bool GLWidget::hasSceneLoaded()
{
    return sceneloaded;
}

void GLWidget::read_pdb_undergrowth(std::string pathname)
{
    underplants = data_importer::read_pdb(pathname);

    redrawPlants(false, layerspec::ALL, clearplants::YES);

    repaint();

}

void GLWidget::setSpeciesPercentages(const std::vector<float> &perc)
{
    species_percentages = perc;
}

void GLWidget::convert_canopytrees_to_real_species()
{
    if (canopytrees_indices)
        for (auto &ct : canopytrees)
        {
            ct.species = specidxes.at(ct.species);
        }
    canopytrees_indices = false;
}

void GLWidget::convert_canopytrees_to_indices()
{
    if (!canopytrees_indices)
        for (auto &ct : canopytrees)
        {
            for (int i = 0; i < specidxes.size(); i++)
            {
                if (specidxes.at(i) == ct.species)
                {
                    ct.species = i;
                }
            }
        }
    canopytrees_indices = true;
}

void GLWidget::redrawPlants(bool repaint_here, layerspec layer, clearplants clr)
{
    report_cudamem("GPU memory in use at start of redrawPlants: ");

    if (clr == clearplants::YES)
        getEcoSys()->clearAllPlants(getTerrain());

    bool placecanopy = layer == layerspec::CANOPY || layer == layerspec::ALL;
    bool placeunder = layer == layerspec::UNDERGROWTH || layer == layerspec::ALL;

    if (show_canopy && placecanopy)
    {
        bool prevset = canopytrees_indices;
        convert_canopytrees_to_real_species();
        getEcoSys()->placeManyPlants(getTerrain(), canopytrees, true);
        if (prevset)
            convert_canopytrees_to_indices();
    }

    if (synth_undergrowth && show_undergrowth && placeunder)
    {
        getEcoSys()->placeManyPlants(getTerrain(), underplants, false);
    }

    getEcoSys()->redrawPlants();
    if (repaint_here)
    {
        GL_ERRCHECK(false);
        repaint();
        GL_ERRCHECK(false);	// TODO: figure out why we get a GL error here. It does not seem to affect the output, but it needs to be checked still
    }
    report_cudamem("GPU memory in use at end of redrawPlants: ");
}

void GLWidget::import_canopyshading(std::string canopyshading)
{
    canopyshading_temp = data_importer::average_mmap<ValueGridMap<float>, ValueGridMap<float>>(data_importer::read_monthly_map<ValueGridMap<float> >(canopyshading));

    float rw, rh;
    getTerrain()->getTerrainDim(rw, rh);
    canopyshading_temp.setDimReal(rw, rh);
}


void GLWidget::report_cudamem(std::string msg) const
{
    size_t freemem, totalmem, inuse;
    inuse = totalmem - freemem;
    inuse /= 1024 * 1024;
    std::cout << msg << " " << inuse << "MB" << std::endl;

}

void GLWidget::reset_common_maps()
{
    std::cout << "Setting pretty map..." << std::endl;

    // set the map that contains indicators for grass, rivers and rock (requires grass simulation to be finished)
    set_pretty_map();		// resetting the entire pretty map might be a little slow. Find alternative?
    loadTypeMap(&pretty_map_painted, TypeMapType::PRETTY_PAINTED);
    loadTypeMap(&pretty_map, TypeMapType::PRETTY);

}

void GLWidget::calc_fast_canopyshading()
{
    float rw, rh;
    getTerrain()->getTerrainDim(rw, rh);

    //calculate updated sunlight based on new canopy trees (quick and dirty)
    getSim()->calc_adaptsun(canopytrees, cdata, rw, rh);
    MapFloat *adaptsun = getSim()->get_adaptsun();

    // do some smoothing on the fast canopy shading calculation
    int dx, dy, dx2, dy2;
    canopyshading_temp.getDim(dx, dy);
    adaptsun->getDim(dx2, dy2);
    if (dx != dx2 || dy != dy2)
    {
        canopyshading_temp.setDim(dx2, dy2);
        canopyshading_temp.setDimReal(rw, rh);
    }
    //smooth_uniform_radial(15, adaptsun->data(), adaptsun->data(), dx2, dy2);
    memcpy(canopyshading_temp.data(), adaptsun->data(), sizeof(float) * dx2 * dy2);
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_A) // 'A' for animated spin around center point of terrain
    {
        // change focal point to center
        getTerrain()->setMidFocus();
        getView()->setForcedFocus(getTerrain()->getFocus());
        getView()->startSpin();
        rtimer->start(20);
    }

    if(event->key() == Qt::Key_C) // 'C' to show canopy height model texture overlay
    {
        setOverlay(TypeMapType::CHM);
        redrawPlants(true, layerspec::ALL, clearplants::YES);
    }

    if(event->key() == Qt::Key_D) // 'D' switch to painting overlay
    {
        setOverlay(TypeMapType::PAINT);
    }

    if(event->key() == Qt::Key_E) // 'E' to remove all texture overlays
    {
        cerr << "overlay changed to empty" << endl;
        setOverlay(TypeMapType::EMPTY);
        refreshOverlay();
    }

    if(event->key() == Qt::Key_F) // 'F' to toggle focus stick visibility
    {
        if(focusviz)
            focusviz = false;
        else
            focusviz = true;
        update();
    }

    if(event->key() == Qt::Key_I) // 'I' for close ueval screencap
    {
        cerr << "image capture: close up" << endl;
        getTerrain()->setMidFocus();
        getView()->setForcedFocus(getTerrain()->getFocus());
        getView()->closeview();

        QImage cap;
        screenCapture(&cap, QSize(500,500));
        cap.save(QCoreApplication::applicationDirPath() + "/../close.png");
        update();
    }
    if (event->key() == Qt::Key_J)
    {
        loadTypeMap(getGrass()->get_data(), TypeMapType::GRASS);
        setOverlay(TypeMapType::GRASS);
    }
    if (event->key() == Qt::Key_L)
    {
        setOverlay(TypeMapType::PRETTY_PAINTED);
    }
    if (event->key() == Qt::Key_M)
    {
        setOverlay(TypeMapType::SPECIES);
    }
    if (event->key() == Qt::Key_N)
    {
        show_undergrowth = !show_undergrowth;
        redrawPlants();
    }
    if(event->key() == Qt::Key_P) // 'P' to toggle plant visibility
    {
        if(focuschange)
            focuschange = false;
        else
            focuschange = true;
        update();
    }
    if(event->key() == Qt::Key_R) // 'R' to show temperature texture overlay
    {
        loadTypeMap(getSlope(), TypeMapType::SLOPE);
        setOverlay(TypeMapType::SLOPE);
    }
    if(event->key() == Qt::Key_S) // 'S' to show sunlight texture overlay
    {
        int gw, gh;
        canopyshading_temp.getDim(gw, gh);
        MapFloat *sun = new MapFloat;
        sun->setDim(gw, gh);

        memcpy(sun->data(), canopyshading_temp.data(), sizeof(float) * gw * gh);

        loadTypeMap(sun, TypeMapType::SUNLIGHT);
        setOverlay(TypeMapType::SUNLIGHT);

        delete sun;
    }
    if(event->key() == Qt::Key_T) // 'T' to show slope texture overlay
    {
        loadTypeMap(getSim()->get_temperature_map(), TypeMapType::TEMPERATURE);
        setOverlay(TypeMapType::TEMPERATURE);
    }
    if(event->key() == Qt::Key_V) // 'V' for top-down view
    {
        cerr << "top down view" << endl;
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
        //loadTypeMap(getMoisture(wet_mth), TypeMapType::WATER);
        loadTypeMap(getSim()->get_average_moisture_map(), TypeMapType::WATER);
        setOverlay(TypeMapType::WATER);
    }
    if (event->key() == Qt::Key_X)
    {
        setOverlay(TypeMapType::PRETTY);
    }
    if (event->key() == Qt::Key_Y)
    {
        show_canopy = !show_canopy;
        redrawPlants();
    }

    /*
     * // Currently, showing the clusters and their densities on the landscape is not implemented, since we would
     * // have to provide for both the GPU and CPU version.
     * // TODO: refactor so that clusters are computed once, before synthesizing undergrowth, on the GPU so that it is
     * // 		done fast, and also so that clusters are available to the interface without synthesizing undergrowth
    if (event->key() == Qt::Key_Z)
    {
        setOverlay(TypeMapType::CLUSTER);
    }
    if (event->key() == Qt::Key_0)
    {
        setOverlay(TypeMapType::CLUSTERDENSITY);
    }
    */
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    vpPoint pnt;
    
    int x = event->x(); int y = event->y();
    float W = (float) width(); float H = (float) height();

    update(); // ensure this viewport is current for unproject

    // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
    if(!viewlock && (event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton))
    {
        // arc rotate in perspective mode
  
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        lastPos = event->pos();
        getView()->startArcRotate(nx, ny);
        viewing = true;
    }
    else if (!viewlock && (event->modifiers() == Qt::ControlModifier))
    {
        vpPoint frompnt, topnt;
        getView()->projectingPoint(x, y, frompnt);
        terrainProject(frompnt, topnt, getView(), getTerrain());

        getView()->setForcedLocation(topnt.x, topnt.y + 2.0f, topnt.z);
        lastPos = event->pos();
        update();
    }
    else if (!viewlock && (event->modifiers() == Qt::ShiftModifier))
    {
        lastPos = event->pos();
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
            focuschange = true; //focusviz = true;
            atimer->start(10);
        }
        // ignores pick if terrain not intersected, should possibly provide error message to user
    }
    if(!viewlock && event->buttons() == Qt::LeftButton) // provide info on terrain
    {
        if(getTerrain()->pick(sx, sy, getView(), pnt))
        {
            if(!decalsbound)
                loadDecals();
            // TO DO: depending on viewing mode, may find sample in DB or value from terrain
            // getEcoSys()->displayCursorSampleDB(getTerrain(), pnt);
        }
    }
}

void GLWidget::setPlantsVisibility(bool visible)
{
    focuschange = visible;
}

void GLWidget::setCanopyVisibility(bool visible)
{
    show_canopy = visible;
}

void GLWidget::setUndergrowthVisibility(bool visible)
{
    show_undergrowth = visible;
}

void GLWidget::pickInfo(int x, int y)
{
   std::string catName;
   int gw, gh;
   bool canopyshading = false;
   canopyshading_temp.getDim(gw, gh);
   if (gw > 0 && gh > 0)
   {
       canopyshading = true;
   }

   cerr << endl;
   cerr << "*** PICK INFO ***" << endl;
   cerr << "location: " << x << ", " << y << endl;
   Simulation *sim = getSim();
   if (sim->hasTerrain())
   {
       sim->pickInfo(x, y);
       cerr << "Elevation (m): " << getTerrain()->getHeight(x, y) << std::endl;
       cerr << "Canopy Height (m): " << getCanopyHeightModel()->get(x, y) * 0.3048f  << endl;
       if (canopyshading)
            cerr << "Canopy shading: " << canopyshading_temp.get(x, y) << endl;
   }
}

int GLWidget::get_terscale_down(int req_dim)
{
    int terw, terh;
    getTerrain()->getGridDim(terw, terh);
    float scf_w = terw / (float)req_dim;
    float scf_h = terh / (float)req_dim;
    //assert(abs(scf_h - scf_w) < 1e-3);		// for now, assume that scf_w and scf_h must be the same. TODO: allow to be different
    //assert(abs(scf_h - round(scf_h)) < 1e-3); 	// assume scale factor is an int
    int scale_down = int(round(scf_h));
    return scale_down;
}

void GLWidget::write_chmfiles()
{
    std::string chmout = pipeout_dirname + "/chm_" + std::to_string(nspecassign) + "_" + std::to_string(nchmfiles) + ".txt";
    std::string chmout_upsampled = pipeout_dirname + "/chm_upsampled_" + std::to_string(nspecassign) + "_" + std::to_string(nchmfiles) + ".txt";
    data_importer::write_txt(chmout, ipc_received_raw);
    data_importer::write_txt(chmout_upsampled, getCanopyHeightModel());
}

void GLWidget::set_ipc_scaling()
{
    // get dimensions of output received from neural net
    ipc_received_raw->getDim(ipc_scaling.srcw, ipc_scaling.srch);

    // get terrain dimensions and required scaling factor for neural net input
    getTerrain()->getGridDim(ipc_scaling.terw, ipc_scaling.terh);

    float wscale = ipc_scaling.terw / (float)ipc_scaling.srcw;
    float hscale = ipc_scaling.terh / (float)ipc_scaling.srch;
    assert(abs(wscale - hscale) < 1e-3);		// for now, assume that hscale and wscale must be the same. TODO: allow to be different
    assert(abs(wscale - round(wscale)) < 1e-3);	// also only allow integer scaling sizes, for now. TODO: allow float scaling sizes...
    ipc_scaling.intscale = round(wscale);

}

void GLWidget::correct_chm_scaling()
{
    MapFloat *chm = getCanopyHeightModel();		// TODO: scale CHM data from the 0 to 65535 range to 0 to 400
    float *chmdata = chm->data();

    // FIXME: remove this hack, together with the corresponding loop below
    float max_height = 0.0f;
    for (int i = 0; i < chm->width() * chm->height(); i++)
    {
        chmdata[i] *= 400.0f / 65535.0f;
        if (chmdata[i] < 1.0f)
        {
            chmdata[i] = 0.0f;
        }
        if (chmdata[i] > max_height) max_height = chmdata[i];
    }
    std::cout << "Maximum height: " << max_height << std::endl;

}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{

    //renderer->updateTypeMapTexture(getTypeMap(TypeMapType::PAINT));

    viewing = false;
    if(event->button() == Qt::LeftButton && cmode == ControlMode::VIEW) // info on terrain cell
    {
        vpPoint pnt;
        int sx, sy;

        sx = event->x(); sy = event->y();

        if (getTerrain()->pick(sx, sy, getView(), pnt))
        {
            int x, y;
            getTerrain()->toGrid(pnt, x, y);
            pickInfo(x, y);
        }
    }
}

void GLWidget::reset_filecounts()
{
    nundergrowthfiles = 0;
}

void GLWidget::update_prettypaint(int x, int y, float radius)
{
    float fgx, fgy, fgh;
    int gx, gy, gh;
    float irad, sqirad;
    int sx, ex, sy, ey;
    int dx, dy;
    vpPoint prjpnt, terpnt;
    getView()->projectingPoint(x, y, prjpnt);
    bool valid = terrainProject(prjpnt, terpnt, getView(), getTerrain());
    getTerrain()->toGrid(terpnt, fgx, fgy, fgh);
    gx = fgx; gy = fgy; gh = fgh;
    irad = getTerrain()->toGrid(radius);
    getTerrain()->getGridDim(dx, dy);
    sqirad = irad * irad;

    sx = gx - irad;
    sy = gy - irad;
    ey = gx + irad;
    ey = gy + irad;
    sx = sx < 0 ? 0 : (sx >= dx ? dx - 1 : sx);
    ex = ex < 0 ? 0 : (ex >= dx ? dx - 1 : ex);
    sy = sy < 0 ? 0 : (sy >= dx ? dx - 1 : sy);
    ey = ey < 0 ? 0 : (ey >= dx ? dx - 1 : ey);

    TypeMap *tmap = getTypeMap(TypeMapType::PRETTY_PAINTED);

    #pragma omp parallel for
    for (int cx = sx; cx <= ex; cx++)
    {
        for (int cy = sy; cy <= ey; cy++)
        {
            float diffx = cx - fgx; diffx *= diffx;
            float diffy = cy - fgy; diffy *= diffy;
            if (diffx + diffy <= sqirad)
            {
                BrushType btype = brush.getBrushType();
                int ibtype = (int)btype;
                float prevval = pretty_map_painted.get(cx, cy);
                int prevcat = prevval / 1000000;
                pretty_map_painted.set(cx, cy, prevval - prevcat * 1000000);	// first remove the class encoded into the value of the pretty map
                pretty_map_painted.set(cx, cy, pretty_map_painted.get(cx, cy) + ibtype * 1000000);		// now add the new class given by the current brush
                tmap->set(cx, cy, pretty_map_painted.get(cx, cy));	// update type map directly also, instead of using the loadTypeMap function to do everything each time
            }
        }
    }
}

void GLWidget::update_species_brushstroke(int x, int y, float radius, int specie)
{

    float fgx, fgy, fgh;
    int gx, gy, gh;
    float irad, sqirad;
    int sx, ex, sy, ey;
    int dx, dy;
    vpPoint prjpnt, terpnt;
    getView()->projectingPoint(x, y, prjpnt);
    bool valid = terrainProject(prjpnt, terpnt, getView(), getTerrain());
    getTerrain()->toGrid(terpnt, fgx, fgy, fgh);
    gx = fgx; gy = fgy; gh = fgh;
    irad = getTerrain()->toGrid(radius);
    std::swap(gx, gy);		// have to swap these, since landscape xy's are the other way around
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

    else if (!viewlock && ((event->buttons() & (Qt::RightButton | Qt::LeftButton)) && event->modifiers() == Qt::ShiftModifier))
    {
        vpPoint frompnt, topnt1, topnt2;
        getView()->projectingPoint(lastPos.x(), lastPos.y(), frompnt);
        terrainProject(frompnt, topnt1, getView(), getTerrain());
        getView()->projectingPoint(x, y, frompnt);
        terrainProject(frompnt, topnt2, getView(), getTerrain());

        //std::cout << "Terrain point: " << topnt.x << ", " << topnt.z << std::endl;
        getView()->startPan(topnt1.x, topnt1.z);
        getView()->pan(topnt2.x, topnt2.z);
        lastPos = event->pos();

        update();
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
            del = (float) pix.y() * 30.0f;
            getView()->incrZoom(del);
            update();

        }
        else if(!deg.isNull()) // mouse wheel instead
        {
            del = (float) -deg.y() * 30.0f;
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

void GLWidget::initCHM(int w, int h)
{
    getCanopyHeightModel()->setDim(w, h);
    getCanopyHeightModel()->initMap();

    getPlacerCanopyHeightModel()->setDim(w, h);
    getPlacerCanopyHeightModel()->initMap();
}
