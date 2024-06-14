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


// eco.cpp: core classes for controlling ecosystems and plant placement
// author: James Gain
// date: 27 February 2016

#include "eco.h"
// #include "interp.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <QDir>
#include <QElapsedTimer>

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

/// NoiseField

NoiseField::NoiseField(int gridX, int gridY, int dstep, long sval)
{
    //int tx, ty;

    // terrain = ter;
    //terrain->getGridDim(tx, ty);
    dimx = gridX * dstep;
    dimy = gridY * dstep;
    nmap = new basic_types::MapFloat();
    nmap->setDim(dimx, dimy);
    nmap->fill(0.0f);
    seed = sval;
    dice = new DiceRoller(0, 1000, sval);
    init();
}

void NoiseField::init()
{
    for(int x = 0; x < dimx; x++)
        for(int y = 0; y < dimy; y++)
        {
            nmap->set(x, y, (float) dice->generate() / 1000.0f);
        }
}

float NoiseField::getNoise(vpPoint p, float tx, float ty)
{
    float convx, convy;
    int x, y;
    // NOTE: this assumes p is in [0,tx] x [0,ty]
    // terrain pointer removed since it is hard to manage with sub-region terrains
    //terrain->getTerrainDim(tx, ty);
    convx = (float) (dimx-1) / tx;
    convy = (float) (dimy-1) / ty;
    x = (int) (p.x * convx);
    y = (int) (p.z * convy);

   //assert((x >=0) && (x < dimx));
   //assert((y >=0) && (y < dimy));

    return nmap->get(x, y);
}

/// PlantGrid

void PlantGrid::initSpeciesTable()
{
    speciesTable.clear();
}

void PlantGrid::delGrid()
{
    int i, j;

    // clear out elements of the vector hierarchy
    for(i = 0; i < (int) pgrid.size(); i++)
        for(j = 0; j < (int) pgrid[i].pop.size(); j++)
            pgrid[i].pop[j].clear();
    for(i = 0; i < (int) pgrid.size(); i++)
        pgrid[i].pop.clear();
    pgrid.clear();
}

void PlantGrid::initGrid()
{
    int i, j, s;

    delGrid();

    // setup empty elements of the vector hierarchy according to the grid dimensions
    for(i = 0; i < gx; i++)
        for(j = 0; j < gy; j++)
        {
            PlantPopulation ppop;
            for(s = 0; s < maxSpecies; s++)
            {
                std::vector<Plant> plnts;
                ppop.pop.push_back(plnts);
            }
            pgrid.push_back(ppop);
        }
}

bool PlantGrid::isEmpty()
{
    bool empty = true;
    int i, j, s, pos = 0;

    // setup empty elements of the vector hierarchy according to the grid dimensions
    for(i = 0; i < gx; i++)
        for(j = 0; j < gy; j++)
        {
            for(s = 0; s < maxSpecies; s++)
            {
                if(!pgrid[pos].pop[s].empty())
                    empty = false;
            }
            pos++;
        }
    return empty;
}

void PlantGrid::cellLocate(Terrain * ter, int mx, int my, int &cx, int &cy)
{
    int tx, ty;

    ter->getGridDim(tx, ty);

    // find grid bin for plant
    cx = (int) (((float) mx / (float) tx) * (float) gx);
    cy = (int) (((float) my / (float) ty) * (float) gy);
    if(cx >= gx)
        cx = gx-1;
    if(cy >= gy)
        cy = gy-1;
}


void PlantGrid::clearCell(int x, int y)
{
    int f = flatten(x, y);

    for(int s = 0; s < (int) pgrid[f].pop.size(); s++)
        pgrid[f].pop[s].clear();
}

void PlantGrid::placePlant(Terrain * ter, int species, Plant plant)
{
    int x, y, cx, cy;

    // find plant location on map
    ter->toGrid(plant.pos, x, y);
    cellLocate(ter, x, y, cx, cy);
    // cerr << "loc in " << cx << ", " << cy << " species " << species << endl;

    // add plant to relevant population
    pgrid[flatten(cx, cy)].pop[species].push_back(plant);
}



