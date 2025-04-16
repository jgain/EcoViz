/*******************************************************************************
 *
 * EcoViz
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

#include "scene.h"
#include "eco.h"
#include "hash_table.h"

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <fstream>
#include "data_importer/data_importer.h"
#include "data_importer/map_procs.h"
#include <QMessageBox>
#include <QElapsedTimer>
#include <qdir.h>

class ElapsedTimer
{
public:
    ElapsedTimer(std::string name) { cap = name; qet.start(); }
    ~ElapsedTimer() { cerr << "Time: " << cap << ": " << qet.elapsed() << "ms"; }
    void elapsed(std::string tx) { cerr << "Timer " << cap << " - " << tx << ": " << qet.elapsed() << "ms"; }
private:
   QElapsedTimer qet;
   std::string cap;

};


//// Transect

bool Transect::inBounds(vpPoint pnt, Terrain * ter)
{
    Plane b[4];
    bool inside = true;

    // determine bounding plane intersects
    float maxx, maxy;
    ter->getTerrainDim(maxx, maxy);

    b[0].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)); // left edge
    b[1].formPlane(vpPoint(maxy, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f)); // right edge
    b[2].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f)); // front edge
    b[3].formPlane(vpPoint(0.0f, 0.0f, maxx), Vector(0.0f, 0.0f, -1.0f)); // back edge

    for(int i = 0; i < 4; i++)
        if(!b[i].side(pnt)) // must be on the correct side of all the bounding planes
            inside = false;
    return inside;
}

bool Transect::findBoundPoints(vpPoint src, Vector dirn, vpPoint * bnd, Terrain * ter)
{
    Plane b[4];
    Vector offset;
    std::list<float> t, tsort;

    // determine bounding plane intersects
    float maxx, maxy;
    ter->getTerrainDim(maxy, maxx);

    cerr << "maxx = " << maxx << " and maxy = " << maxy << endl;

    b[0].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)); // left edge
    b[1].formPlane(vpPoint(maxx, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)); // right edge
    b[2].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f)); // front edge
    b[3].formPlane(vpPoint(0.0f, 0.0f, maxy), Vector(0.0f, 0.0f, 1.0f)); // back edge

    for(int i = 0; i < 4; i++)
    {
        float tval;
        if(!b[i].rayPlaneIntersect(src, dirn, tval))
        {
            if(i%2==0)
                tval = numeric_limits<float>::min();
            else
                tval = numeric_limits<float>::max();
        }
        t.push_back(tval);
    }

    // sort by t and use inner pair
    tsort = t;
    tsort.sort();
    auto its = tsort.begin();
    its++;
    float tstart = (* its);
    its++;
    float tend = (* its);

    // find corresponding points but tvalues shift slightly to ensure they intersect the terrain
    offset = dirn;
    offset.mult(tstart + 0.1f);
    offset.pntplusvec(src, &bnd[0]);
    offset = dirn;
    offset.mult(tend - 0.1f);
    offset.pntplusvec(src, &bnd[1]);
    ter->drapePnt(bnd[0], bnd[0]);
    ter->drapePnt(bnd[1], bnd[1]);

    // check to see that both points are in bounds, return false otherwise
    for(int i = 0; i < 2; i++)
    {
        if(bnd[i].x < 0.0f || bnd[i].x > maxx)
            return false;
        if(bnd[i].z < 0.0f || bnd[i].z > maxy)
            return false;
    }
    return true;
}

void Transect::paintThickness(Terrain * ter)
{
    Plane offset[2];
    Vector offvec;
    vpPoint offpnt, vizpnt;
    int dx, dy;

    // establish offset planes to bound the transect thickness
    // cerr << "normal = " << normal.i << ", " << normal.j << ", " << normal.k << endl;
    offvec = normal;
    offvec.mult(thickness / 2.0f);
    offvec.pntplusvec(center, &offpnt);
    // cerr << "offpnt = " << offpnt.x << ", " << offpnt.y << ", " << offpnt.z << endl;
    // cerr << "center = " << center.x << ", " << center.y << ", " << center.z << endl;
    offset[0].formPlane(offpnt, normal);
    offvec.mult(-1.0f);
    offvec.pntplusvec(center, &offpnt);
    offvec.normalize();
    offset[1].formPlane(offpnt, offvec);

    // test if grid points on vizmap lie between offset planes
    ter->getGridDim(dx, dy);
    for(int y = 0; y < dy; y++)
        for(int x = 0; x < dx; x++)
        {
            // position on terrain corresponding to grid point, projected onto the base plane
            vizpnt = ter->toWorld(y, x, 0.0f); // JG/PCM - orientation flip
            if(!offset[0].side(vizpnt) && !offset[1].side(vizpnt)) // between planes so draw in red
                mapviz->set(x, y, 1.0f); // PCM:  more flipping (ensure map  indices are valid, adjusted other values too)
            else
                mapviz->set(x, y, 0.0f);
        }
}

std::pair<Plane, Plane>  Transect::getTransectPlanes(vpPoint &basePlaneOrigin)
{
    Plane offset[2];
    Vector offvec;
    vpPoint offpnt, vizpnt;

    // establish offset planes to bound the transect thickness
    // cerr << "normal = " << normal.i << ", " << normal.j << ", " << normal.k << endl;
    offvec = normal;
    offvec.mult(thickness / 2.0f);
    offvec.pntplusvec(center, &offpnt);

    basePlaneOrigin = offpnt;

    // cerr << "offpnt = " << offpnt.x << ", " << offpnt.y << ", " << offpnt.z << endl;
    // cerr << "center = " << center.x << ", " << center.y << ", " << center.z << endl;
    offset[0].formPlane(offpnt, normal);
    offvec.mult(-1.0f);
    offvec.pntplusvec(center, &offpnt);
    offvec.normalize();
    offset[1].formPlane(offpnt, offvec);



    return std::make_pair(offset[0], offset[1]);
}

/**
 * @brief derive Derive a transect passing through two points on the terrain
 * @param p1    first point
 * @param p2    second point
 */
