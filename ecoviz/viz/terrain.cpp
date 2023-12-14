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


// terrain.h: model for terrain. Responsible for storage and display of heightfield terrain data
// author: James Gain
// date: 17 December 2012

#ifdef _WIN32
#include <glew.h>
#else
#include <GL/glew.h>
#endif

#include "terrain.h"
#include <sstream>
#include <streambuf>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <utility>

using namespace std;

bool Terrain::inGridBounds(int x, int y)
{
    int gx, gy;
    getGridDim(gx, gy);
    return x >= 0 && x < gx && y >= 0 && y < gy;
}

void Terrain::toGrid(vpPoint p, float & x, float & y, float & h) const
{
    int gx, gy;
    float tx, ty, convx, convy;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    convx = (float) (gx-1) / tx;
    convy = (float) (gy-1) / ty;
    x = p.x * convx;
    y = p.z * convy;
    h = p.y;
    if(scaleOn)
        h *= scfac;
}


void Terrain::toGrid(vpPoint p, int &x, int &y) const
{
    int gx, gy;
    float tx, ty, convx, convy;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    convx = (float) (gx-1) / tx;
    convy = (float) (gy-1) / ty;
    x = (int) (p.x * convx);
    y = (int) (p.z * convy);
}


float Terrain::toGrid(float wdist) const
{
    int gx, gy;
    float tx, ty, conv;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    conv = (float) (gx-1) / tx;
    return wdist * conv;
}


vpPoint Terrain::toWorld(float x, float y, float h) const
{
    int gx, gy;
    float tx, ty, convx, convy;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    convx = tx / (float) (gx-1);
    convy = ty / (float) (gy-1);

    return vpPoint(x * convx, h, y * convy);
}

vpPoint Terrain::toWorld(int x, int y, float h) const
{
    int gx, gy;
    float tx, ty, convx, convy;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    convx = tx / (float) (gx-1);
    convy = ty / (float) (gy-1);

    return vpPoint((float) x * convx, h, (float) y * convy);
}

float Terrain::getHeightFromReal(float x, float y)
{
    int gx, gy;
    toGrid(vpPoint(x, 0, y), gx, gy);
    return grid->get(gx,gy);
}

float Terrain::toWorld(float gdist) const
{
    int gx, gy;
    float tx, ty, conv;

    getGridDim(gx, gy);
    getTerrainDim(tx, ty);
    conv = tx / (float) (gx-1);

    return gdist * conv;
}


bool Terrain::inWorldBounds(vpPoint p) const
{
    return (p.x >= 0.0f && p.x <= dimx && p.z >= 0.0f && p.z <= dimy);
}

bool Terrain::inSynthBounds(vpPoint p) const
{
    return (p.x >= 0.0f-synthx && p.x <= dimx+synthx && p.z >= 0.0f-synthy && p.z <= dimy+synthy);
}

void Terrain::init(int dx, int dy, float sx, float sy)
{
    grid->setDim(dx, dy);
    drawgrid->setDim(dy, dx);
    setTerrainDim(sx, sy);
    setFocus(vpPoint(sy/2.0f, grid->get(dx/2-1,dy/2-1), sx/2.0f));
    scfac = 1.0f;

    // init accel structure
    spherestep = 8;
    numspx = (grid->height()-1) / spherestep + 1; numspy = (grid->width()-1) / spherestep + 1; // JG mod
    for(int i = 0; i < numspx; i++)
    {
        std::vector<AccelSphere> sphrow;
        for(int j = 0; j < numspy; j++)
        {
            AccelSphere sph;
            sphrow.push_back(sph);
        }
        boundspheres.push_back(sphrow);
    }

    bufferState = BufferState::REALLOCATE;
    accelValid = false;
    scaleOn = false;
}

void Terrain::initGrid(int dx, int dy, float sx, float sy)
{
    init(dx, dy, sx, sy);
    grid->fill(0.0f);
    drawgrid->fill(0.0f);
}

void Terrain::delGrid()
{
    if(boundspheres.size() > 0)
    {
        for(int i = 0; i < (int) boundspheres.size(); i++)
            boundspheres[i].clear();
        boundspheres.clear();
    }

    bufferState = BufferState::REALLOCATE;
    accelValid = false;
}

