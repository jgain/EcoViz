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


// eco.h: core classes for controlling ecosystems and plant placement
// author: James Gain
// date: 27 February 2016

#ifndef _eco_h
#define _eco_h

#define LOWRES

#include "pft.h"
#include "dice_roller.h"
#include "common/basic_types.h"
#include "unordered_map"
#include "boost/functional/hash.hpp"

const int maxNiches = 10;  //< maximum number of initial terrain niches from HL system
const int maxSpecies = 96; // multiplier is for three age categories
const int pgdim = 50;

struct Plant
{
    vpPoint pos;    //< position on terrain in world units
    float height;   //< plant height in metres
    float canopy;   //< canopy radius in metres
    glm::vec4 col;  //< colour variation randomly assigned to plant
};

struct SubSpecies
{
    std::string name;   //< subspecies name
    int chance;         //< chance of selecting this species, out of 1000
};

struct PlantPopulation
{
    std::vector<std::vector<Plant>> pop;    //< list of all plants in an ecosystem by type
};

class NoiseField
{
private:
    long seed;      //< random number initialization to ensure a Noisefield can be reproduced
    Terrain * terrain;  //< underlying terrain
    int dimx, dimy; //< dimsions of the noisefield (a multiplicative factor of the terrain)
    basic_types::MapFloat * nmap;  //< field of noise values
    DiceRoller * dice; //< random number generator

public:

    /**
     * @brief NoiseField initializer
     * @param ter   underlying terrain
     * @param dstep number of sub-cells per terrain cell (basically a multiplication factor)
     * @param sval  seed value
     */
    NoiseField(Terrain * ter, int dstep, long sval);

    ~NoiseField(){ delete dice; }

    /// initialize the noisemap
    void init();

    /// recover a random value in [0, 1] at a point on the terrain
    float getNoise(vpPoint p);
};

class PlantGrid
{
private:
    std::vector<PlantPopulation> pgrid; //< flattened grid holding plant populations
    std::vector<std::vector<SubSpecies>> speciesTable; //< name and probabilities for subspecies assignment

    /// return the row-major linearized value of a grid position
    inline int flatten(int dx, int dy){ return dx * gy + dy; }

    /// reset grid to empty state
    void initGrid();

    /**
     * @brief cellLocate    Find the cell index of location in map coordinates
     * @param ter           Terrain onto which the ecosystem is placed
     * @param mx            x-location in map coordinates
     * @param my            y-location in map coordinates
     * @param cx            x-index in grid
     * @param cy            y-index in grid
     */
    void cellLocate(Terrain * ter, int mx, int my, int &cx, int &cy);

    /// return the number of plant species in the plant population
    int numSpecies()
    {
        if((int) pgrid.size() > 0)
            return (int) pgrid[0].pop.size();
        else
            return 0;
    }

    /// Initialise the list of species according to Biome
    void initSpeciesTable();

public:
    int gx, gy;     //< grid dimensions

    PlantGrid(){ gx = 0; gy = 0; initGrid(); initSpeciesTable(); }

    PlantGrid(int dx, int dy){ gx = dx; gy = dy; initGrid(); initSpeciesTable(); }

    ~PlantGrid(){ delGrid(); speciesTable.clear(); }

    /// completely delete grid
    void delGrid();

    /// clear the contents of the grid to empty
    void clear(){ initGrid(); }

    /// Check whether the grid contains any plants
    bool isEmpty();

    /**
     * @brief initRnd Set up the underlying noise field
     * @param ter   Corresponding terrain
     * @param steps Number of steps in each terrain cell
     */
    void initRnd(Terrain * ter, int steps);

    /**
     * @brief clearCell Clear all plants in a specified grid cell
     * @param x     x-location in grid
     * @param y     y-location in grid
     */
    void clearCell(int x, int y);

    /**
     * @brief clearRegion   Remove plants within a contiguous block of cells
     * @param ter           Terrain onto which plants are placed
     * @param region        Region to clear
     */
    void clearRegion(Terrain * ter, Region region);

    /**
     * @brief placePlant    Insert a plant into the correct species population localised with the grid
     * @param ter       Terrain onto which the plant is to be placed
     * @param species   Species type
     * @param plant     Plant information
     */
    void placePlant(Terrain * ter, int species, Plant plant);

    /**
     * @brief placePlant    Insert a plant into the correct species population at a given location in the grid
     * @param ter       Terrain onto which the plant is to be placed
     * @param species   Species type
     * @param plant     Plant information
     * @param x         x-location
     * @param y         y-location
     */
    void placePlantExactly(Terrain * ter, int species, Plant plant, int x, int y);