void Transect::derive(vpPoint p1, vpPoint p2, Terrain * ter)
{
    Vector align, offset, pntsep;
    vpPoint np1, np2, swap;
    Plane b[4];
    std::list<float> t, tsort;

    // if necessary, swap p1 and p2
    np1 = p1; np2 = p2;
    if(np1.x > np2.x)
    {
        swap = np1;
        np1 = np2;
        np2 = swap;
    }

    // calculate normal
    align.diff(np1, np2);
    align.j = 0.0f;
    pntsep = align;
    align.normalize();
    normal = Vector(-align.k, 0.0f, align.i);

    findBoundPoints(np1, align, bounds, ter);
    // calculate key parameters
    setInnerStart(np1, ter); setInnerEnd(np2, ter);
    hori = align;
    vert = Vector(0.0f, 1.0f, 0.0f);
    align.diff(bounds[0], bounds[1]);
    align.j = 0.0f;
    extent = pntsep.length();
    pntsep.mult(0.5f);
    pntsep.pntplusvec(np1, &center);
    ter->drapePnt(center, center);
    setThickness(10.0f, ter);
    //if(!valid) // signal to make transect visible

    valid = true;
}

void Transect::zoom(float zdel, Terrain * ter)
{
    vpPoint c, i;
    Vector tostart, toend;

    float newextent = std::max(thickness, extent+zdel);
    float scf = newextent/extent;
    extent = newextent;

    // adjust inner start
    c = center; c.y = 0.0f;
    i = inners[0]; i.y = 0.0f;
    tostart.diff(c, i);
    tostart.mult(scf);
    tostart.pntplusvec(center, &i);
    ter->drapePnt(i, i);
    setInnerStart(i, ter);

    // adjust inner end
    toend = tostart;
    toend.mult(-1.0f); // opposite direction
    toend.pntplusvec(center, &i);
    ter->drapePnt(i, i);
    setInnerEnd(i, ter);

    redraw = true;
}

//// TimelineGraph

std::vector<std::string> TimelineGraph::graph_titles = {"Basal area", "Stem number", "DBH distribution"};

TimelineGraph::TimelineGraph()
{
    timeline = nullptr;
    hscale = 0; vscale = 0;
    numseries = 0;
    title = "";
}

TimelineGraph::TimelineGraph(Timeline * tline, int nseries, std::string name)
{
    title = name;
    vscale = 0;
    numseries = nseries;
    setTimeLine(tline);
}

TimelineGraph::TimelineGraph(const TimelineGraph & rhs)
{
    timeline = new Timeline;
    *timeline = *rhs.timeline; // default copy

    graphdata = rhs.graphdata;
    hscale = rhs.hscale;
    vscale = rhs.vscale;
    numseries = rhs.numseries;
    title = rhs.title;
}

TimelineGraph::~TimelineGraph()
{
    for(auto g: graphdata)
        g.clear();
    graphdata.clear();
}

void TimelineGraph::init()
{
    hscale = timeline->getNumIdx();
    for(auto g: graphdata)
        g.clear();
    graphdata.clear();

    for(int i = 0; i < numseries; i++)
    {
        std::vector<float> series;
        series.resize(hscale, 0.0f);
        graphdata.push_back(series);
    }
}

void TimelineGraph::extractDataSeries(Scene *scene, ChartType chart_type)
{
    title = graph_titles[chart_type];
    switch (chart_type){
    case ChartBasalArea:
        extractNormalizedBasalArea(scene);
        break;
    case ChartStemNumber:
        extractDBHSums(scene);
        break;
    case ChartDBHDistribution:
        break;

    };
}

void TimelineGraph::assignData(int attrib, int time, float value)
{
    // check bounds
    if(attrib < 0 || attrib >= (int) graphdata.size())
    {
        cerr << "Error TimelineGraph::assignData: out of graphdata bounds" << endl;
    }
    if(time < 0 || time >= (int) graphdata[attrib].size())
    {
        cerr << "Error TimelineGraph::assignData: out of graphdata bounds" << endl;
    }

    graphdata[attrib][time] = value;
    if(value > vscale)
        vscale = value;
}

void TimelineGraph::extractDBHSums(Scene * s)
{
    ElapsedTimer tmr("extractDBHSums");
    int vmax = 0;
    int nspecies = s->getBiome()->numPFTypes();
    float hectares = s->getMasterTerrain()->getTerrainHectArea();
    setNumSeries(nspecies);

    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {

        std::vector<basic_tree> trees(s->sampler->sample(s->cohortmaps->get_map(t), nullptr));
        std::vector<basic_tree> mature = s->cohortmaps->get_maturetrees(t);
        tmr.elapsed("sampler");
   std::cerr << "\n Ntrees = " << trees.size() << "; Nmature = " << mature.size() << "\n";
        for(auto &tree: mature)
        {
            if(s->getMasterTerrain()->inGridBounds(tree.y, tree.x))
               trees.push_back(tree);
        }
        tmr.elapsed("build list");
        auto dbhs = std::vector<float>(nspecies);
        for (const auto &tree : trees) {
            dbhs[tree.species] += tree.dbh;
        }
        float dbhtot = 0.f;
        for (int spc=0; spc<nspecies; ++spc) {
            dbhs[spc]/=hectares;
            assignData(spc, t, dbhs[spc]);
            dbhtot += dbhs[spc]; // cumulative sum
        }

        if(dbhtot > vmax)
            vmax = dbhtot;
        tmr.elapsed("iterate data");
    }

    setVertScale(vmax);
}