// PCM ----
// NOTE: source region is *grid relative* - all other data is preserved, scale etc.
// assume end points are included.

 std::unique_ptr<Terrain> Terrain::buildSubTerrain(int x0, int x1, int y0, int y1)
 {
    int dx = x1-x0+1;
    int dy = y1-y0+1;
    float val;

    std::unique_ptr<Terrain> newTerrain(new Terrain);

    newTerrain->init(dx, dy, (float) dx * step, (float) dy * step);
    newTerrain->step = step;
    newTerrain->scfac = scfac;
    newTerrain->scaleOn = scaleOn;
    // other state that may have changed since init()?

    // copy data
    for (int x = 0; x < dx; x++)
    {
        for (int y = 0; y < dy; y++)
        {
            val  = grid->get(x0+x,y0+y);

            newTerrain->grid->set(x,y, val);
            newTerrain->drawgrid->set(y,x, val);
        }
    }

    newTerrain->calcMeanHeight();

    return newTerrain;
 }



void Terrain::setMidFocus()
{
    int dx, dy;
    float sx, sy;
    
    getGridDim(dx, dy);
    getTerrainDim(sx, sy);
    if(dx > 0 && dy > 0)
        setFocus(vpPoint(sy/2.0f, grid->get(dx/2-1,dy/2-1), sx/2.0f));
    else
        setFocus(vpPoint(0.0f, 0.0f, 0.0f));
}

void Terrain::getMidPoint(vpPoint & mid)
{
    int dx, dy;
    float sx, sy;

    getGridDim(dx, dy);
    getTerrainDim(sx, sy);
    if(dx > 0 && dy > 0)
        mid = vpPoint(sx/2.0f, grid->get(dy/2-1,dx/2-1), sy/2.0f);
    else
        mid = vpPoint(0.0f, 0.0f, 0.0f);

}

void Terrain::getGridDim(int & dx, int & dy) const
{
    dx = grid->width();
    dy = grid->height();
}

void Terrain::getGridDim(uint & dx, uint & dy)
{
    dx = (uint) grid->width();
    dy = (uint) grid->height();
}

void Terrain::getTerrainDim(float &tx, float &ty) const
{
    tx = dimx; ty = dimy;
}

void Terrain::setTerrainDim(float tx, float ty)
{
    int gx, gy;

    getGridDim(gx, gy);
    dimx = tx; dimy = ty;

    // calc allowable synth border
    synthx = (0.5f / (float) (gx-1) + pluszero) * dimx;
    synthy = (0.5f / (float) (gy-1) + pluszero) * dimy;
}

float Terrain::samplingDist()
{
    int dx, dy;
    float tx, ty;
    getGridDim(dx, dy);
    getTerrainDim(tx, ty);
    return (0.5f * std::min(tx, ty)) / (float) (std::max(dx,dy)-1); // separation between vertices, about 2-3 vertices per grid cell
}

float Terrain::smoothingDist()
{
    return 30.0f * samplingDist(); // about 10 grid points per curve segment
}

float Terrain::longEdgeDist()
{
    float tx, ty;
    getTerrainDim(tx, ty);
    return std::max(tx, ty);
}

float Terrain::getHeight(int x, int y)
{
    return grid->get(x,y);
}

float Terrain::getFlatHeight(int idx)
{
    int x, y, dx, dy;
    getGridDim(dx, dy);

    x = idx % dx;
    y = idx / dx;
    return grid->get(x,y);
}

void Terrain::getNormal(int x, int y, Vector & norm)
{
    vpPoint x1, x2, y1, y2;
    int dx, dy;
    Vector dfdx, dfdy;

    getGridDim(dx, dy);

    // x-positions
    if(x > 0)
        x1 = toWorld(x-1, y, getHeight(x-1, y));
    else
        x1 = toWorld(x, y, getHeight(x, y));

    if(x < dx-1)
        x2 = toWorld(x+1, y, getHeight(x+1, y));
    else
        x2 = toWorld(x, y, getHeight(x, y));

    // y-positions
    if(y > 0)
        y1 = toWorld(x, y-1, getHeight(x, y-1));
    else
        y1 = toWorld(x, y, getHeight(x, y));

    if(y < dy-1)
        y2 = toWorld(x, y+1, getHeight(x, y+1));
    else
        y2 = toWorld(x, y, getHeight(x, y));

    // cross pattern
    dfdx.diff(x1, x2);
    dfdy.diff(y1, y2);
    dfdx.normalize();
    dfdy.normalize();

    norm.cross(dfdx, dfdy);
    norm.mult(-1.0f); // JGBUG - may be wrong direction
    norm.normalize();
}

float Terrain::getTerrainHectArea()
{
    float tx, ty, area;

    getTerrainDim(tx, ty);
    area = tx * ty; // sq metres
    area /= 10000.0f; // 10,000 sq metres in a hectare
    return area;
}

float Terrain::getCellExtent()
{
    return dimx / (float) grid->width();
}

