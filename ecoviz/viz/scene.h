// scene.h: wrapper class to manage all data associated with a simulated botanical scene
// author: James Gain
// date: 27 February 2016

#ifndef SCENE_H
#define SCENE_H

#include "eco.h"
#include "dice_roller.h"
#include "common/basic_types.h"
#include "stroke.h"
#include "typemap.h"
#include "shape.h"
#include "cohortsampler.h"
#include "mitsuba_model.h"

// minimum and maximum transect thickness
const float mintwidth = 10.0f;
const float maxtwidth = 100.0f;

class Transect
{
private:
    // Terrain * terrain;      //< underlying terrain
    vpPoint center;     //< center point, midway along the slicing line on the terrain surface
    vpPoint bounds[2];  //< bounding extremes of the transect where it reaches the edge of the terrain
    vpPoint inners[2];  //< demarcating points of the inner section of the transect
    vpPoint clampedinners[2]; //< inner section endpoints clamped to the terrain bounds
    Vector normal;      //< normal of the slicing plane
    Vector vert, hori;  //< vertical and horizontal vectors in the transect plane
    float thickness;    //< width of the transect on either side of the slicing plane
    float extent;       //< base-plane length of seperation between inner demarcating points
    bool redraw;        //< whether or not a redraw of transect manipulators is required
    bool valid;       //< true if a valid transect exists, false otherwise
    basic_types::MapFloat * mapviz; //< visualization in map form of the thickness of the transect

    /**
     * @brief findBoundPoints From a source point and vector direction find points on the defined line lying on the extreme edges of the terrain
     * @param src   starting point for line
     * @param dirn  direction of the line
     * @param bnd   bounding points
     * @param ter   underlying terrain
     * @return      return true if the line intersects the terrain bounds, false otherwise
     */
    bool findBoundPoints(vpPoint src, Vector dirn, vpPoint * bnd, Terrain * ter);

    /**
     * @brief paintThickness Draw the transect thickness visualization into mapviz
     */
    void paintThickness(Terrain * ter);

    /**
     * @brief inBounds Check whether a point lies within the bounds of the vertical projection of the terrain
     * @param pnt   Point being bounds checked
     * @param ter   Underlying Terrain
     * @return      true if with the terrain bounds, false otherwise
     */
    bool inBounds(vpPoint pnt, Terrain * ter);

public:

    Transect(Terrain * ter)
    {
        mapviz = new basic_types::MapFloat();
        init(ter);
        redraw = false;
        valid = false;
    }

    ~Transect()
    {
        delete mapviz;
    }

    // initialize to default setting based on terrain
    inline void init(Terrain * ter)
    {
        thickness = 20.0f;
        ter->getMidPoint(center);
        normal = Vector(0.0f, 0.0f, 1.0f);
        float rw, rh;
        ter->getTerrainDim(rw, rh);
        extent = rw;

        int dx, dy;
        ter->getGridDim(dx, dy);
        mapviz->setDim(dx, dy);
        mapviz->fill(0.0f);
    }

    inline void reset()
    {
        redraw = false;
        valid = false;
        mapviz->fill(0.0f);
        thickness = 20.0f;
    }

    // getters and setters
    inline void setThickness(float width, Terrain * ter)
    {
        // ensure thickness remains within bounds
        thickness = std::max(width, mintwidth);
        thickness = std::min(thickness, maxtwidth);
        paintThickness(ter);
        redraw = true;
    }

    inline float getThickness(){ return thickness; }
    inline float getExtent(){ return extent; }
    inline void setCenter(vpPoint c){ center = c; }
    inline vpPoint getCenter(){ return center; }
    inline Vector getNormal(){ return normal; }
    inline vpPoint getBoundStart(){ return bounds[0]; }
    inline vpPoint getBoundEnd(){ return bounds[1]; }
    inline vpPoint getInnerStart(){ return inners[0]; }