void TimelineGraph::extractNormalizedBasalArea(Scene *s)
{
    ElapsedTimer tmr("extractbasal area");
    float vmax = 0.0f;
    int nspecies = s->getBiome()->numPFTypes();
    float hectares = s->getMasterTerrain()->getTerrainHectArea();

    setNumSeries(nspecies);

    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {
        std::vector<basic_tree> trees(s->sampler->sample(s->cohortmaps->get_map(t), nullptr));
        std::vector<basic_tree> mature = s->cohortmaps->get_maturetrees(t);
        for(auto &tree: mature)
        {
            if(s->getMasterTerrain()->inGridBounds(tree.y, tree.x))
               trees.push_back(tree);
        }
        cerr << "num trees = " << (int) trees.size() << " t = " << t << endl;
        auto basal_areas = std::vector<float>(nspecies);
        for(const auto &tree: trees)  // count species
            basal_areas[tree.species] += (PI * tree.dbh*tree.dbh/ 4. / 10000.); // dbh is in cm, need to convert to m.

        float basaltot = 0.f;
        for (int spc=0; spc<nspecies; ++spc) {
            basal_areas[spc] /= hectares; // calc m2/ha
            basaltot += basal_areas[spc];
            assignData(spc, t, basal_areas[spc]);
        }
        if (basaltot > vmax)
            vmax = basaltot;
    }

    // print out graph data for debugging
    cerr << "BASAL AREA GRAPH DATA" << endl;
    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {
        cerr << "T " << t << " ";
        for (int spc=0; spc<nspecies; ++spc)
        {
            cerr <<  graphdata[spc][t] << " ";
        }
        cerr << endl;
    }
    setVertScale(vmax);
}

void TimelineGraph::extractSpeciesCounts(Scene * s)
{
    int vmax = 0;
    int nspecies = s->getBiome()->numPFTypes();
    setNumSeries(nspecies);

    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {
        int tot = 0;

        std::vector<basic_tree> trees(s->sampler->sample(s->cohortmaps->get_map(t), nullptr));
        std::vector<basic_tree> mature = s->cohortmaps->get_maturetrees(t);
        for(auto &tree: mature)
        {
            if(s->getMasterTerrain()->inGridBounds(tree.y, tree.x))
               trees.push_back(tree);
        }
        for(int spc = 0; spc < nspecies; spc++) // iterate over species
        {
            int scount = 0;
            for(auto tree: trees)  // count species
                if(tree.species == spc)
                    scount++;
            assignData(spc, t, (float) scount);
            tot += scount;
        }
        if(tot > vmax)
            vmax = tot;
    }

    setVertScale(vmax);
}

//// sceneView - for controlling loading and saving of view state

void viewScene::save(std::string filename, std::string comment)
{
    ofstream outfile;

    outfile.open((char *) filename.c_str(), ios_base::out);
    if(outfile.is_open())
    {
        outfile << region.x0 << " " << region.x1 << " " << region.y0 << " " << region.y1 << endl;
        view.save(outfile);
        outfile << comment << endl; // add as last
        outfile.close();
    }
    else
    {
        cerr << "Error viewScene::save: unable to write to file "  << filename << endl;
    }
}

void viewScene::load(std::string filename)
{
    ifstream infile;
    infile.open((char *) filename.c_str(), ios_base::in);
    std::string comment;
    if(infile.is_open())
    {
        infile >> region.x0 >> region.x1 >> region.y0 >> region.y1;
        view.load(infile);
        infile >> comment;
        cerr << comment;
        infile.close();
    }
    else
    {
        cerr << "Error viewScene::load: unable to open file " << filename << endl;
    }
}

////  mapScene - light weight class for overview map

std::string mapScene::get_dirprefix()
{
    while (datadir.back() == '/')
        datadir.pop_back();

    int slash_idx = datadir.find_last_of("/");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = datadir + "/" + setname;
    return dirprefix;
}

bool mapScene::subwindowValid(Region subwindow)
{
    int gridx, gridy;
    fullResTerrain->getGridDim(gridx, gridy);

    if (subwindow.x0 < 0 || subwindow.x0 > gridx-1)
        return false;
    if (subwindow.x1 < 0 || subwindow.x1 > gridx-1)
        return false;
    if (subwindow.y0 < 0 || subwindow.y0 > gridy-1)
        return false;
    if (subwindow.y1 < 0 || subwindow.y1 > gridy-1)
        return false;

    return true;
}

// extract the sub-region specified by region and update internal data structures
// return the new extracted terrain for later processing.
std::unique_ptr<Terrain> mapScene::extractTerrainSubwindow(Region region)
{
    if (!subwindowValid(region))
    {
        std::ostringstream oss;
        oss << "run-time error: extractTerrainSubwindow: region invalid: ++++ [x0,y0,x1,y1] = [" << region.x0 << "," << region.y0 << "," <<
               region.x1 << "," << region.y1 << "]";

        throw std::runtime_error(oss.str());
    }
    else
    {
        std::cout << " Subregion extracted -  [x0,y0,x1,y1] = [" << region.x0 << "," << region.y0 << "," <<
               region.x1 << "," << region.y1 << "]";
    }

    return fullResTerrain->buildSubTerrain(region.x0, region.y0, region.x1, region.y1);
}