void PlantGrid::placePlantExactly(Terrain * ter, int species, Plant plant, int x, int y)
{
     pgrid[flatten(x, y)].pop[species].push_back(plant);
}

void PlantGrid::clearRegion(Terrain * ter, Region region)
{
    int x, y, sx, sy, ex, ey;

    getRegionIndices(ter, region, sx, sy, ex, ey);

    for(x = sx; x <= ex; x++)
        for(y = sy; y <= ey; y++)
            clearCell(x, y); // clear a specific cell in the grid
}

void PlantGrid::pickPlants(Terrain * ter, TypeMap * clusters, int niche, PlantGrid & outgrid)
{
    int x, y, s, p, mx, my, sx, sy, ex, ey, f;
    Plant plnt;
    Region region = clusters->getRegion();

    // map region to cells in the grid
    getRegionIndices(ter, region, sx, sy, ex, ey);

    for(x = sx; x <= ex; x++)
        for(y = sy; y <= ey; y++)
        {
            f = flatten(x, y);
            for(s = 0; s < (int) pgrid[f].pop.size(); s++)
                for(p = 0; p < (int) pgrid[f].pop[s].size(); p++)
                {
                    plnt = pgrid[f].pop[s][p];

                    ter->toGrid(plnt.pos, mx, my); // map plant terrain location to cluster map
                    if(clusters->getMap()->get(my,mx) == niche) // niche value on terrain matches the current plant distribution
                        outgrid.placePlantExactly(ter, s, plnt, x, y);
                }
        }
}

void PlantGrid::pickAllPlants(Terrain * ter, float offx, float offy, float scf, PlantGrid & outgrid)
{
    int x, y, s, p, f;
    Plant plnt;

    for(x = 0; x < gx; x++)
        for(y = 0; y < gy; y++)
        {
            f = flatten(x, y);
            for(s = 0; s < (int) pgrid[f].pop.size(); s++)
            {
                //if((int) pgrid[f].pop[s].size() > 0)
                //    cerr << "Species " << s << " Found" << endl;
                for(p = 0; p < (int) pgrid[f].pop[s].size(); p++)
                {
                    plnt = pgrid[f].pop[s][p];
                    plnt.pos.x *= scf; plnt.pos.z *= scf;
                    plnt.height *= scf; plnt.canopy *= scf;
                    plnt.pos.x += offy; plnt.pos.z += offx; // allows more natural layout
                    outgrid.placePlant(ter, s, plnt);
                }
            }
        }
}

void PlantGrid::vectoriseByPFT(int pft, std::vector<Plant> &pftPlnts)
{
    int x, y, p, s, f;
    Plant plnt;

    s = (int) pft;
    pftPlnts.clear();
    for(x = 0; x < gx; x++)
        for(y = 0; y < gy; y++)
        {
            f = flatten(x, y);
            if(s < 0  || s > (int) pgrid[f].pop.size())
                cerr << "PlantGrid::vectoriseBySpecies: mismatch between requested species and available species" << endl;

            for(p = 0; p < (int) pgrid[f].pop[s].size(); p++)
            {
                plnt = pgrid[f].pop[s][p];
                pftPlnts.push_back(plnt);
            }
        }
}

void PlantGrid::reportNumPlants()
{
    int i, j, s, plntcnt, speccnt;

    cerr << "grid dimensions = " << gx << " X " << gy << endl;
    for(i = 0; i < gx; i++)
        for(j = 0; j < gy; j++)
        {
            plntcnt = 0;
            for(s = 0; s < maxSpecies; s++)
            {
                speccnt = (int) pgrid[flatten(i,j)].pop[s].size();
                plntcnt += speccnt;
            }
            cerr << "count " << i << ", " << j << " = " << plntcnt << endl;
        }
}

void PlantGrid::setPopulation(int x, int y, PlantPopulation & pop)
{
    pgrid[flatten(x, y)] = pop;
}