    inline void setInnerStart(vpPoint s, Terrain * ter)
    {
        inners[0] = s;
        if(inBounds(inners[0], ter))
            clampedinners[0] = inners[0];
        else
            clampedinners[0] = bounds[0];
        redraw = true;
    }
    inline vpPoint getInnerEnd(){ return inners[1]; }
    inline void setInnerEnd(vpPoint e, Terrain * ter)
    {
        inners[1] = e;
        if(inBounds(inners[1], ter))
            clampedinners[1] = inners[1];
        else
            clampedinners[1] = bounds[1];
        redraw = true;
    }
    inline vpPoint getClampedInnerStart(){ return clampedinners[0]; }
    inline vpPoint getClampedInnerEnd(){ return clampedinners[1]; }
    inline Vector getHorizontal(){ return hori; }
    inline Vector getVertical(){ return vert; }
    inline bool getChangeFlag(){ return redraw; }
    inline bool getValidFlag(){ return valid; }
    inline void setValidFlag(bool status){ valid = status; }
    inline basic_types::MapFloat * getTransectMap(){ return mapviz; }
    inline void setTransectMap(basic_types::MapFloat * mviz){ mapviz = mviz; }

    /**
     * @brief clearChangeFlag reset change flag to false
     */
    void clearChangeFlag(){ redraw = false; }

    /**
     * @brief derive Derive a transect passing through two points on the terrain
     * @param p1    first point
     * @param p2    second point
     * @param ter   underlying terrain
     */
    void derive(vpPoint p1, vpPoint p2, Terrain * ter);

    /**
     * @brief zoom  Adjust the seperation between innerpoints to simulate zooming in and out of a transect
     * @param zdel  adjustment to zoom
     * @param ter   underlying terrain
     */
    void zoom(float zdel, Terrain * ter);

    // PCM 2023 ****
    // return the planes bounding the transect;
    // planeBaseOrigin = centre of 1st plane (for use with view transformation)
    std::pair<Plane, Plane> getTransectPlanes(vpPoint &planeBaseOrigin);
};

struct TransectCreation
{
    vpPoint t1, t2;     // endpoints on the terrain
    int trxstate;       // current state of selecting endpoings
    Transect * trx;     // all remaining transect state
    bool showtransect;  // whether or not display of the transect indicators is active
    Shape trxshape[3];  //< geometry for transect line display
};

class Timeline
{
private:
    // Timeline assumes integer timeline values in [start, end] with internal intervals of timestep
    int start, end, current;  //< starting, ending and current times
    int timestep;             //< integer interval between

public:

    Timeline()
    {
        start = 0; end = 0; current = 0; timestep = 1;
    }

    Timeline(int tstart, int tend, int tnow, int tdelta)
    {
        start = tstart;
        end = tend;
        current = tnow;
        timestep = tdelta;
    }

    /**
     * @brief getCurrentIdx Get the current array index for timeline related data
     * @return array index position for current time
     */
    int getCurrentIdx(){ return (current - start) / timestep; }

    /**
     * @brief setCurrentIdx Set the current array index for the timeline
     * @param t     index position for current time
     */
    void setCurrentIdx(int t)
    {
        setNow(start + t * timestep);
    }

    /**
     * @brief getNumIdx Get the size of the timeline array
     * @return  array size
     */
    int getNumIdx(){ return (end - start + 1) / timestep; }

    // getters and setters for timeline attributes
    void setNow(int t)
    {
        // snap to timestep interval
        int trunct = (int) round((float) (t - start) / (float) timestep);
        current = start + timestep * trunct;

        // ensure that it is within bounds
        current = max(start, current);
        current = min(end, current);
    }

    int getNow(){ return current; }
    int getTimeStart(){ return start; }
    int getTimeEnd(){ return end; }
    int getTimeStep(){ return timestep; }
    void getTimeBounds(int &tstart, int &tend){ tstart = start; tend = end; }
    void setTimeBounds(int tstart, int tend){ start = tstart; end = tend; }
};

class Scene;

class TimelineGraph
{
private:
    Timeline * timeline;    //< associated timeline for marking the current time and retrieving timeline bounds
    std::vector<std::vector<float>> graphdata; //< per attribute (e.g., species) per timestep data
    int hscale;             //< number of steps in the timeline
    float vscale;             //< highest value on the verical axis
    int numseries;          //< number of attributes
    std::string title;      //< title for the graph
    static std::vector< std::string > graph_titles;

public:

    TimelineGraph();

    TimelineGraph(Timeline * tline, int nseries, std::string name);