// factor: default reduction factor to extract sub-region for main terrain (10 = 1/10th)
// return value = a unique_ptr to extracted Terrain  that must be managed by the caller
// noLoad = TRUE, skip loading code - data is alaeday present (loaded in a shared instance)
std::unique_ptr<Terrain> mapScene::loadOverViewData(int factor, bool noLoad)
{

    //std::string terfile = datadir+"/dem.elv";
    std::string binfile = datadir+"/" + basename + ".elvb";
    std::string txtfile = datadir+"/" + basename + ".elv";
    std::string terfile;

    bool binaryElvFile = false;

    if (std::ifstream(binfile).is_open())
    {
        binaryElvFile = true;
        terfile = binfile;
    }
    else
        terfile = txtfile;

    float terx, tery;
    int gridx, gridy;
    vpPoint mid;

    // load terrain
    if (noLoad == false)
    {
        if (binaryElvFile)
            fullResTerrain->loadElvBinary(terfile);
        else
            fullResTerrain->loadElv(terfile);

        fullResTerrain->calcMeanHeight();
        std::cout << "\n ****** Hi-res Terrain loaded...\n";
     }

    std::cout << " ****** Mean height: " << fullResTerrain->getHeightMean() << "\n";
    fullResTerrain->getTerrainDim(terx, tery);
    std::cout << " ****** terrain area : " << terx << " X " << tery << "\n";
    fullResTerrain->getGridDim(gridx, gridy);
    std::cout << " ****** grid samples: " << gridx << " x " << gridy << "\n";
    fullResTerrain->getMidPoint(mid);
    std::cout << " ****** midpt = (" << mid.x << ", " << mid.y << ", " << mid.z << ")\n";


    // define default region to be small aspect ratio preserving subregion of full terrain input
    float centrex = (gridx-1)/2.0f, centrey = (gridy-1)/2.0f;
    float widthx = (gridx-1)/float(factor), widthy = (gridy-1)/float(factor);
    Region defRegion;
    defRegion.x0 = int( centrex - (0.5*widthx) );
    defRegion.y0 = int( centrey - (0.5*widthy) );
    defRegion.x1 = int( centrex + (0.5*widthx) );
    defRegion.y1 = int( centrey + (0.5*widthy) );

    // PCM: test - full coverage of input terrain
    //defRegion.x0 = 0;
    //defRegion.y0 = 0;
    //defRegion.x1 = gridx-1;
    //defRegion.y1 = gridy-1;

    // create downsampled overview
    if (noLoad == false)
    {
        if (binaryElvFile)
            lowResTerrain->loadElvBinary(terfile, downFactor);
        else
            lowResTerrain->loadElv(terfile, downFactor);
        lowResTerrain->calcMeanHeight();
        std::cout << "\n ****** Lo-res Terrain loaded...\n";
    }

    selectedRegion = defRegion;

    std::cout << " ****** Mean height: " << lowResTerrain->getHeightMean() << "\n";
    lowResTerrain->getTerrainDim(terx, tery);
    std::cout << " ****** terrain area : " << terx << " X " << tery << "\n";
    lowResTerrain->getGridDim(gridx, gridy);
    std::cout << " ****** grid samples: " << gridx << " x " << gridy << "\n";
    lowResTerrain->getMidPoint(mid);
    std::cout << " ****** midpt = (" << mid.x << ", " << mid.y << ", " << mid.z << ")\n";


    // create texture/map overlay

    /*
    // match dimensions for empty overlay
    int dx, dy;
    getTerrain()->getGridDim(dx, dy);
    getTypeMap(TypeMapType::TRANSECT)->matchDim(dy, dx);
    getTypeMap(TypeMapType::TRANSECT)->fill(1);
    getTypeMap(TypeMapType::EMPTY)->matchDim(dy, dx);
    getTypeMap(TypeMapType::EMPTY)->clear();
    */

    // default region extract

    return extractTerrainSubwindow(defRegion);
}

//// Scene

Scene::Scene(string ddir, string base) : terrain( new Terrain())
{
    //terrain = new Terrain();
    terrain->initGrid(1024, 1024, 10000.0f, 10000.0f);
    eco = new EcoSystem();
    biome = new Biome();
    tline = new Timeline();

    datadir = ddir;
    basename = base;
    int dx, dy;
    terrain->getGridDim(dx, dy);

    // instantiate typemaps for all possible typemaps		(XXX: this could lead to memory issues for larger landscapes?)
    for (int t = 0; t < int(TypeMapType::TMTEND); t++)
        maps[t] = new TypeMap(dy, dx, (TypeMapType)t);
    terrain->getGridDim(dx, dy);
    maps[2]->setRegion(Region(0, 0, dy-1, dx-1));		// this is for the 'TypeMapType::CATEGORY' typemap? Any reason why this one is special?

    nfield = new NoiseField(dx,dy,5, 0);
    dmaps = new DataMaps();
    masterTerrain = nullptr;
}

Scene::~Scene()
{
    //delete terrain;

    // cycle through all typemaps, and if exists, delete and assign nullptr to indicate empty
    for (int t = 0; t < int(TypeMapType::TMTEND); t++)
    {
        if (maps[int(t)] != nullptr)
        {
            delete maps[int(t)];
            maps[int(t)] = nullptr;
        }
    }

    delete eco;
    delete biome;
    delete tline;
    delete nfield;
    delete dmaps;
}

std::string Scene::get_dirprefix()
{
    // std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();

    // std::cout << "Datadir after fixing: " << datadir << std::endl;

    int slash_idx = datadir.find_last_of("/");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = datadir + "/" + setname;
    return dirprefix;
}

bool Scene::readMonthlyMap(std::string filename, std::vector<basic_types::MapFloat *> &monthly)
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
        terrain->getGridDim(dx, dy);
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

bool Scene::writeMonthlyMap(std::string filename, std::vector<basic_types::MapFloat *> &monthly)
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

void Scene::reset_sampler(int maxpercell)
{
    float rw, rh;
    getMasterTerrain()->getTerrainDim(rw, rh);
    int gw, gh;
    cohortmaps->get_grid_dims(gw, gh);
    float tw, th;
    cohortmaps->get_cohort_dims(tw, th);

    if (sampler)
        sampler.reset();
    sampler = std::unique_ptr<cohortsampler>(new cohortsampler(tw, th, rw - 1.0f, rh - 1.0f, 1.0f, 1.0f, maxpercell + 5, 3));
    //sampler = std::unique_ptr<cohortsampler>(new cohortsampler(tw, th, rw - 1.0f, rh - 1.0f, 1.0f, 1.0f, 60, 3));
}

void Scene::loadScene(std::vector<int> timestepIDs, bool shareCohorts, std::shared_ptr<CohortMaps> cohorts)
{
    // std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();

    // std::cout << "Datadir after fixing: " << datadir << std::endl;

    int slash_idx = datadir.find_last_of(".");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = get_dirprefix();

    loadScene(dirprefix, timestepIDs, shareCohorts, cohorts);
}