PlantPopulation * PlantGrid::getPopulation(int x, int y)
{
    return & pgrid[flatten(x, y)];
}

void PlantGrid::getRegionIndices(Terrain * ter, Region region, int &sx, int &sy, int &ex, int &ey)
{
    cellLocate(ter, region.x0, region.y0, sx, sy);
    cellLocate(ter, region.x1, region.y1, ex, ey);

    // cerr << "map = " << region.x0 << ", " << region.y0 << " -> " << region.x1 << ", " << region.y1 << endl;
    // cerr << "grid = " << sx << ", " << sy << " -> " << ex << ", " << ey << endl;
}

/// PlantShape

void ShapeGrid::genSpherePlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - sphere
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, trunkheight+canopyheight/2.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::scale(tfm, glm::vec3(1.0, canopyheight, 1.0f)); // make sure tree fills 1.0f on a side bounding box
    tfm = glm::rotate(tfm, glm::radians(-90.0f), rotx);

#ifdef HIGHRES
    shape.genSphere(0.5f, 20, 20, tfm);
#endif
#ifdef LOWRES
    shape.genSphere(0.5f, 6, 6, tfm);
#endif
}

void ShapeGrid::genBoxPlant(float trunkheight, float trunkradius, float taper, float scale, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - tapered box
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, trunkheight, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(-90.0f), rotx);
    shape.genPyramid(1.0f*scale, taper*scale, canopyheight*scale, tfm);
}

void ShapeGrid::genConePlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - cone
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, trunkheight, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(-90.0f), rotx);
#ifdef HIGHRES
    shape.genCappedCone(0.5f, canopyheight, 20, 1, tfm, false);
#endif
#ifdef LOWRES
    shape.genCappedCone(0.5f, canopyheight, 6, 1, tfm, false);
#endif

}

void ShapeGrid::genInvConePlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - cone
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, 1.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(-270.0f), rotx);
    //tfm = glm::translate(tfm, glm::vec3(0.0f, 0.0f, -canopyheight));
#ifdef HIGHRES
    shape.genCappedCone(0.5f, canopyheight, 20, 1, tfm, false);
#endif
#ifdef LOWRES
    shape.genCappedCone(0.5f, canopyheight, 6, 1, tfm, false);
#endif

}

void ShapeGrid::genUmbrellaPlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - cone
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, 1.0f, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(90.0f), rotx);
#ifdef HIGHRES
    shape.genCappedCone(0.5f, canopyheight, 20, 1, tfm, false);
#endif
#ifdef LOWRES
    shape.genCappedCone(0.5f, canopyheight, 6, 1, tfm, false);
#endif

}

void ShapeGrid::genHemispherePlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - sphere
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, trunkheight, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::scale(tfm, glm::vec3(1.0, canopyheight*2.0f, 1.0f)); // make sure tree fills 1.0f on a side bounding box
    tfm = glm::rotate(tfm, glm::radians(90.0f), rotx);

#ifdef HIGHRES
    shape.genHemisphere(0.5f, 20, 20, tfm);
#endif
#ifdef LOWRES
    shape.genHemisphere(0.5f, 6, 6, tfm);
#endif

}

void ShapeGrid::genCylinderPlant(float trunkheight, float trunkradius, Shape &shape)
{
    glm::mat4 idt, tfm;
    glm::vec3 trs, rotx;
    float canopyheight;

    rotx = glm::vec3(1.0f, 0.0f, 0.0f);
    canopyheight = 1.0f - trunkheight;

    // trunk - cylinder
    idt = glm::mat4(1.0f);
    tfm = glm::rotate(idt, glm::radians(-90.0f), rotx);
    // extra 0.1 to trunk is to ensure that trunk joins tree properly
    shape.genCappedCylinder(trunkradius, trunkradius, trunkheight+0.1f, 3, 1, tfm, false);

    // canopy - cylinder
    idt = glm::mat4(1.0f);
    trs = glm::vec3(0.0f, trunkheight, 0.0f);
    tfm = glm::translate(idt, trs);
    tfm = glm::rotate(tfm, glm::radians(-90.0f), rotx);

#ifdef HIGHRES
    shape.genCappedCylinder(0.5f, 0.5f, canopyheight, 20, 1, tfm, false);
#endif
#ifdef LOWRES
   shape.genCappedCylinder(0.5f, 0.5f, canopyheight, 6, 1, tfm, false);
#endif
}