    /// Output current sate of the PlantGrid
    void reportNumPlants();

    /**
     * @brief pickPlants    Select plants that match the current niche and place in an output grid
     * @param ter           Terrain onto which the plants will be placed
     * @param clusters      Map of ecosystem niches
     * @param niche         Required niche that must be matched during plant selection
     * @param outgrid       Place where plants will be placed
     */
    void pickPlants(Terrain * ter, TypeMap * clusters, int niche, PlantGrid & outgrid);

    /**
     * @brief pickAllPlants Select all plants from the current grid and copy to an output grid indiscriminately
     * @param ter           Terrain onto which the plants will be placed
     * @param offx          x-coordinate offset added to plant position
     * @param offy          y-coordinate offset added to plant position
     * @param scf           scale factor for initial position and size before offset
     * @param outgrid       Place where plants will be placed
     */
    void pickAllPlants(Terrain * ter, float offx, float offy, float scf, PlantGrid & outgrid);

    /**
     * @brief setPopulation copy a plant population into the grid
     * @param x     grid location in the x direction
     * @param y     grid location in the y direction
     * @param pop   plant population to place in the grid
     */
    void setPopulation(int x, int y, PlantPopulation & pop);

    /**
     * @brief getPopulation get a plant population from the grid
     * @param x     grid location in the x direction
     * @param y     grid location in the y direction
     * @return      population at the specified grid position
     */
    PlantPopulation * getPopulation(int x, int y);

    /**
     * @brief getSynthPoints get a collection of synthesized points from the grid for possible modification
     * @param x     grid location in x
     * @param y     grid location in y
     * @return      access to synthesized points at the specified grid position
     */
    // std::vector<AnalysisPoint> * getSynthPoints(int x, int y);

    /**
     * @brief getRegionIndices  Find the grid bounds of a map region
     * @param ter       Terrain onto which the ecosystem is placed
     * @param region    Bounding region in map coordinates
     * @param sx        starting x-index
     * @param sy        starting y-index
     * @param ex        ending x-index
     * @param ey        ending y-index
     */
    void getRegionIndices(Terrain * ter, Region region, int &sx, int &sy, int &ex, int &ey);

    /**
     * @brief vectoriseByPFT    Select all plants that match a specific plant functional type
     * @param pft       Plant functional type for selection
     * @param pftPlnts  Vector in which to place extracted plants
     */
    void vectoriseByPFT(int pft, std::vector<Plant> &pftPlnts);
};

/// Plant Rendering
class ShapeGrid
{
private:
    std::vector<std::vector<Shape>> shapes; //< shape templates for each grid cell and each species
    Biome * biome;                          //< biome determines shape and colour of trees
    int gx, gy;                             //< grid dimensions to match a plant grid

    /// return the row-major linearized value of a grid position
    inline int flatten(int dx, int dy){ return dx * gy + dy; }

    /// reset grid to empty state
    void initGrid();

    /**
      * Create geometry for PFT with sphere top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genSpherePlant(float trunkheight, float trunkradius, Shape &shape);

    /**
      * Create geometry for PFT with tapered box top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param taper        proportion of box base to which box top tapers
      * @param scale        scale factor to reduce visual impact, otherwise this tree is visually dominant
      * @param shape        geometry for PFT
      */
    void genBoxPlant(float trunkheight, float trunkradius, float taper, float scale, Shape &shape);

    /**
      * Create geometry for PFT with cone top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genConePlant(float trunkheight, float trunkradius, Shape &shape);

    /**
      * Create geometry for PFT with inverted cone top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genInvConePlant(float trunkheight, float trunkradius, Shape &shape);

    /**
      * Create geometry for PFT with inversted cone top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genUmbrellaPlant(float trunkheight, float trunkradius, Shape &shape);

    /**
      * Create geometry for PFT with hemisphere top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genHemispherePlant(float trunkheight, float trunkradius, Shape &shape);

    /**
      * Create geometry for PFT with cylindrical top
      * @param trunkheight  proportion of height devoted to bare trunk
      * @param trunkradius  radius of main trunk
      * @param shape        geometry for PFT
      */
    void genCylinderPlant(float trunkheight, float trunkradius, Shape &shape);

public:

    ShapeGrid(){ gx = 0; gy = 0; }

    ShapeGrid(int dx, int dy, Biome * shpbiome){ gx = dx; gy = dy; biome = shpbiome; initGrid(); }

    ~ShapeGrid(){ delGrid(); }

    /// completely delete grid
    void delGrid();

    /// clear the contents of the grid to empty
    void clear(){ initGrid(); }

    /**
     * Create geometry to represent each of the Functional Plant Types over a grid
     */
    void genPlants();