void Scene::loadScene(std::string dirprefix, std::vector<int> timestepIDs, bool shareCohorts, std::shared_ptr<CohortMaps> cohorts)
{
    std::vector<std::string> timestep_files;
    bool checkfiles = true;

    using namespace data_importer;

    //std::string terfile = datadir+"/dem.elv";
    for(auto &ts: timestepIDs)
    {
        //timestep_files.push_back(datadir + "/ecoviz_" + std::to_string(ts) + ".pdb");
        string binfile = datadir + "/" + basename + std::to_string(ts) + ".pdbb";
        string txtfile = datadir + "/" + basename + std::to_string(ts) + ".pdb";
        string filename;
        if (ifstream(binfile).is_open()) //prefer binary version
            filename = binfile;
        else
            filename = txtfile;

        timestep_files.push_back(filename); // datadir + "/" + basename + std::to_string(ts) + ".pdbb");
    }

    // load terrain (already calld loadElv and calcMeanHeight)

    //terrain = std::move(newTerr);

    //getTerrain()->loadElv(terfile);
    //getTerrain()->calcMeanHeight();

    // match dimensions for empty overlay

    // continue setup for sub-terrain newTerr
    /*
    int dx, dy;
    getTerrain()->getGridDim(dx, dy);
    getTypeMap(TypeMapType::TRANSECT)->matchDim(dy, dx);
    getTypeMap(TypeMapType::TRANSECT)->fill(1);
    getTypeMap(TypeMapType::EMPTY)->matchDim(dy, dx);
    getTypeMap(TypeMapType::EMPTY)->clear();
    */

    // NB: must maintain original terrain dims/size when imposing a sub-region terrain
    // ****entire**** region plan/eco data read in - later we select out the relevant parts for
    // rendering
    Region srcRegion;
    float sx,ex, sy, ey, parentXdim, parentYdim;
    terrain->getSourceRegion(srcRegion,
                             sx, sy, ex, ey,
                             parentXdim, parentYdim);

    assert(parentXdim > 0.0);
    assert(parentYdim > 0.0);

    // getTerrain()->getTerrainDim(rw, rh);

    if (getBiome()->read_dataimporter(":/resources/databases/european.db"))
    {
        // loading plant distribution
        getEcoSys()->setBiome(getBiome());
    }

    auto species_lookup = getBiome()->getSpeciesIndexLookupMap();

    // check that pdb files exist
    for (auto &fname : timestep_files)
    {
        std::ifstream ifs(fname);

        if (!ifs.is_open())
        {
            cerr << "Could not open PDB file at " << fname << endl;
            checkfiles = false;
        }
        ifs.close();
    }

    if(checkfiles)
    {
        // import cohorts
        try {
            if (shareCohorts == false || (shareCohorts == true && !cohorts) )
                cohortmaps = std::shared_ptr<CohortMaps>(new CohortMaps(timestep_files, parentXdim, parentYdim, "3.0", species_lookup));
            else
                cohortmaps = cohorts;
        } catch (const std::exception &e) {
            cerr << "Exception in create cohort maps: " << e.what();
        }
        if (cohortmaps) {
            before_mod_map = cohortmaps->get_map(0);
            //cohortmaps->do_adjustments(2);

            if (cohortmaps->get_nmaps() > 0)
            {
                reset_sampler(cohortmaps->get_maxpercell());

                //std::vector<basic_tree> trees = sampler->sample(cohortmaps[0]);
                //data_importer::write_pdb("testsample.pdb", trees.data(), trees.data() + trees.size());
            }
        }

        // set timeline
        tline->setTimeBounds(1, (int) timestepIDs.size());
        tline->setNow(1);
        cerr << "TimeLine" << endl;
        cerr << "start = " << tline->getTimeStart() << " end = " << tline->getTimeEnd() << " delta = " << tline->getTimeStep() << " current = " << tline->getNow() << endl;
    }
}

void Scene::loadDataMaps(int timesteps)
{
    std::string idxfile, datafile;

    // std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();
    idxfile = datadir + "/" + basename + "_idx.asc";
    datafile = datadir + "/" + basename + "_map.csv";

    dmaps->loadDataMaps(idxfile, datafile, timesteps);
}