void ShapeGrid::delGrid()
{
    int i, j;

    // clear out elements of the vector hierarchy
    for(i = 0; i < (int) shapes.size(); i++)
        for(j = 0; j < (int) shapes[i].size(); j++)
            shapes[i][j].clear();
    for(i = 0; i < (int) shapes.size(); i++)
        shapes[i].clear();
    shapes.clear();
}

void ShapeGrid::initGrid(bool buildGeom)
{
    int i, j, s;

    // setup empty elements of the vector hierarchy according to the grid dimensions
    for(i = 0; i < gx; i++)
        for(j = 0; j < gy; j++)
        {
            std::vector<Shape> shapevec;
            for(s = 0; s < maxSpecies; s++)
            {
                Shape shape;
                shapevec.push_back(shape);
            }
            shapes.push_back(shapevec);
        }
    if (buildGeom) genPlants();
}

void ShapeGrid:: genPlants()
{
    float trunkheight, trunkradius;
    int s;

    for(s = 0; s < biome->numPFTypes(); s++)
    {
        Shape currshape;
        PFType * pft = biome->getPFType(s);
        currshape.setColour(pft->basecol);
        trunkheight = pft->draw_hght; trunkradius = pft->draw_radius;
        //genSpherePlant(trunkheight, trunkradius, currshape);		// XXX: just for debugging. Remove later and uncomment below switch statement
        switch(pft->shapetype)
        {
        case TreeShapeType::SPHR:
            genSpherePlant(trunkheight, trunkradius, currshape);
            break;
        case TreeShapeType::BOX:
            genBoxPlant(trunkheight, trunkradius, pft->draw_box1, pft->draw_box2, currshape);
            break;
        case TreeShapeType::CONE:
            genConePlant(trunkheight, trunkradius, currshape);
            break;
        case TreeShapeType::INVCONE:
            genInvConePlant(trunkheight, trunkradius, currshape);
            break;
        case TreeShapeType::HEMISPHR:
            genHemispherePlant(trunkheight, trunkradius, currshape);
            break;
        case TreeShapeType::CYL:
            genCylinderPlant(trunkheight, trunkradius, currshape);
            break;
        default:
            break;
        }
        shapes[0][s] = currshape;
    }
}


