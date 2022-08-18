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

//// Transect

bool Transect::inBounds(vpPoint pnt, Terrain * ter)
{
    Plane b[4];
    bool inside = true;

    // determine bounding plane intersects
    float maxx, maxy;
    ter->getTerrainDim(maxx, maxy);

    b[0].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)); // left edge
    b[1].formPlane(vpPoint(maxx, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f)); // right edge
    b[2].formPlane(vpPoint(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f)); // front edge
    b[3].formPlane(vpPoint(0.0f, 0.0f, maxy), Vector(0.0f, 0.0f, -1.0f)); // back edge

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
    ter->getTerrainDim(maxx, maxy);

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
    for(int x = 0; x < dx; x++)
        for(int y = 0; y < dy; y++)
        {
            // position on terrain corresponding to grid point, projected onto the base plane
            vizpnt = ter->toWorld(y, x, 0.0f);
            if(!offset[0].side(vizpnt) && !offset[1].side(vizpnt)) // between planes so draw in red
                mapviz->set(x, y, 1.0f);
            else
                mapviz->set(x, y, 0.0f);
        }
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
    title = "";
}

TimelineGraph::TimelineGraph(Timeline * tline, int nseries, std::string name)
{
    title = name;
    vscale = 0;
    numseries = nseries;
    setTimeLine(tline);
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
    int vmax = 0;
    int nspecies = s->getBiome()->numPFTypes();
    setNumSeries(nspecies);

    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {
        float tot = 0.0f;

        std::vector<basic_tree> trees(s->sampler->sample(s->cohortmaps->get_map(t), nullptr));
        std::vector<basic_tree> mature = s->cohortmaps->get_maturetrees(t);
        for(auto &tree: mature)
        {
            if(s->getTerrain()->inGridBounds(tree.x, tree.y))
               trees.push_back(tree);
        }
        for(int spc = 0; spc < nspecies; spc++) // iterate over species
        {
            float dbhtot = 0.0f;
            for(auto tree: trees)  // count species
                if(tree.species == spc)
                    dbhtot += tree.dbh;
            assignData(spc, t, dbhtot);
            tot += dbhtot;
        }
        if(tot > vmax)
            vmax = tot;
    }

    setVertScale(vmax);
}