// PCM - this won't export properly with overview window since that loads entire ecosystem, and can't deal with sub-regions
// TBD
void Scene::exportSceneXml(map<string, vector<MitsubaModel>>& speciesMap, ofstream& xmlFile, Transect * transect) {
    set<string> plantCodeNotFound; // Store plant codes that were not found to display a warning message at the end

    PlantGrid* pg = this->getEcoSys()->getPlants();
    for (int x = 0; x < pg->gx; x++)
    {
        for (int y = 0; y < pg->gy; y++)
        {
            auto ppopulation = pg->getPopulation(x, y);
            for (int s = 0; s < (int)ppopulation->pop.size(); s++)
            {
                for (int p = 0; p < (int)ppopulation->pop[s].size(); p++)
                {
                    Plant plant = ppopulation->pop[s][p];

                    if (transect)
                    {
                        float xStart = transect->getBoundStart().x;
                        float zStart = transect->getBoundStart().z;

                        float xEnd = transect->getBoundEnd().x;
                        float zEnd = transect->getBoundEnd().z;

                        if (xStart < 0.001 && zStart < 0.001 && xEnd < 0.001 && zEnd < 0.001)
                        {
                            // Transect not defined
                            continue;
                        }

                        float distanceToTransect = abs((xEnd - xStart) * (zStart - plant.pos.z) - (xStart - plant.pos.x) * (zEnd - zStart));
                        distanceToTransect /= sqrt((xEnd - xStart) * (xEnd - xStart) + (zEnd - zStart) * (zEnd - zStart));

                        if (distanceToTransect > transect->getThickness() / 2)
                        {
                            continue;
                        }
                    }

                    string code = this->biome->getPFType(s)->code;

                    map<string, vector<MitsubaModel>>::iterator it;
                    if ((it = speciesMap.find(code)) != speciesMap.end())
                    {
                        int indexNearestHeightMax = 0;
                        vector<MitsubaModel>& vectMitsubaModel = it->second;
                        for (int i = 0; i < vectMitsubaModel.size(); i++)
                        {
                            if (vectMitsubaModel[i].maxHeight >= plant.height && vectMitsubaModel[i].maxHeight <= vectMitsubaModel[indexNearestHeightMax].maxHeight)
                            {
                                indexNearestHeightMax = i;
                            }
                        }

                        int xHash = plant.pos.x * 100;
                        int zHash = plant.pos.z * 100;
                        int rotate = hashTable[(int)(hashTable[(int)((xHash) & 0xfffL)] ^ ((zHash) & 0xfffL))] % 360;

                        xmlFile << "\t<shape type=\"instance\">\n";
                        xmlFile << "\t\t<ref id=\"" << vectMitsubaModel[indexNearestHeightMax].id << "\"/>\n";
                        xmlFile << "\t\t<transform name=\"toWorld\">\n";
                        // Rotation
                        xmlFile << "\t\t\t<rotate y=\"1\" angle=\"" << rotate << "\"/>\n";
                        // Translation
                        xmlFile << "\t\t\t<translate x=\"" << plant.pos.x << "\" y=\"" << plant.pos.y << "\" z=\"" << plant.pos.z << "\"/>\n";
                        // Scale
                        xmlFile << "\t\t\t<scale value=\"" << plant.height / vectMitsubaModel[indexNearestHeightMax].actualHeight << "\"/>\n";
                        xmlFile << "\t\t</transform>\n";
                        xmlFile << "\t</shape>\n\n";
                    }
                    else if (plantCodeNotFound.find(code) == plantCodeNotFound.end())
                    {
                        // Plant code was not found in the profile
                        plantCodeNotFound.insert(code);
                    }
                }
            }
        }
    }

    if (!plantCodeNotFound.empty())
    {
        QMessageBox messageBox;
        QString warningMessage = "The following plant codes were not found and have been skipped :";
        for (string code : plantCodeNotFound)
        {
            warningMessage += QString("\n - ") + code.data();
        }
        messageBox.warning(0, "Plant code not found", warningMessage);
    }
}


/*
 * Export the scene to JSON format
 * @param speciesMap: Map containing the species metadata
 * @param jsonFile: The JSON file to write to
 * @param scene: The scene to export
 * @param transect: The transect to export
 */
void Scene::exportInstancesJSON(map<string, vector<MitsubaModel>>& speciesMap, string urlInstances, string nameInstances, Scene* scene, Transect* transect) 
{

  QDir().mkpath(QString::fromStdString(urlInstances + nameInstances) + "/");

  ofstream jsonFile;
  jsonFile.open(urlInstances + nameInstances +".json");

  set<string> plantCodeNotFound; // Store plant codes that were not found to display a warning message at the end

  map<string, ofstream> streams;


  Region parentRegion;
  float parentX0, parentY0, parentX1, parentY1, parentDimx, parentDimy;

  bool parentRegionAvailable = scene->getTerrain()->getSourceRegion(parentRegion, parentX0, parentY0, parentX1, parentY1, parentDimx, parentDimy);

  PlantGrid* pg = this->getEcoSys()->getPlants();
  for (int x = 0; x < pg->gx; x++)
  {
    for (int y = 0; y < pg->gy; y++)
    {
			auto ppopulation = pg->getPopulation(x, y); // Get the population of the current cell
      for (int s = 0; s < (int)ppopulation->pop.size(); s++)
      {
        for (int p = 0; p < (int)ppopulation->pop[s].size(); p++)
        {
					Plant plant = ppopulation->pop[s][p]; // Get the current plant

					if (parentRegionAvailable)
					{
						if (plant.pos.x < parentY0 || plant.pos.x > parentY1 || plant.pos.z < parentX0 || plant.pos.z > parentX1)
						{
							continue;
						}
					}

          string code = this->biome->getSpeciesMetaData()[s].scientific_name;

          map<string, vector<MitsubaModel>>::iterator it;
					if ((it = speciesMap.find(code)) != speciesMap.end()) // If the plant code is found in the profile
          {
            int indexNearestHeightMax = 0;
            vector<MitsubaModel>& vectMitsubaModel = it->second;

            for (int i = 0; i < vectMitsubaModel.size(); i++)
            {
              if (vectMitsubaModel[i].maxHeight <= plant.height )
              {
                indexNearestHeightMax = i;
              }
            }

            int xHash = plant.pos.x * 100;
            int zHash = plant.pos.z * 100;
            int rotate = hashTable[(int)(hashTable[(int)((xHash) & 0xfffL)] ^ ((zHash) & 0xfffL))] % 360;
            double scale = plant.height / vectMitsubaModel[indexNearestHeightMax].actualHeight;

            // Current Instance
            string key = vectMitsubaModel[indexNearestHeightMax].id;
            if (streams.find(key) != streams.end())
						{
              streams[key] << ",\n";
						}
            else
            {
              // Create Stream
              streams.emplace(key, ofstream());

              // Init stream
              streams[key].open(urlInstances + "/" + nameInstances + "/" + nameInstances + "_" + key + ".json");
              streams[key] << "{\n";
              streams[key] << "\t\"ObjectsInstances\": [\n";
              streams[key] << "\t{\n";
              streams[key] << "\t\t\"Ref\": \"" << key << "\",\n";
              streams[key] << "\t\t\"Instances\": [\n";
            }

            ofstream& stream = streams[key];

            stream << "\t\t\t{";
            stream << "\"Rotate\": [ 0, " << std::to_string(rotate/10.) << ", 0 ],";// Rotation Y
            stream << "\"Translate\": [ " << std::to_string(plant.pos.x - parentY0) << ", " << std::to_string(plant.pos.y) << ", " << std::to_string(plant.pos.z - parentX0) << " ],";// Translation
            stream << "\"Scale\": [ "<< std::to_string(scale) <<", "<< std::to_string(scale) <<", "+ std::to_string(scale) <<" ]";// Scale
            stream << "}";

          }
          else if (plantCodeNotFound.find(code) == plantCodeNotFound.end())
          {
            // Plant code was not found in the profile
            plantCodeNotFound.insert(code);
          }
        }
      }
    }
  }
  
  if (!streams.empty())
	{
    jsonFile << "{\n";
    jsonFile << "\t\"Import\": [\n";
	}

  bool first = true;
  for (auto const &ent : streams)
  {

    if(!first)
			jsonFile << ",\n";
    jsonFile << "\t\t\"./"<<nameInstances<<"/"<< nameInstances <<"_" << ent.first << ".json\"";

    ofstream& stream = streams[ent.first];
    stream << "\n\t\t]\n";
    stream << "\t}]\n";
    stream << "}\n";
    stream.close();
    first = false;
  }

  if (!streams.empty())
  {
    jsonFile << "\n\t]\n";
    jsonFile << "}\n";
  }


  if (!plantCodeNotFound.empty())
  {
    QMessageBox messageBox;
    QString warningMessage = "The following plant codes were not found and have been skipped :";
    for (string code : plantCodeNotFound)
    {
      warningMessage += QString("\n - ") + code.data();
    }
    messageBox.warning(0, "Plant code not found", warningMessage);
  }
}