    /**
     * @brief attachBiome Call when a new biome is loaded
     * @param shpbiome  Biome to be attached to the ShapeGrid
     */
    void attachBiome(Biome * shpbiome){ biome = shpbiome; genPlants(); }

    /**
     * @brief bindPlants    Update positioning information for plants within certain location bounds
     * @param view          The current viewpoint
     * @param ter           Terrain onto which plants will be bound
     * @param plantvis      Flags for which plant species are visible
     * @param esys          The ecosystem grid
     * @param region        A bound on the region to be updated
     */
    void bindPlants(View * view, Terrain * ter, std::vector<bool> * plantvis, PlantGrid * esys, Region region);
    void bindPlantsSimplified(Terrain *ter, PlantGrid *esys, std::vector<bool> * plantvis, std::vector<Plane> cullPlanes = {});

    /**
     * @brief drawPlants    Bundle rendering parameters for instancing lists
     * @param drawParams    Rendering parameters for the different plant species
     */
    void drawPlants(std::vector<ShapeDrawData> &drawParams);
};


enum class SamplingDimensions // Sampling Dimensions in the Database
{
    ANGLE,      //<
    WATER,      //< Average Soil Moisture per month over the course of a year
    SUN,        //< Average Sun Exposure per month over the course of a year
    TEMP,       //< Average Temperature per month over the course of a year
    OLD,        //< Age of the Ecosystem Simulation
    SDEND
};

/// Coordinates in the sampling database
struct SampleCoord
{
    int slope;
    int water;
    int sun;
    int temp;
    int age;
};

class EcoSystem
{
private:

    ShapeGrid eshapes;                //< graphical representation of ecosystem
    std::vector<PlantGrid> niches;    //< individual ecosystems for each niche
                                      //< the purpose of niches is to allow a coarse form of selection and rendering
                                      //< by default only the first niche is used
    float maxtreehght;                      //< largest tree height among all loaded species
    PlantGrid esys;                   //< combined output ecosystem
    Biome * biome;                      //< biome matching ecosystem

public:

    EcoSystem();

    EcoSystem(Biome * ecobiome);

    ~EcoSystem();

    /// Initialize ecosystem in empty state
    void init();

    /// Remove all plants and reset ecosystem
    void clear();

    /// Getter for maximum tree height
    float getMaxTreeHeight(){ return maxtreehght; }

    /// getPlants: return a pointer to the actual plants in the ecosystem. Assumes pickAllPlants has been called previously.
    PlantGrid * getPlants(){ return &esys; }

    /// getNiche: return a pointer to a particular ecosystem niche (n)
    PlantGrid * getNiche(int n){ return &niches[n]; }

    void setBiome(Biome * ecobiome){ biome = ecobiome; clear(); eshapes.attachBiome(ecobiome); }

    /**
       * Write plant positions to a PDB format text file
       * @param filename  name of file to load (PDB format)
       * @param niche     particular cluster for which plants are active, 0 means all clusters
       * @retval true  if save succeeds,
       * @retval false otherwise.
       */
    bool saveNichePDB(string filename, int niche = 0);

    /**
     * Pick plants from different niche ecosystems based on the cluster map to form a final ecosystem
     * @param ter       terrain onto which the plants will be placed
     * @param clusters  resource cluster texture map
     * NB: this will require that plants are rebound
     */
    void pickPlants(Terrain * ter, TypeMap * clusters);

    /**
     * Pick plants from all niche ecosystems to form a final ecosystem
     * @param ter       terrain onto which the plants will be placed
     * @param canopyOn  display the canopy in niche 0 if true
     * @param underStoreyOn display the understorey in niches > 0 if true
     * NB: this will require that plants are rebound
     */
    void pickAllPlants(Terrain * ter, bool canopyOn = true, bool underStoreyOn = true);

    /**
     * Position plants instances on terrain for subsequent rendering. Needs to be run after plant positions have changed.
     * @param ter           terrain onto which plants will be placed
     * @param plantvis      boolean array of which plant species are to be rendered and which not
     * @param drawParams    parameters for drawing plant species, appended to the current drawing parameters
     * @param bind          whether or not the plants need to be recreated after a change
     */
    void bindPlantsSimplified(Terrain *ter, std::vector<ShapeDrawData> &drawParams, std::vector<bool> * plantvis, bool bind=false, std::vector<Plane> cullPlanes = {});
    void placePlant(Terrain *ter, NoiseField * nfield, const basic_tree &tree);
    void placeManyPlants(Terrain *ter, NoiseField * nfield, const std::vector<basic_tree> &trees);
};

#endif