void TimelineGraph::extractNormalizedBasalArea(Scene *s)
{
    float vmax = 0.0f;
    int nspecies = s->getBiome()->numPFTypes();
    float hectares = s->getTerrain()->getTerrainHectArea();
    int spccnt;

    setNumSeries(nspecies);

    for(int t = 0; t < timeline->getNumIdx(); t++) // iterate over timesteps
    {
        float tot = 0.0f;

        std::vector<basic_tree> trees(s->sampler->sample(s->cohortmaps->get_map(t), nullptr));
        std::vector<basic_tree> mature = s->cohortmaps->get_maturetrees(t);
        for(auto &tree: mature)
        {
            if(s->getTerrain()->inGridBounds(tree.x, tree.y))
               trees.push_back(tree);
        }
        for(int spc = 0; spc < nspecies; spc++) // iterate over species
        {
            float basaltot = 0.0f;
            spccnt = 0;
            for(auto tree: trees)  // count species
                if(tree.species == spc)
                {
                    basaltot += (PI * tree.dbh*tree.dbh/ 4. / 10000.); // dbh is in cm, need to convert to m.
                    spccnt++;
                }
            basaltot /= hectares;
            assignData(spc, t, basaltot);
            tot += basaltot;
        }
        if(tot > vmax)
            vmax = tot;
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
            if(s->getTerrain()->inGridBounds(tree.x, tree.y))
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

//// Scene

Scene::Scene(string ddir)
{
    terrain = new Terrain();
    terrain->initGrid(1024, 1024, 10000.0f, 10000.0f);
    eco = new EcoSystem();
    biome = new Biome();
    tline = new Timeline();

    datadir = ddir;
    int dx, dy;
    terrain->getGridDim(dx, dy);

    // instantiate typemaps for all possible typemaps		(XXX: this could lead to memory issues for larger landscapes?)
    for (int t = 0; t < int(TypeMapType::TMTEND); t++)
        maps[t] = new TypeMap(dx, dy, (TypeMapType)t);
    terrain->getGridDim(dx, dy);
    maps[2]->setRegion(Region(0, 0, dx-1, dy-1));		// this is for the 'TypeMapType::CATEGORY' typemap? Any reason why this one is special?

    nfield = new NoiseField(terrain, 5, 0);

    for(int m = 0; m < 12; m++)
    {
        moisture.push_back(new basic_types::MapFloat());
        sunlight.push_back(new basic_types::MapFloat());
        temperature.push_back(0.0f);
    }
    slope = new basic_types::MapFloat();
    chm = new basic_types::MapFloat();
    cdm = new basic_types::MapFloat();
}

Scene::~Scene()
{
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

    delete eco;
    delete biome;
    delete tline;
    delete nfield;
    for(int m = 0; m < 12; m++)
    {
        delete sunlight[static_cast<int>(m)];
        delete moisture[static_cast<int>(m)];
    }
    temperature.clear();
    delete chm;
    delete cdm;
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

bool Scene::readSun(std::string filename)
{
    return readMonthlyMap(filename, sunlight);
}

bool Scene::writeSun(std::string filename)
{
    return writeMonthlyMap(filename, sunlight);
}

bool Scene::readMoisture(std::string filename)
{
    return readMonthlyMap(filename, moisture);
}

bool Scene::writeMoisture(std::string filename)
{
    return writeMonthlyMap(filename, moisture);
}

void Scene::calcSlope()
{
    int dx, dy;
    Vector up, n;

    // slope is dot product of terrain normal and up vector
    up = Vector(0.0f, 1.0f, 0.0f);
    terrain->getGridDim(dx, dy);
    slope->setDim(dx, dy);
    slope->fill(0.0f);
    for(int x = 0; x < dx; x++)
        for(int y = 0; y < dy; y++)
        {
            terrain->getNormal(x, y, n);
            float rad = acos(up.dot(n));
            float deg = RAD2DEG * rad;
            slope->set(y, x, deg);
        }
}

void Scene::reset_sampler(int maxpercell)
{
    float rw, rh;
    getTerrain()->getTerrainDim(rw, rh);
    int gw, gh;
    cohortmaps->get_grid_dims(gw, gh);
    float tw, th;
    cohortmaps->get_cohort_dims(tw, th);

    if (sampler)
        sampler.reset();
    sampler = std::unique_ptr<cohortsampler>(new cohortsampler(tw, th, rw - 1.0f, rh - 1.0f, 1.0f, 1.0f, maxpercell + 5, 3));
    //sampler = std::unique_ptr<cohortsampler>(new cohortsampler(tw, th, rw - 1.0f, rh - 1.0f, 1.0f, 1.0f, 60, 3));
}

void Scene::loadScene(int timestep_start, int timestep_end)
{
    // std::cout << "Datadir before fixing: " << datadir << std::endl;
    while (datadir.back() == '/')
        datadir.pop_back();

    // std::cout << "Datadir after fixing: " << datadir << std::endl;

    int slash_idx = datadir.find_last_of("/");
    std::string setname = datadir.substr(slash_idx + 1);
    std::string dirprefix = get_dirprefix();

    loadScene(dirprefix, timestep_start, timestep_end);
}

void Scene::loadScene(std::string dirprefix, int timestep_start, int timestep_end)
{
    bool checkfiles = true;

    using namespace data_importer;

    std::vector<std::string> timestep_files;
    std::string terfile = datadir+"/dem.elv";

    for (int ts = timestep_start; ts <= timestep_end; ts++)
    {
        timestep_files.push_back(datadir + "/ecoviz_" + std::to_string(ts) + ".pdb");
    }

    // load terrain
    getTerrain()->loadElv(terfile);
    getTerrain()->calcMeanHeight();

    // match dimensions for empty overlay
    int dx, dy;
    getTerrain()->getGridDim(dx, dy);
    getTypeMap(TypeMapType::TRANSECT)->matchDim(dx, dy);
    getTypeMap(TypeMapType::TRANSECT)->fill(1);
    getTypeMap(TypeMapType::EMPTY)->matchDim(dx, dy);
    getTypeMap(TypeMapType::EMPTY)->clear();

    float rw, rh;
    getTerrain()->getTerrainDim(rw, rh);

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
        cohortmaps = std::unique_ptr<CohortMaps>(new CohortMaps(timestep_files, rw, rh, "2.0"));
        before_mod_map = cohortmaps->get_map(0);
        //cohortmaps->do_adjustments(2);

        cerr << "after pdb load" << endl;

        if (cohortmaps->get_nmaps() > 0)
        {
            reset_sampler(cohortmaps->get_maxpercell());

            //std::vector<basic_tree> trees = sampler->sample(cohortmaps[0]);
            //data_importer::write_pdb("testsample.pdb", trees.data(), trees.data() + trees.size());
        }

        // set timeline
        tline->setTimeBounds(timestep_start, timestep_end);
        tline->setNow(timestep_start);
        cerr << "TimeLine" << endl;
        cerr << "start = " << tline->getTimeStart() << " end = " << tline->getTimeEnd() << " delta = " << tline->getTimeStep() << " current = " << tline->getNow() << endl;
    }

    if (getBiome()->read_dataimporter(SONOMA_DB_FILEPATH))
    {
        // loading plant distribution
        getEcoSys()->setBiome(getBiome());
    }
}

void Scene::saveScene(std::string dirprefix)
{
    std::string terfile = dirprefix+".elv";
    std::string pdbfile = dirprefix+".pdb";

    // save terrain
    getTerrain()->saveElv(terfile);

    if(!getEcoSys()->saveNichePDB(pdbfile))
        cerr << "Error Scene::saveScene: saving plane file " << pdbfile << " failed" << endl;
}

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
                        // Scale
                        xmlFile << "\t\t\t<scale value=\"" << plant.height / vectMitsubaModel[indexNearestHeightMax].actualHeight << "\"/>\n";
                        // Rotation
                        xmlFile << "\t\t\t<rotate y=\"1\" angle=\"" << rotate << "\"/>\n";
                        // Translation
                        xmlFile << "\t\t\t<translate x=\"" << plant.pos.x << "\" y=\"" << plant.pos.y << "\" z=\"" << plant.pos.z << "\"/>\n";
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
