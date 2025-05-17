/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  J.E. Gain  (jgain@cs.uct.ac.za)
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

// TypeMap.h: class for passing around terrian type information

#ifndef _TYPEMAP
#define _TYPEMAP

#include "glheaders.h"
#include <vector>
#include <memory>
#include <common/region.h>
#include "common/basic_types.h"

enum class TypeMapType
{
    EMPTY,              //< default colour for terrain
    TRANSECT,           //< to display the transect region
    GREYRAMP,           //< greyscale colour map, well suited to monitors
    HEATRAMP,           //< black, red, yellow, white colour map, typical heatmap
    BLUERAMP,           //< white to blue colour ramp, suitable for moisture
    TMTEND
};

const int numRamps = 3;
const std::array<std::string, 3> ramp_names = {"grey", "heat", "blue"};

//const std::array<TypeMapType, 10> all_typemaps = {TypeMapType::EMPTY, TypeMapType::TRANSECT, TypeMapType::CATEGORY, TypeMapType::SLOPE, TypeMapType::WATER, TypeMapType::SUNLIGHT, TypeMapType::TEMPERATURE, TypeMapType::CHM, TypeMapType::CDM, TypeMapType::SUITABILITY}; // to allow iteration over the typemaps
class DataMaps
{
private:
    std::vector<std::vector<basic_types::MapFloat *>> dmaps; ///< per year data maps for the entire terrain, to be converted on demand to textures
    std::vector<std::string> dnames;                         ///< the names for each map
    std::vector<float> dmax;                                 ///< maximum value for each data category across all years
    Region cover;                                            ///< region of terrain covered by maps
    int dimx, dimy;                                          ///< data map dimensions

    /**
     * @brief loadIndexMap  Load a map of indices in ESRI ASCII format
     * @param idxfilename   Index file name
     * @param idxmap        Map to be populated with indices (-1 represents an unassigned cell)
     */
    bool loadIndexMap(const std::string & idxfilename, basic_types::MapInt & idxmap);

    /**
     * @brief initMaps Initilize the maps data structure
     * @param numYears  Number of years, one set of maps per year
     * @param numMaps   Number of maps for each year
     * @param dx        Number of cells in x
     * @param dy        Number of cells in y
     */
    void initMaps(int numYears, int numMaps, int dx, int dy);

    /**
     * @brief indexToLoc    return the (x,y) location of an index in a provided index map
     * @param idxmap    map containing indices
     * @param idx       index to be found
     * @param x         x-coordinate of index
     * @param y         y-coordinate of index
     * return       true if the index is found in the index map, otherwise false
     */
    bool indexToLoc(basic_types::MapInt & idxmap, int idx, int & x, int & y);

    /**
     * @brief getTerVal Retrieve the value from a particular map for a particular year as specified in terrain region coordinates
     * @param year       Simulation year
     * @param midx       Data map index
     * @param tx         terrain region x-coordinate
     * @param ty         terrain region y-coordinate
     * @return   the value at the corresponding position in the relevant data map or -1.0f if undefined
     */
    float getTerVal(int year, int midx, int tx, int ty);

public:

    DataMaps(){ cover = Region(0, 0, 0, 0); dimx = 0; dimy = 0; }

    ~DataMaps()
    {
        clearMaps();
        dnames.clear();
        dmax.clear();
    }



    /**
     * @brief clearmaps Delete a previously created maps data structure
     */
    void clearMaps();

    // getters
    int getNumMaps(){ return (int) dnames.size(); }
    std::vector<std::string> * getNames(){ return &dnames; }

    /**
     * @brief getRange Retrieve the maximum value in a particular data map across all years
     * @param idx   Map index
     * @return      Maximum data value
     */
    float getRange(int idx){ return dmax[idx]; }

    /**
     * @brief loadDataMaps  Load data overlays for the terrain
     * @param idxfilename   Index file name
     * @param datafilename  Data file name, which assigns values to cell indices
     * @param numyears      Number of years in visualization
     */
    bool loadDataMaps(const std::string &idxfilename, const std::string &datafilename, int numyears);

    /**
     * @brief extractRegion Extract a subregion in terrain coordinates from a particular map for a given year
     * @param year          Simulation year
     * @param midx          Index of map
     * @param superRegion   Portion of the terrain covered by the entirey of the map
     * @param subRegion     Terrain region to extract
     * @param subMap        Extracted data map
     */
    void extractRegion(int year, int midx, Region superCover, Region subRegion, basic_types::MapFloat * subMap);
};