/*
 * Export the terrain
 * @param jsonFile: The JSON file to write to
 * @param transect: The transect to export
*/
void Scene::exportSceneJSON(const string jsonDirPath, const string cameraName, const string lightsName,
  const string terrainName, const string instancesName, const string sceneName, const int resX, 
  const int resY, const int quality, const int threads)
{

	ofstream jsonFile;
  jsonFile.open(jsonDirPath + "/"+ sceneName +".json");

	// Write JSON
  jsonFile << "{\n";

  jsonFile << "\t\"Import\":\n";
  jsonFile << "\t[\n";
  jsonFile << "\t\t\"ModelSpecies/Species.json\",\n";
  jsonFile << "\t\t\"Cameras/" << cameraName <<".json\",\n";
  jsonFile << "\t\t\"EnvMaps/" << lightsName << ".json\",\n";
  jsonFile << "\t\t\"Terrain/" << terrainName << ".json\",\n";
  jsonFile << "\t\t\"Instances/" << instancesName << ".json\"\n";
  jsonFile << "\t],\n";
  jsonFile << "\t\"Scene\":\n"; 
  jsonFile << "\t{\n";
  jsonFile << "\t\t\"Name\": \""<< sceneName <<"\",\n";
  jsonFile << "\t\t\"Resolution\": [" << resX << ","<<resY << "],\n";
  jsonFile << "\t\t\"Quality\": " << quality << ",\n";
  jsonFile << "\t\t\"M3Backend\": \"scalar_rgb\",\n";
  jsonFile << "\t\t\"Threads\": " << threads << ",\n";
  jsonFile << "\t\t\"Frames\": [0, 0],\n";
  jsonFile << "\t\t\"OutputDir\": \".\"\n";
  jsonFile << "\t}\n";

  jsonFile << "}\n";

	jsonFile.close();

}

