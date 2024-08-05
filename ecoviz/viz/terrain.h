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


/**
 * @file
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include <memory>
#include "vecpnt.h"
#include "view.h"
#include "trenderer.h"
#include "common/basic_types.h"

#define DEFAULT_DIMX 512
#define DEFAULT_DIMY 512

// terrain.h: model for terrain. Responsible for storage and display of heightfield terrain data
// author: James Gain
// date: 17 December 2012

class AccelSphere
{
private:
    friend class boost::serialization::access;
    /// Boost serialization
    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        ar & center; ar & radius;
    }

public:
    vpPoint center;
    float radius;
};

/// Renderable heightmap
class Terrain
{

private:
    /// State of the vertex buffer objects
    enum class BufferState
    {
        REALLOCATE,       ///< Size is wrong, create new ones
        DIRTY,            ///< Content is wrong
        CLEAN             ///< Everything is up to date
    };

    /// Vertex in the vertex buffer
    struct Vertex
    {
        GLfloat position[3];
        GLfloat normal[3];
    };

    basic_types::MapFloat * grid;           ///< grid of height values in metres
    basic_types::MapFloat * drawgrid;       ///< inverted grid for rendering
    vpPoint focus;                          ///< focal point fo view

    float dimx, dimy;                       ///< dimensions of terrain in metres
    long locx, locy;                        ///< location of the top-left corner of the terrain
    float synthx, synthy;                   ///< offset to allow for synthesis in metres
    float scfac;                            ///< artificial vertical scaling to make large terrains more discernable
    float step;                             ///< interval between grid vertices in meters

    int numspx, numspy;                     ///< dimensions of accel structure
    int spherestep;                         ///< number of grid points per accel sphere
    bool accelValid;                        ///< true if the accel structure does not need updating
    bool scaleOn;                           ///< is the terrain being scaled
    std::vector<std::vector<AccelSphere>> boundspheres; ///< bounding sphere accel structure

    mutable BufferState bufferState = BufferState::REALLOCATE;  ///< Buffer state

    float hghtrange;        ///< maximum terrain height range from synthesizer
    float hghtmean;         ///< mean terrain height, calculated on first synthesis

    float latitude;         ///< latitude location of terrain in degrees, if available

    Region sourceRegion;    ///< source region for a terrain built from larger terrain;
                            ///< caller must maintain knowledge of parent; this is used for
                            ///< ecosystem offsetting from master terrain to this sub-terrain
                            ///< by default it is not set (0 for each entry): NOTE: these are
                            ///< *grid* coordinates relative to parent grid - they must be converted to
                            ///< world space for terrain checks.
    int parentGridx;       ///< if source Region is set, this will also be set and indicate parent grid
    int parentGridy;       ///< samples in x/y (NOTE: float terrain extent can be recovered as dim*step

    // PM: terrain renderer
    mutable bool glewSetupDone;
    mutable GLuint htMapTextureId;

    /// re-build sphere acceleration structure when terrain is changed
    void buildSphereAccel();

    /// internal intialisation
    void init(int dx, int dy, float sx, float sy);

    //void updateBuffers(PMrender::TRenderer *renderer) const;

    friend class boost::serialization::access;
    /// Boost serialization
    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        ar & grid;
        ar & focus;
        ar & dimx; ar & dimy;
        ar & synthx; ar & synthy;
        ar & numspx; ar & numspy;
        ar & spherestep;
        ar & accelValid;
        ar & boundspheres;
        ar & hghtrange;
        ar & hghtmean;
    }

public:

    /// Constructor
    Terrain(Region source = Region()) // empty source region by default
    {
        glewSetupDone = false; dimx = dimy = synthx = synthy = 0.0f; numspx = numspy = 0;
        hghtrange = 0.0f; hghtmean = 0.0f; accelValid = false;
        grid = new basic_types::MapFloat();
        drawgrid = new basic_types::MapFloat();
        // PCM - set only if this terrain was created from (larger) parent terrain
        sourceRegion = source;
        parentGridx = parentGridy = 0;
      }

    /// Destructor
    ~Terrain()
    {
        delete grid;
        delete drawgrid;
    }


    //returns true if source region is set.
    // src: the grid coordinates (relative to source) of this terrain
    // sx, sy, ex. ey are "physical" terrain  coordinates covered by source
    // parentDimx/y are the full original terrain extents (in meters)
    bool getSourceRegion(Region &src, float &sx, float &sy, float &ex, float &ey,
                         float& parentDimx, float & parentDimy) const
    {
        src = sourceRegion;
        sx = src.x0*step;
        sy = src.y0*step;
        ex = src.x1*step;
        ey = src.y1*step;

        parentDimx = (parentGridx)*step;
        parentDimy = (parentGridy)*step;

        return !sourceRegion.empty();
    }

    Region getEntireRegion()
    {
        int dx, dy;
        getGridDim(dx, dy);
        return Region(0, 0, dx, dy);
    }

    void setHeightMaptextureID(GLuint id) { htMapTextureId = id; }

    /**
     * Update the vertex buffers if necessary. This is called immediately before rendering.
     *
     * @pre There is a current OpenGL contex
     * @post @ref bufferState == @ref BufferState::CLEAN
     */
    void updateBuffers(PMrender::TRenderer *renderer);

    /// Allocate memory for a grid of size @a dx by @a dy, and scale @a sx by @a sy in metres
    void initGrid(int dx, int dy, float sx, float sy);

    // delGrid: deallocate grid
    void delGrid();

    // draw: display terrain using OpenGL as a triangle mesh
    void draw(View * view, PMrender::TRenderer *renderer);

    // setFocus, getFocus: access methods for view focus on terrain
    inline void setFocus(vpPoint f){ focus = f; }
    inline vpPoint getFocus(){ return focus; }
    
    /// set the focal point to the middle of the terrain
    void setMidFocus();

    /// get middle of terrain
    void getMidPoint(vpPoint & mid);

    /// getter for maximum terrain extent in horizontal or vertical
    float getMaxExtent(){ return std::max(dimx, dimy); }

    /// getter for mean height
    float getHeightMean(){ return hghtmean; }

    /// getter for height range
    float getHeightRange(){ return hghtrange; }

    /// calculate the minimum and maximum height over the terrain
    void getHeightBounds(float &minh, float &maxh);

    /// Get terrain area in hectares
    float getTerrainHectArea();

    /// getter for extent of individual grid cell in world coordinates
    float getCellExtent();

    /// getter for area of individual grid cell in world coordinates
    float getCellArea(){ return getCellExtent()*getCellExtent(); }

    /// set buffer to dirty to force render reload
    void setBufferToDirty(){ bufferState = BufferState::DIRTY; accelValid = false; }

    /// Raise the focus so that it sits on the terrain after synthesis
    inline void raiseFocus()
    {
        vpPoint drape;
        drapePnt(focus, drape); focus = drape;
    }

    /// Obtain height at a particular grid position
    float getHeight(int x, int y);

    /// Obtain height at a flattened grid index
    float getFlatHeight(int idx);

    /// Return a world-space normal given a grid position
    void getNormal(int x, int y, Vector & norm);

    /// check if a point is within the grid dimensions, return true if it is
    bool inGridBounds(int x, int y);

    /// Obtain grid size @a dx and @a dy
    void getGridDim(int & dx, int & dy) const;
    void getGridDim(uint &dx, uint & dy);

    /// Obtain terrain extents in metres
    void getTerrainDim(float &tx, float &ty) const;

    // Obtain the global corner position
    void getTerrainLoc(long &lx, long &ly);

    /// Set terrain extents in metres
    void setTerrainDim(float tx, float ty);

    /// Return sub-grid sampling distance in metre world coordinates
    float samplingDist();

    /// Return control point separation in metre world coordinates
    float smoothingDist();

    /// Return longest edge extent of terrain in metres
    float longEdgeDist();

    /// get distance between points in metres
    float getPointStep() const { return step; }

    /// getter for artificial scale factor
    float getScaleFactor(){ return scfac; }

    /// setter for artificial scale factor
    void setScaleFactor(float scale){ scfac = scale; }

    /// setter for turning scaling of terrain height on/off
    void setScaleOn(bool scaling){ scaleOn = scaling; }

    /// getter for whether scaling of terrain height on/off
    bool getScaleOn(){ return scaleOn; }

    /// getter for latitude
    float getLatitude(){ return latitude; }

    /**
     * Convert from world coordinates in the range (0, height, 0) to (dimx, height, dimy) to grid coordinates
     */
    void toGrid(vpPoint p, float & x, float & y, float & h) const;

    /// convert from world coordinates to integer grid coordinates, discarding height
    void toGrid(vpPoint p, int &x, int &y) const;

    /// convert world distance to grid distance
    float toGrid(float wdist) const;

    /**
     * Convert from terrain coordinates to world coordinates in the range (0, height, 0) to (dimx, height, dimy)
     */
    vpPoint toWorld(float x, float y, float h) const;
    vpPoint toWorld(int x, int y, float h) const;

    /// convert grid distance to world distance
    float toWorld(float gdist) const;

    /// Is the point inside the bounds of the terrain in world coordinates
    bool inWorldBounds(vpPoint p) const;

    /// Is the point inside the synth bounds of the terrain in world coordinates
    bool inSynthBounds(vpPoint p) const;

    /**
     * Find the intersection of an arbitrary ray with the terrain.
     *
     * @param start    start point of ray
     * @param dirn     direction of ray
     * @param[out] p   Picked point, if the return value is @c true
     * @retval @c true if the ray strikes the terrain.
     * @retval @c false otherwise.
     */
    bool rayIntersect(vpPoint start, Vector dirn, vpPoint & p);

    /**
     * Find the intersection of a ray from the center of projection through
     * screen coordinates with the terrain.
     *
     * @param sx, sy   Center of projection
     * @param view     View specifying the projection
     * @param[out] p   Picked point, if the return value is @c true
     * @retval @c true if the ray from the center of projection through screen coordinates
     *                 (@a sx, @a sy) passes within @a tol of a grid point on
     *                 the terrain.
     * @retval @c false otherwise.
     */
    bool pick(int sx, int sy, View * view, vpPoint & p);

    /**
     * Drop a point in world coordinates vertically until it intersects the terrain
     * @param pnt   initial position of the point
     * @param drape position after being dropped vertically
     * @retval @c true if within the range of the terrain.
     * @retval @c   false otherwise.
     */
    bool drapePnt(vpPoint pnt, vpPoint & drape);

    /// Create a flat terrain
    void test();

    /**
       * Load a terrain from a (text) file. (.elv)
       * @param filename   File to load (simple ascii elevation format)
       */
    void loadElv(const std::string &filename);

    /**
       * Load a terrain from a binary file (.elvb).
       * @param filename   File to load (simple ascii elevation format)
       */
    void loadElvBinary(const std::string &filename);

    /**
       * Load a terrain from a file - pply dwonsampling (skip every nth sample)
       * @param filename   File to load (simple ascii elevation format)
       * @param downsample donsam,pling factor (integer > 1)
       */
    void loadElv(const std::string &filename, int downsample);
    /* As above, but open a binary file */
    void loadElvBinary(const std::string &filename, int downsample);

    /**
       * Save a terrain to file.
       * @param filename   File to save (simple ascii elevation format)
       * @see @ref MemMap for exception information
       */
    void saveElv(const std::string &filename);

    /// Recalculate the mean height over the terrain
    void calcMeanHeight();
    float getHeightFromReal(float x, float y);

    /// PCM - build new terrain from sub-region -
    /// calling function must assume responsibility for memory

    std::unique_ptr<Terrain> buildSubTerrain(int x0, int y0, int x1, int y1);
};

#endif // TERRAIN_H