class TypeMap
{
private:
    basic_types::MapInt * tmap;      ///< a map corresponding to the terrain storing integer types
    std::vector<GLfloat *> colmap;  ///< a 32-element lookup table for converting type indices to colours
    Region dirtyreg;                ///< bounding box in terrain grid integer coordinates (e.g, x=[0-width), y=[0-hieght))
    TypeMapType usage;              ///< indicates map purpose
    int numSamples;                 ///< number of active entries in lookup table
    ///< global data map from which a sub-region is extracted on demand

    /// Set up the colour table with natural USGS inspired map colours
    void initNaturalColTable();

    /// Set up the colour table with colours appropriate to the initial ecosystem pallete of operations
    void initPaletteColTable();

    /// Set up two tone colour to display the transect region
    void initTransectColTable();

    /**
     * @brief initPerceptualColTable Set up a colour table sampled from a perceptually uniform colour map stored in a CSV file
     * @param colmapfile        file on disk containing the colour map
     * @param samples           number of samples taken from the colour map
     * @param truncend          proportion of the colourmap to select, truncating from the upper end
     */
    void initPerceptualColTable(std::string colmapfile, int samples, float truncend = 1.0f);

    /// clip a region to the bounds of the map
    void clipRegion(Region &reg);

public:

    TypeMap(){ usage = TypeMapType::EMPTY; }

    TypeMap(TypeMapType purpose);

    /**
     * Create type map that matches the terrain dimensions
     * @param w         map width
     * @param h         map height
     * @param purpose   map purpose, to represent different kinds of layers
     */
    TypeMap(int w, int h, TypeMapType purpose);

    virtual ~TypeMap();

    /// getters for width and height
    int width(){ return tmap->width(); }
    int height(){ return tmap->height(); }

    /// fill map with a certain colour
    void fill(int val){ tmap->fill(val); }

    /// Match type map dimensions to @a w (width) and @a h (height)
    void matchDim(int w, int h);
    
    /// clear typemap to unconstrained
    void clear();

    /// getter for underlying map
    basic_types::MapInt * getMap(void){ return tmap; }
    
    /// getter for individual value
    int get(int x, int y){ return tmap->get(y,x); }
    void set(int x, int y, int val){ tmap->set(y,x,val); }
    
    /// replace underlying map
    void replaceMap(basic_types::MapInt * newmap);

    /// load from file, return number of clusters
    int load(const std::string &filename, TypeMapType purpose);

    /// load from category data from PNG file, return number of clusters
    bool loadCategoryImage(const std::string &filename);

    /// convert a floating point map into a discretized type map
    int convert(basic_types::MapFloat * map, TypeMapType purpose, float range = 1.0f);

    /// save a mask file version of the type map
    void save(const std::string &filename);

    /**
     * @brief saveToPaintImage   Save paint map out as a greyscale image
     * @param filename      Name of file to save to
     */
    void saveToPaintImage(const std::string &filename);

    /// getter for colour table
    std::vector<GLfloat *> * getColourTable(void) { return &colmap; }

    /// getter for sampling range
    int getTopSample(){ return numSamples-1; }

    /// getter for update region
    Region getRegion(void) { return dirtyreg; }

    /// setter for update region
    void setRegion(const Region& toupdate)
    {
        dirtyreg = toupdate;
        clipRegion(dirtyreg);
    }

    /// return region that covers the entire type map
    Region coverRegion()
    {
        return Region(0,0,width(),height());
    }

    /// setter for purpose
    void setPurpose(TypeMapType purpose);

    /// Reset the indicated type to zero everywhere it appears in the map
    void resetType(int ind);

    /**
     * Index to colour translation. First element is the erase colour
     * @param ind   index for a type (potentially) stored in the type map
     * @retval      4-element RGBA colour associated with @a ind
     */
    GLfloat * getColour(int ind)
    {
        if(ind >= 0 && ind < 32) // limit on number of available colours
            return colmap[ind];
        else
            return NULL;
    }

    /**
     * Set the colour associated with a particular index
     * @param ind   index for the type that is to be given a new colour association
     * @param col   4-element RBGA colour to associate with @a ind
     */
    void setColour(int ind, GLfloat * col);

    int getNumSamples();
};

#endif