void ShapeGrid::bindPlantsSimplified(Terrain *ter, PlantGrid *esys, std::vector<bool> * plantvis, std::vector<Plane> cullPlanes)
{
    int x, y, s, p, sx, sy, ex, ey, f;
    PlantPopulation * plnts;
    int bndplants = 0, culledplants = 0;
    int gwidth, gheight;

    ter->getGridDim(gwidth, gheight);
    Region wholeRegion = Region(0, 0, gwidth - 1, gheight - 1);
    esys->getRegionIndices(ter, wholeRegion, sx, sy, ex, ey);
    Region parentRegion;
    float parentX0, parentY0, parentX1, parentY1, parentDimx, parentDimy;

    bool parentRegionAvailable = ter->getSourceRegion(parentRegion, parentX0, parentY0,
                                                      parentX1, parentY1, parentDimx, parentDimy);
    // std::cout << " +++++ Parent origin = " << (parentRegionAvailable ? "[DEFINED]" : "[NULL]")
    //          << " = (" << parentX0 << "," << parentY0 << ")\n";

    // std::vector<std::vector<glm::mat4> > xforms; // transformation to be applied to each instance
    std::vector<std::vector<glm::vec3> > xformsTrans; // transformation to be applied to each instance - tranalte (x,y,z)
    std::vector<std::vector<glm::vec2> > xformsScale; // transformation to be applied to each instance - scale (base, height)
    std::vector<std::vector<float> > colvars; // colour variation to be applied to each instance (scale value multiplied in shader)

    vpPoint loc;
    float rad;

    for(x = sx; x <= ex; x++)
        for(y = sy; y <= ey; y++)
        {
            plnts = esys->getPopulation(x, y);

            for(s = 0; s < (int) plnts->pop.size(); s++) // iterate over plant types
            {
                //std::vector<glm::mat4> xform; // transformation to be applied to each instance
                std::vector<glm::vec3> xformTrans;
                std::vector<glm::vec2> xformScale;

                std::vector<float> colvar; // colour variation to be applied to each instance

                //if((int) plnts->pop[s].size() > 0)
                //    cerr << "Species " << s << " Present" << endl;
                if((* plantvis)[s])
                    {
                    //glm::vec4 species_base_color = biome->getSpeciesColourV4(s);

                    for(p = 0; p < (int) plnts->pop[s].size(); p++) // iterate over plant specimens
                    {
                        rad = plnts->pop[s][p].canopy/2.0; // radius = 0.5 canopy_width
                        loc = plnts->pop[s][p].pos;

                        bool regionExclude = false;
                        if (parentRegionAvailable)
                        {
                            loc.x -= parentY0; // PCM: this x/y flip is very confusing...
                            loc.z -= parentX0;

                            if ( (loc.x-rad) < 0.0 || (loc.z-rad) < 0.0 ||
                                (loc.x+rad) > (parentY1-parentY0) || (loc.z+rad) > (parentX1 - parentX0))
                                regionExclude = true;
                        }

                         // ***** PCM 2023 - cull plant cylinder against planes if cullPlanes available
                        bool cull = false;
                        if (cullPlanes.size() > 0 && !regionExclude)
                        {
                            //rad = plnts->pop[s][p].canopy/2.0; // radius = 0.5 canopy_width
                            //loc = plnts->pop[s][p].pos;
                            for (std::size_t planes = 0; planes < cullPlanes.size(); ++planes)
                            {
                                // if candidate cyl is beyond plane or overlaps with plane, fails test (small tolerance)
                                if (cullPlanes[planes].side(loc) == true || cullPlanes[planes].dist(loc) < rad + 0.01)
                                {
                                    cull = true;
                                    break;
                                }
                            }

                        }

                        // *****************************************

                        // only display reasonably sized plants
                        if(plnts->pop[s][p].height > 0.01f && !regionExclude &&
                               (cullPlanes.size() == 0 || (cullPlanes.size() > 0 && cull == false) ) )
                        {
                            // setup transformation for individual plant, including scaling and translation
                            //glm::mat4 idt, tfm;
                            //glm::vec3 trs, sc; // rotate_axis = glm::vec3(0.0f, 1.0f, 0.0f);
                            // loc = plnts->pop[s][p].pos;
                            // GLfloat rotate_rad;

                            /*
                            if (plnts->pop[s+a][p].iscanopy)	// we use a different generator depending on whether we have a canopy or undergrowth plant - keeps rotations the same whether we render undergrowth plants or not
                            {
                                rotate_rad = rand_unif(generator_canopy) * glm::pi<GLfloat>() * 2.0f;
                            }
                            else
                            {
                                rotate_rad = rand_unif(generator_under) * glm::pi<GLfloat>() * 2.0f;
                            }*/

                            /*
                            idt = glm::mat4(1.0f);
                            trs = glm::vec3(loc.x, loc.y, loc.z);
                            tfm = glm::translate(idt, trs);
                            sc = glm::vec3(plnts->pop[s][p].canopy, plnts->pop[s][p].height, plnts->pop[s][p].canopy);		// XXX: use this for actual tree models
                            tfm = glm::scale(tfm, sc);
                            */
                            // tfm = glm::rotate(tfm, rotate_rad, rotate_axis);
                            xformTrans.push_back(glm::vec3(loc.x, loc.y, loc.z));
                            xformScale.push_back(glm::vec2(plnts->pop[s][p].canopy, plnts->pop[s][p].height));
                            //xform.push_back(tfm);

                            colvar.push_back(plnts->pop[s][p].col); // colour variation
                            bndplants++;
                        }
                        else
                        {
                            culledplants++;
                        }
                    }

                }

                if (xformsScale.size() < s + 1)
                {
                    xformsScale.resize(s  + 1);
                }
                if (xformsTrans.size() < s + 1)
                {
                    xformsTrans.resize(s  + 1);
                }
                if (colvars.size() < s + 1)
                {
                    colvars.resize(s + 1);
                }
                xformsTrans[s].insert(xformsTrans[s].end(), xformTrans.begin(), xformTrans.end());
                xformsScale[s].insert(xformsScale[s].end(), xformScale.begin(), xformScale.end());

                colvars[s].insert(colvars[s].end(), colvar.begin(), colvar.end());

                f = flatten(x, y);
                shapes[f][s].removeAllInstances();
            }
        }

    assert(xformsTrans.size() == xformsScale.size());

    for (std::size_t i = 0; i < xformsTrans.size(); i++)
    {
        shapes[0][i].removeAllInstances();
        shapes[0][i].bindInstances(&xformsTrans[i], &xformsScale[i], &colvars[i]);
        //shapes[0][i].bindInstances(&xforms[i], &colvars[i]);
    }
    // DEBUG:
    // std::cerr << "bindPlantsSimplified - bound: " << bndplants << "; culled: " << culledplants << std::endl;
}