    ~TimelineGraph();

    void init();

    /// getters and setters
    Timeline * getTimeLine(){ return timeline; }
    void setTimeLine(Timeline * tline){ timeline = tline; init(); }
    void setTitle(std::string name){ title = name; }
    void setVertScale(float vertscale){ vscale = vertscale; }
    float getVertScale(){ return vscale; }
    void setNumSeries(int nseries){ numseries = nseries; init(); }
    int getNumSeries(){ return numseries; }
    int getHoriScale(){ return hscale; }
    std::string getTitle() { return title; }

    // types of data series
    enum ChartType { ChartBasalArea, ChartStemNumber, ChartDBHDistribution} ;
    static std::vector<ChartType> getChartTypes() { return {ChartBasalArea, ChartStemNumber, ChartDBHDistribution}; }

    /**
     * @brief extractData build the aggregate statistics for the time series
     * @param chart_type  type of data
     */
    void extractDataSeries(Scene *scene, ChartType chart_type);

    /**
     * @brief assignData Assign single data value for an attribute at a particular time
     * @param attrib    attribute to which the data is being assigned
     * @param time      time for which assignment takes place
     * @param value     value being set
     */
    void assignData(int attrib, int time, float value);
    float getData(int attrib, int time){ return graphdata[attrib][time]; }

    /**
     * @brief extractSpeciesCounts Create a graph for the number of instances of each species over the timeline period
     * @param s     Scene for extracting counts
     */
    void extractSpeciesCounts(Scene * s);

    /**
     * @brief extractDBHSums Create a graph for the total diameter at breast height of each species over the timeline period
     * @param s     Scene for extracting counts
     */
    void extractDBHSums(Scene * s);

    /**
     * @brief extractNormalizedBasalArea Create a graph for the total basal area normalized by landscape size for each species over the timeline period
     * @param s     Scene for extracting counts
     */
    void extractNormalizedBasalArea(Scene * s);
};

// lightweight scene class for overview map
class mapScene
{
private:
    std::unique_ptr<Terrain> fullResTerrain;   // input terrain -  full resolution
    std::unique_ptr<Terrain> lowResTerrain;    // low resolution terrain for overview rendering
    std::unique_ptr<Terrain> subwTerrain;      // terrain sub-window corresponding to user overview selection
    std::unique_ptr<TypeMap> overlay;          // single overlay supported (blended over terrain)
    std::string datadir;                       // directory containing all the scene data
    std::string overlayName;                   // name of overlap image
    Region subwindow;                          // subwindow extents (relative to full resolution terrain)

    // downsampling/rescaling
    bool sampledReady;
    int downFactor;

    // ensure scene directory is valid
    std::string get_dirprefix();

public:

    mapScene(const std::string & ddir, const std::string overlayNm) :
        fullResTerrain(new Terrain), lowResTerrain(new Terrain), subwTerrain(new Terrain),
        overlay(new TypeMap)
    {
        overlayName = overlayNm;
        datadir = ddir;
        sampledReady = false;
        downFactor = 4;
        subwindow.x0 = subwindow.y0 = subwindow.x1 = subwindow.y1 = -1;
    }
    ~mapScene();


    // getters

    std::unique_ptr<Terrain> & getHighResTerrain(void)  { return fullResTerrain; }
    std::unique_ptr<Terrain> & getLowResTerrain(void) { return lowResTerrain; }
    std::unique_ptr<Terrain> & getTerrainSubwindow(void) { return subwTerrain; }
    std::unique_ptr<TypeMap> & getOverlayMap(void) { return overlay; }
    Region getSubwindowExtents(void) const { return subwindow;}
    bool subwindowValid(void)
    {
        if (subwindow.x0 == -1 || subwindow.x1 == -1 ||
            subwindow.x1 == -1 || subwindow.y1 == -1)
            return false;

        return true;
    }
    void setDownsampleFactor(int factor) { downFactor = factor; }

    // extract the sub-region specified by region and update internal data structures
    // return the new extracted terrain for later processing.
    bool extractTerrainSubwindow(Region region)
    {
        if (!subwindowValid())
            return false;

        subwTerrain = fullResTerrain->buildSubTerrain(region.x0, region.y0, region.x1, region.y1);

        return true;
    }