/*
 * Export the terrain 
 * @param jsonFile: The JSON file to write to
 * @param transect: The transect to export 
*/
void Scene::exportTerrainJSON(const string terrainURL, const string terrainName, Transect* transect)
{

  QDir().mkdir(QString::fromStdString(terrainURL) + "/OBJ");
  QDir().mkdir(QString::fromStdString(terrainURL) + "/Masks");

  // Export OBJ
  Terrain* terrain = getTerrain();
  terrain->saveOBJ(terrainURL + "OBJ/" + terrainName + ".obj");
  terrain->saveOBJ_Border(terrainURL + "OBJ/" + terrainName+"_Border.obj");

  // Create terrain texture
  // - Save Masks according to a specific slope
  exportTextureSlope(terrainURL + "Masks/"+ terrainName + "maskSlopeBedrock.png", 50.,55.);
  exportTextureSlope(terrainURL + "Masks/" + terrainName + "maskSlopeGround.png", 60., 78.);

  // Export JSON
  ofstream jsonFile;
  jsonFile.open(terrainURL + "/" + terrainName + ".json");

  jsonFile << "{\n";

  // Terrain Object
  jsonFile << "\t\"Objects\":\n";
  jsonFile << "\t[{\n";
  jsonFile << "\t\t\"Name\": \"terrain\",\n";
  jsonFile << "\t\t\"Definition\" : \n";
  jsonFile << "\t\t[{\n";
  jsonFile << "\t\t\t\"Name\": \"Terrain\",\n";
  jsonFile << "\t\t\t\"File\" : \"OBJ/"<< terrainName <<".obj\",\n";
  jsonFile << "\t\t\t\"Material\" : {\n";
  jsonFile << "\t\t\t\t\"Type\":   \"Blended\",\n";
  jsonFile << "\t\t\t\t\"Layers\" : [\n";
  jsonFile << "\t\t\t\t\t{\n";
  jsonFile << "\t\t\t\t\t\t\"ColorMap\":   \"Textures/Grass_BaseColor.png\",\n";
  jsonFile << "\t\t\t\t\t\t\"NormalMap\" : \"Textures/coast_sand_rocks_02_nor_gl_2k.exr\",\n";
  jsonFile << "\t\t\t\t\t\t\"UVScale\" : 100.0\n"; //,\n";
  //jsonFile << "\t\t\t\t\t\t\"OpacityMap\" : \"Terrain/Textures/Grass_Opacity.png\",\n";
  //jsonFile << "\t\t\t\t\t\t\"NormalMap\" : \"Terrain/Textures/Grass_Normal.png\",\n";
  //jsonFile << "\t\t\t\t\t\t\"BumpMap\" : \"Terrain/Textures/Grass_Height.png\",\n";
  //jsonFile << "\t\t\t\t\t\t\"BumpMapIntensity\" : 1.0\n";
  jsonFile << "\t\t\t\t\t},\n";
  jsonFile << "\t\t\t\t\t{\n";
  jsonFile << "\t\t\t\t\t\t\"AlphaMap\" : \"Masks/" + terrainName + "maskSlopeGround.png\",\n";
  jsonFile << "\t\t\t\t\t\t\"ColorMap\":   \"Textures/Grass_Dry_BaseColor.jpg\",\n";
  jsonFile << "\t\t\t\t\t\t\"NormalMap\" : \"Textures/aerial_rocks_04_nor_gl_2k.exr\",\n";
  jsonFile << "\t\t\t\t\t\t\"UVScale\" : 100.0\n";
  jsonFile << "\t\t\t\t\t},\n";
  jsonFile << "\t\t\t\t\t{\n";
  jsonFile << "\t\t\t\t\t\t\"AlphaMap\" : \"Masks/" + terrainName + "maskSlopeBedrock.png\",\n";
  jsonFile << "\t\t\t\t\t\t\"ColorMap\":   \"Textures/T_rock02_DS.tga\",\n";
  jsonFile << "\t\t\t\t\t\t\"NormalMap\" : \"Textures/T_rock01_N.tga\",\n";
  //jsonFile << "\t\t\t\t\t\t\"BumpMap\" : \"Terrain/Textures/Bedrock_Stone_B.png\",\n";
  //jsonFile << "\t\t\t\t\t\t\"BumpMapIntensity\" : 1.0,\n";
  jsonFile << "\t\t\t\t\t\t\"UVScale\" : 30.0\n";
  jsonFile << "\t\t\t\t\t}\n";
  jsonFile << "\t\t\t\t]\n";
  jsonFile << "\t\t\t},\n";
  jsonFile << "\t\t\t\"Instances\": \n";
  jsonFile << "\t\t\t[{\n";
  jsonFile << "\t\t\t\t\"Rotate\": [0, 0, 0] ,\n";
  jsonFile << "\t\t\t\t\"Scale\" : [1, 1, 1] ,\n";
  jsonFile << "\t\t\t\t\"Translate\" : [0, 0, 0]\n";
  jsonFile << "\t\t\t}]\n";
  jsonFile << "\t\t}]\n";
  jsonFile << "\t},\n";

  jsonFile << "\t{\n";
  jsonFile << "\t\t\"Name\": \"terrainBorder\",\n";
  jsonFile << "\t\t\"Definition\" : \n";
  jsonFile << "\t\t[{\n";
  jsonFile << "\t\t\t\"Name\": \"Terrain\",\n";
  jsonFile << "\t\t\t\"File\" : \"OBJ/"<< terrainName <<"_Border.obj\",\n";
  jsonFile << "\t\t\t\"Material\" : {\n";
  jsonFile << "\t\t\t\t\"Type\":   \"DiffuseColor\",\n";
  jsonFile << "\t\t\t\t\"Color\" : [0.8, 0.8, 0.8]\n";
  jsonFile << "\t\t\t},\n";
  jsonFile << "\t\t\t\"Instances\": \n";
  jsonFile << "\t\t\t[{\n";
  jsonFile << "\t\t\t\t\"Rotate\": [0, 0, 0] ,\n";
  jsonFile << "\t\t\t\t\"Scale\" : [1, 1, 1] ,\n";
  jsonFile << "\t\t\t\t\"Translate\" : [0, 0, 0]\n";
  jsonFile << "\t\t\t}]\n";
  jsonFile << "\t\t}]\n";
  jsonFile << "\t}],\n";


  // Onjects Instances
  jsonFile << "\t\"ObjectsInstances\": \n";
  jsonFile << "\t[{\n";
  jsonFile << "\t\t\"Ref\": \"terrain\", \n";
  jsonFile << "\t\t\"Instances\" :\n";
  jsonFile << "\t\t[{\n";
  jsonFile << "\t\t\t\"Rotate\": [0, 0, 0] ,\n";
  jsonFile << "\t\t\t\"Scale\" : [1, 1, 1] ,\n";
  jsonFile << "\t\t\t\"Translate\" : [0, 0, 0]\n";
  jsonFile << "\t\t}]\n";
  jsonFile << "\t},\n";
  jsonFile << "\t{\n";
  jsonFile << "\t\t\"Ref\": \"terrainBorder\", \n";
  jsonFile << "\t\t\"Instances\" :\n";
  jsonFile << "\t\t[{\n";
  jsonFile << "\t\t\t\"Rotate\": [0, 0, 0] ,\n";
  jsonFile << "\t\t\t\"Scale\" : [1, 1, 1] ,\n";
  jsonFile << "\t\t\t\"Translate\" : [0, 0, 0]\n";
  jsonFile << "\t\t}]\n";
  jsonFile << "\t}]\n";

  // End JSON
  jsonFile << "}\n";
}


void Scene::exportTextureSlope(string URL, float slopeMin, float slopeMax)
{
  // - Extract terrain slope map
  int x, y;
  terrain->getGridDim(x, y);
  basic_types::MapFloat* slopeMap = new basic_types::MapFloat();
  slopeMap->setDim(x, y);
  slopeMap->fill(0.0f);

  terrain->calcSlopeMap(slopeMap);

  QImage slopeImage(x, y, QImage::Format_RGB32);
  for (int i = 0; i < x; i++)
  {
    for (int j = 0; j < y; j++)
    {
      float slope = slopeMap->get(i, j);
      // clamp slope to 0-90
      slope = std::max(slopeMin, slope);
      slope = std::min(slopeMax, slope);
      slopeImage.setPixel(i, (y-1)-j, qRgb(255 - 255 * (slope- slopeMin) / (slopeMax-slopeMin), 255 - 255 * (slope - slopeMin) / (slopeMax - slopeMin), 255 - 255 * (slope - slopeMin) / (slopeMax - slopeMin)));
    }
  }

  slopeImage.save(QString(URL.data()) );
}