void ShapeGrid::bindPlants(View * view, Terrain * ter, std::vector<bool> * plantvis, PlantGrid * esys, Region region)
{
    int x, y, s, p, sx, sy, ex, ey, f;
    PlantPopulation * plnts;
    int bndplants = 0, culledplants = 0;
    int gwidth, gheight;

    ter->getGridDim(gwidth, gheight);
    Region wholeRegion = Region(0, 0, gwidth - 1, gheight - 1);
    esys->getRegionIndices(ter, wholeRegion, sx, sy, ex, ey);

    //std::vector<std::vector<glm::mat4> > xforms; // transformation to be applied to each instance
    std::vector<std::vector<glm::vec3> > xformsTrans; // transformation to be applied to each instance - tranalte (x,y,z)
    std::vector<std::vector<glm::vec2> > xformsScale; // transformation to be applied to each instance - scale (base, height)

    std::vector<std::vector<float> > colvars; // colour variation to be applied to each instance (in shader - scalar needed)

    for(x = sx; x <= ex; x++)
        for(y = sy; y <= ey; y++)
        {
            plnts = esys->getPopulation(x, y);

            for(s = 0; s < (int) plnts->pop.size(); s+=3) // iterate over plant types
            {
                //std::vector<glm::mat4> xform; // transformation to be applied to each instance
                std::vector<glm::vec3> xformTrans;
                std::vector<glm::vec2> xformScale;
                std::vector<float> colvar; // colour variation to be applied to each instance

                for(int a = 0; a < 3; a++)
                {
                    for(p = 0; p < (int) plnts->pop[s+a].size(); p++) // iterate over plant specimens
                    {
                        if(plnts->pop[s+a][p].height > 0.01f) // only display reasonably sized plants
                        {
                            // setup transformation for individual plant, including scaling and translation
                            //glm::mat4 idt, tfm;
                            //glm::vec3 trs, sc; // rotate_axis = glm::vec3(0.0f, 1.0f, 0.0f);
                            vpPoint loc = plnts->pop[s+a][p].pos;
                            // GLfloat rotate_rad;

                            /*
                            if (plnts->pop[s+a][p].iscanopy)	// we use a different generator depending on whether we have a canopy or undergrowth plant - keeps rotations the same whether we render undergrowth plants or not
                            {
                                rotate_rad = rand_unif(generator_canopy) * glm::pi<GLfloat>() * 2.0f;
                            }
                            else
                            {
                                rotate_rad = rand_unif(generator_under) * glm::pi<GLfloat>() * 2.0f;
                            }*/

                            /*
                            idt = glm::mat4(1.0f);
                            trs = glm::vec3(loc.x, loc.y, loc.z);
                            tfm = glm::translate(idt, trs);
                            sc = glm::vec3(plnts->pop[s+a][p].height, plnts->pop[s+a][p].height, plnts->pop[s+a][p].height);		// XXX: use this for actual tree models
                            tfm = glm::scale(tfm, sc);
                            */
                            xformTrans.push_back(glm::vec3(loc.x, loc.y, loc.z));
                            xformScale.push_back(glm::vec2(plnts->pop[s+a][p].height, plnts->pop[s+a][p].height) );

                            // xform.push_back(tfm);

                            colvar.push_back(plnts->pop[s+a][p].col); // colour variation
                            bndplants++;
                        }
                        else
                        {
                            culledplants++;
                        }
                    }

                }
                if (xformsScale.size() < s / 3 + 1)
                {
                    xformsScale.resize(s / 3 + 1);
                }
                if (xformsTrans.size() < s / 3 + 1)
                {
                    xformsTrans.resize(s / 3 + 1);
                }
                if (colvars.size() < s / 3 + 1)
                {
                    colvars.resize(s / 3 + 1);
                }
                xformsTrans[s / 3].insert(xformsTrans[s / 3].end(), xformTrans.begin(), xformTrans.end());
                xformsScale[s / 3].insert(xformsScale[s / 3].end(), xformScale.begin(), xformScale.end());
                colvars[s / 3].insert(colvars[s / 3].end(), colvar.begin(), colvar.end());

                f = flatten(x, y);
                shapes[f][s / 3].removeAllInstances();
            }
        }

    assert(xformsTrans.size() == xformsScale.size());

    for (std::size_t i = 0; i < xformsTrans.size(); i++)
    {
        cerr << i << endl;
        shapes[0][i].removeAllInstances();
        //shapes[0][i].bindInstances(&xforms[i], &colvars[i]);
        shapes[0][i].bindInstances(&xformsTrans[i], &xformsScale[i], &colvars[i]);
    }
    cerr << "num bound plants = " << bndplants << endl;
    cerr << "num culled plants = " << culledplants << endl;
}