void Terrain::updateBuffers(PMrender::TRenderer * renderer)
{
    const int width = grid->width();
    const int height = grid->height();
    float scx, scy;

    getTerrainDim(scx, scy);

    glewExperimental = GL_TRUE;
    if(!glewSetupDone)
      {
         GLenum err = glewInit();
         if (GLEW_OK != err)
         {
            std::cerr<< "GLEW: initialization failed\n\n";
         }
         glewSetupDone = true;
      }

    if (bufferState == BufferState::REALLOCATE || bufferState == BufferState::DIRTY )
    {
        // renderer->updateHeightMap(width, height, scx, scy, drawgrid->getPtr(), true);
        renderer->updateHeightMap(height, width, scy, scx, drawgrid->getPtr(), true);
    }
    else
    {
        // renderer->updateHeightMap(width, height, scx, scy, drawgrid->getPtr());
        renderer->updateHeightMap(height, width, scy, scx, drawgrid->getPtr());
    }

    bufferState = BufferState::CLEAN;
}

void Terrain::draw(View * view, PMrender::TRenderer *renderer)
{
    updateBuffers(renderer);

    // call draw function
    renderer->draw(view);
}

void Terrain::buildSphereAccel()
{
    int si, sj, i, j, imin, imax, jmin, jmax;
    float rad, sqlen;
    vpPoint p, c, b1, b2;
    Vector del;

    // cerr << "numspx = " << numspx << ", numspy = " << numspy << endl;
    for(si = 0; si < numspx; si++)
        for(sj = 0; sj < numspy; sj++)
        {
            imin = si*spherestep; imax = std::min(imin+spherestep, grid->height());
            jmin = sj*spherestep; jmax = std::min(jmin+spherestep, grid->width());
            // cerr << "(" << si << ", " << sj << ") = " << "i: " << imin << " - " << imax << " j: " << jmin << " - " << jmax << endl;

            // center point
            b1 = toWorld(imin, jmin, grid->get(jmin, imin));
            b2 = toWorld(imax, jmax, grid->get(jmax-1, imax-1));
            c.affinecombine(0.5f, b1, 0.5f, b2);

            // update radius
            rad = 0.0f;
            for(j = jmin; j < jmax; j++)
                for(i = imin; i < imax; i++)
                {
                    p = toWorld(i, j, grid->get(j,i));
                    del.diff(c, p);
                    sqlen = del.sqrdlength();
                    if(sqlen > rad)
                        rad = sqlen;
                }
            boundspheres[si][sj].center = c;
            boundspheres[si][sj].radius = sqrtf(rad);
        }
    accelValid = true;
}

bool Terrain::rayIntersect(vpPoint start, Vector dirn, vpPoint & p)
{
    int i, j, si, sj, imin, imax, jmin, jmax;
    vpPoint currp;
    float besttval, tval, dist;
    bool found = false;
    float tol = dimx / (float) (grid->width()-1); // set world space detection tolerance to approx half gap between grid points

    besttval = 100000000.0f;

    if(!accelValid)
        buildSphereAccel();

    // bounding sphere accel structure
    for(si = 0; si < numspx; si++)
        for(sj = 0; sj < numspy; sj++)
        {
            rayPointDist(start, dirn, boundspheres[si][sj].center, tval, dist);
            if(dist <= boundspheres[si][sj].radius) // intersects enclosing sphere so test enclosed points
            {
                imin = si*spherestep; imax = std::min(imin+spherestep, grid->height());
                jmin = sj*spherestep; jmax = std::min(jmin+spherestep, grid->width());
                // check ray against grid points
                for(j = jmin; j < jmax; j++)
                    for(i = imin; i < imax; i++)
                    {
                        currp = toWorld(i, j, grid->get(j,i));
                        rayPointDist(start, dirn, currp, tval, dist);
                        if(dist < tol)
                        {
                            found = true;
                            if(tval < besttval)
                            {
                                besttval = tval;
                                p = currp;
                            }
                        }
                    }

            }

        }

    return found;
}

bool Terrain::pick(int sx, int sy, View * view, vpPoint & p)
{
    vpPoint start;
    Vector dirn;

    // find ray params from viewpoint through screen <sx, sy>
    view->projectingRay(sx, sy, start, dirn);

    return rayIntersect(start, dirn, p);
}