    void loadOverViewData(void)
    {

        std::string terfile = datadir+"/dem.elv";

        // load terrain
        fullResTerrain->loadElv(terfile);
        fullResTerrain->calcMeanHeight();

        // create downsampled overview

        lowResTerrain->loadElv(terfile, downFactor);
        lowResTerrain->calcMeanHeight();

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


    }
};

class Scene
{
private:
    Terrain * terrain;                          //< underlying terrain
    TypeMap * maps[(int) TypeMapType::TMTEND];  //< underlying type map data
    basic_types::MapFloat * slope, * chm, * cdm;             //< condition maps
    std::vector<basic_types::MapFloat *> sunlight;           //< local per cell illumination for each month
    std::vector<basic_types::MapFloat *> moisture;           //< local per cell moisture for each month
    std::vector<float> temperature;             //< average monthly temperature
    string datadir;                             //< directory containing all the scene data
    Timeline * tline;                           //< timeline
    NoiseField * nfield;                        //< random noise map

    ValueGridMap<std::vector<data_importer::ilanddata::cohort> > before_mod_map;


    EcoSystem * eco;
    Biome * biome;

    // ensure scene directory is valid
    std::string get_dirprefix();

public:

    std::unique_ptr<CohortMaps> cohortmaps;     //< agreggate ecosystem data
    std::unique_ptr<cohortsampler> sampler;     //< to derive individual trees from cohort maps

    Scene(string ddir);

    ~Scene();

    /// getters for currently active view, terrain, typemaps, renderer, ecosystem
    Terrain * getTerrain(){ return terrain; }
    TypeMap * getTypeMap(TypeMapType purpose){ return maps[static_cast<int>(purpose)]; }
    EcoSystem * getEcoSys(){ return eco; }
    basic_types::MapFloat * getSunlight(int month){ return sunlight[month]; }
    basic_types::MapFloat * getSlope(){ return slope; }
    basic_types::MapFloat * getMoisture(int month){ return moisture[month]; }
    basic_types::MapFloat * getCanopyHeightModel(){ return chm; }
    basic_types::MapFloat * getCanopyDensityModel(){ return cdm; }
    Biome * getBiome(){ return biome; }
    Timeline * getTimeline(){ return tline; }
    NoiseField * getNoiseField(){ return nfield; }

    /**
     * @brief calcSlope    Calculate per cell ground slope
     */
    void calcSlope();

    /**
     * @brief readMonthlyMap Read a set of 12 maps from file
     * @param filename   name of file to be read
     * @param monthly    content will be loaded into this vector of maps
     */
    bool readMonthlyMap(std::string filename, std::vector<basic_types::MapFloat *> &monthly);

    /**
     * @brief writeMonthlyMap    Write a set of 12 maps to file
     * @param filename   name of file to be written
     * @param monthly    map content to be written
     */
    bool writeMonthlyMap(std::string filename, std::vector<basic_types::MapFloat *> &monthly);

    /// read and write condition maps
    bool readSun(std::string filename);
    bool writeSun(std::string filename);
    bool readMoisture(std::string filename);
    bool writeMoisture(std::string filename);

    /**
    * @brief reset_sampler  Recreate the sampling
    * @param maxpercell Maximum number of trees that can appear in an individual cell
    */
    void reset_sampler(int maxpercell);

    /**
     * @brief loadScene     Load scene attributes located in the specified directory (or default initialization if no directory provided)
     * @param dirprefix     combined directory path and file name prefix containing the scene
     * @param timestep_start
     * @param timestep_end
     */
    void loadScene(int timestep_start, int timestep_end);
    void loadScene(std::string dirprefix, int timestep_start, int timestep_end);

     /**
      * Export the scene (for Mitsuba) to the XML specified
      * @param speciesMap      Correspondence map between the plant type and a vector binding a height to a mitsuba id
      * @param xmlFile         XML file in which the scene will be exported
      * @param transect        Transect control in case the export concerns only the transect view, nullptr instead
      */
     void exportSceneXml(map<string, vector<MitsubaModel>>& speciesMap, ofstream& xmlFile, Transect * transect = nullptr);
};

#endif // SCENE_H