void ShapeGrid::drawPlants(std::vector<ShapeDrawData> &drawParams)
{
    int s; // x, y, f;
    ShapeDrawData sdd;

    for(s = 0; s < (int) shapes[0].size(); s++) // iterate over plant types
    {
        sdd = shapes[0][s].getDrawParameters();
        sdd.current = false;
        drawParams.push_back(sdd);
    }
}


/// EcoSystem

EcoSystem::EcoSystem()
{
    biome = new Biome();
    init();
}

EcoSystem::EcoSystem(Biome * ecobiome)
{
    biome = ecobiome;
    init();
}

EcoSystem::~EcoSystem()
{
    esys.delGrid();
    eshapes.delGrid();
    transectShapes.delGrid();

    for(int i = 0; i < (int) niches.size(); i++)
    {
        niches[i].delGrid();
    }
    niches.clear();
}

void EcoSystem::init()
{
    esys = PlantGrid(pgdim, pgdim);
    eshapes = ShapeGrid(pgdim, pgdim, biome);

    // add in copy for transect plants...geometry generated by ShapeGrid constructor call above *once*
    transectShapes = eshapes;

    // cmap = ConditionsMap();

    for(int i = 0; i < maxNiches; i++)
    {
        PlantGrid pgrid(pgdim, pgdim);
        niches.push_back(pgrid);
    }
    clear();
    maxtreehght = 0.0f;
    srand (time(NULL));
}

void EcoSystem::clear()
{
    esys = PlantGrid(pgdim, pgdim);

    for(int i = 0; i < (int) niches.size(); i++)
    {
        niches[i] = PlantGrid(pgdim, pgdim);
    }
}