bool Terrain::drapePnt(vpPoint pnt, vpPoint & drape)
{
    float x, y, h, drapeh, u, v, h0, h1, ux, uy;
    int cx, cy, dx, dy;

    getGridDim(dx, dy);
    toGrid(pnt, x, y, h); // locate point on base domain

    // test whether point is in bounds
    ux = (float) (dy-1) - pluszero;
    uy = (float) (dx-1) - pluszero;

    if(x < pluszero || y < pluszero || x > ux || y > uy)
        return false;

    // index of grid cell
    cx = (int) floor(x);
    cy = (int) floor(y);

    // get parametric coordinates within grid cell
    u = (x - (float) cx);
    v = (y - (float) cy);

    // bilinear interpolation
    h0 = (1.0f - u) * grid->get(cy,cx) + u * grid->get(cy,cx+1);
    h1 = (1.0f - u) * grid->get(cy+1,cx) + u * grid->get(cy+1,cx);
    drapeh = (1.0f - v) * h0 + v * h1;
    // this could be implemented using ray-triangle intersection
    // but it would be much less efficient
    drape = toWorld(x, y, drapeh);

    return true;
}

void Terrain::loadElv(const std::string &filename, int dFactor)
{
    //float lat;
    int dx, dy;


    float val;
    ifstream infile;



    infile.open((char *) filename.c_str(), ios_base::in);
    if(infile.is_open())
    {
        std::size_t count =0;
        infile >> dx >> dy;
        infile >> step;

        assert(dx > dFactor);
        assert(dy > dFactor);

        int newdx = int(dx/dFactor) + ( dx % dFactor > 0 ? 1: 0),
            newdy = int(dy/dFactor) + ( dy % dFactor > 0 ? 1: 0);

        // infile >> lat;

        delGrid();
        init(newdx, newdy, (float) dx * step, (float) dy * step);
        // latitude = lat;
        // original code: outer loop over x, inner loop over y
        // raster format (ESRI) is oriented differently
        for (int x = 0; x < dx; x++)
        // for (int y = 0; y < dy; y++)
        {
            for (int y = 0; y < dy; y++)
            // for (int x = 0; x < dx; x++)
            {
                infile >> val;
                // only take every dFactor'th sample, starting at 0
                if (x % dFactor == 0 && y % dFactor == 0)
                {
                    count++;
                    grid->set(x/dFactor, y/dFactor, val); //  * 0.3048f); // convert from feet to metres
                    drawgrid->set(y/dFactor, x/dFactor, val); // * 0.3048f);
                }
            }
        }

        assert(count == newdx*newdy);

        setMidFocus();
        infile.close();
    }
    else
    {
        cerr << "Error Terrain::loadElv (with downsample): unable to open file " << filename << endl;
    }
}
void Terrain::loadElv(const std::string &filename)
{
    //float lat;
    int dx, dy;

    float val;
    ifstream infile;

    infile.open((char *) filename.c_str(), ios_base::in);
    if(infile.is_open())
    {
        infile >> dx >> dy;
        infile >> step;

        // infile >> lat;

        delGrid();
        init(dx, dy, (float) dx * step, (float) dy * step);
        // latitude = lat;
        // original code: outer loop over x, inner loop over y
        // raster format (ESRI) is oriented differently
        for (int x = 0; x < dx; x++)
        // for (int y = 0; y < dy; y++)
        {
            for (int y = 0; y < dy; y++)
            // for (int x = 0; x < dx; x++)
            {
                infile >> val;
                grid->set(x, y, val); //  * 0.3048f); // convert from feet to metres
                drawgrid->set(y, x, val); // * 0.3048f);
            }
        }
        setMidFocus();
        infile.close();
    }
    else
    {
        cerr << "Error Terrain::loadElv:unable to open file " << filename << endl;
    }
}

void Terrain::saveElv(const std::string &filename)
{
    int gx, gy;

    ofstream outfile;

    outfile.open((char *) filename.c_str(), ios_base::out);
    if(outfile.is_open())
    {
        getGridDim(gx, gy);
        outfile << gx << " " << gy << " " << step << " " << latitude << endl;
        for (int x = 0; x < gx; x++)
        {
            for (int y = 0; y < gy; y++)
            {
                outfile << grid->get(x,y) << " ";
            }
        }
        outfile << endl;
        outfile.close();
    }
    else
    {
        cerr << "Error Terrain::loadElv:unable to open file " << filename << endl;
    }
}

void Terrain::calcMeanHeight()
{
    int i, j, cnt = 0;
    hghtmean = 0.0f;

    for(j = 0; j < grid->height(); j++)
        for(i = 0; i < grid->width(); i++)
        {
            hghtmean += grid->get(j,i);
            cnt++;
        }
    hghtmean /= (float) cnt;
}

void Terrain::getHeightBounds(float &minh, float &maxh)
{
    int i, j;
    float hght;

    maxh = -10000000.0f;
    minh = 100000000.0;

    for(j = 0; j < grid->height(); j++)
        for(i = 0; i < grid->width(); i++)
        {
            hght = grid->get(j,i);
            if(hght < minh)
                minh = hght;
            if(hght > maxh)
                maxh = hght;
        }
}