void EcoSystem::pickPlants(Terrain * ter, TypeMap * clusters)
{
    Region reg = clusters->getRegion();
    esys.clearRegion(ter, reg);
    for(int n = 0; n < (int) niches.size(); n++)
    {
        niches[n].pickPlants(ter, clusters, n, esys);
    }
    // dirtyPlants = true;
}

void EcoSystem::pickAllPlants(Terrain * ter, bool canopyOn, bool underStoreyOn)
{
    //esys.clear(); WR?
    for(int n = 0; n < (int) niches.size(); n++)
    {
        if(n == 0 && canopyOn)
            niches[n].pickAllPlants(ter, 0.0f, 0.0f, 1.0f, esys);
        if(n > 0 && underStoreyOn)
            niches[n].pickAllPlants(ter, 0.0f, 0.0f, 1.0f, esys);
    }
    // dirtyPlants = true;
}

void EcoSystem::bindPlantsSimplified(Terrain *ter, std::vector<ShapeDrawData> &drawParams, std::vector<bool> * plantvis,
                                    bool rebind, std::vector<Plane> cullPlanes)
{
    bool applyToTransect = false; // is this bind call for main window or transect window?

    // we assume if cullPlanes are defined, its for the transect window (this hold currently)

    applyToTransect = (cullPlanes.size() > 0 ? true : false);

    if(rebind) {
        // plant positions have been updated since the last bindPlants
        if (applyToTransect == false)
            eshapes.bindPlantsSimplified(ter, &esys, plantvis, cullPlanes);
        else
            transectShapes.bindPlantsSimplified(ter, &esys, plantvis, cullPlanes);
    }

    if (applyToTransect == false)
        eshapes.drawPlants(drawParams);
    else
        transectShapes.drawPlants(drawParams);
}

void EcoSystem::placePlant(Terrain *ter, NoiseField * nfield, const basic_tree &tree)
{
    float tx, ty;
    int gx, gy;
    // cerr << "x = " << tree.x << " y = " << tree.y << endl;
    ter->getTerrainDim(tx, ty);
    ter->getGridDim(gx, gy);
    float h = ter->getHeightFromReal(tx - tree.y, tree.x);
    vpPoint pos(tree.x, h, tx - tree.y);
    // cerr << "h = " << h << endl;

    int spc = tree.species;

    // introduce small random variation in colour
    // PCM: I swapped x/y for call to getNoise: I am not sure what correct order is, but this avoid out-of-range error in coordinate lookup (and
    // we really just want a random numbetr).
    float rndoff = nfield->getNoise(vpPoint(tx-tree.y,h,tree.x), tx, ty)*0.3f; // (float)(rand() % 100) / 100.0f * 0.3f;
    // glm::vec4 coldata = glm::vec4(-0.15f+rndoff, -0.15f+rndoff, -0.15f+rndoff, 1.0f); // randomly vary lightness of plant
    // PCM: now done in shader!
    /*
    if (canopy && (fmod(pos.x / 0.9144f, 0.5f) < 1e-4 || fmod(pos.z / 0.9144f, 0.5f) < 1e-4))
    {
        std::cout << "Middle of cell encountered " << std::endl;
    }
    if (canopy)
    {
        std::cout << "Adding canopytree xy: " << pos.x << ", " << pos.z << std::endl;
    }
    */

    //Plant plnt = {pos, tree.height, tree.radius, coldata};	//XXX: not sure if I should multiply radius by 2 here - according to scaling info in the renderer, 'radius' is actually the diameter, as far as I can see (and visual results also imply this)
    Plant plnt = {pos, tree.height, tree.radius, rndoff};
    esys.placePlant(ter, spc, plnt);
}

void EcoSystem::placeManyPlants(Terrain *ter, NoiseField * nfield, const std::vector<basic_tree> &trees)
{
    for (int i = 0; i < int(trees.size()); i++)
    {
        // std::cerr << i << std::endl;
        placePlant(ter, nfield, trees[i]);
    }
}
