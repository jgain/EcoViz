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

    // if non null ter provided, retarget this terrain
    inline void reset(Terrain *ter = nullptr)
    {
        redraw = false;
        valid = false;
        thickness = 20.0f;

        if (ter == nullptr)
        {
            mapviz->fill(0.0f);
        }
        else // reset to new terrain dimensions
        {
            delete mapviz;
            mapviz = new basic_types::MapFloat();
            init(ter);
        }
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

// for externally managing the saving and restoring sub-regions and camera views
class viewScene
{
public:

    Region region;  // active region of the global terrain
    View view;      // rendering viewpoint

    viewScene(){}
    viewScene(Region reg, View vw){ region = reg; view = vw; }

    // getters and setters
    void setRegion(Region reg){ region = reg; }
    void setView(View vw){ view = vw; }
    Region getRegion(){ return region; }
    View getView(){ return view; }

    // save region and view to file
    void save(std::string filename);

    // load region and view from file
    void load(std::string filename);
};

// lightweight scene class for overview map
class mapScene
{
private:
    std::unique_ptr<Terrain> fullResTerrain;   // input terrain -  full resolution
    std::unique_ptr<Terrain> lowResTerrain;    // low resolution terrain for overview rendering
    std::unique_ptr<TypeMap> overlay;          // single overlay supported (blended over terrain)
    std::string datadir;                       // directory containing all the scene data
    std::string basename;                      // the dem name (without extension)
    std::string overlayName;                   // name of overlap image
    Region selectedRegion;                     // current selected sub-region  - relative to hi-res input
                                               //(needed for overview render)

    // downsampling/rescaling
    int downFactor;

    // ensure scene directory is valid
    std::string get_dirprefix();

public:

    mapScene(const std::string & ddir, const std::string overlayNm, const std::string & base) :
        fullResTerrain(new Terrain), lowResTerrain(new Terrain),
        overlay(new TypeMap)
    {
        overlayName = overlayNm;
        datadir = ddir;
        basename = base;
        downFactor = 4;

        selectedRegion = Region();

        // dummy init to avoid issues with early paintGL() which expects valid terrain
        lowResTerrain->initGrid(10, 10, 100.0f, 100.0f);
    }
    ~mapScene() {}


    // getters

    std::unique_ptr<Terrain> & getHighResTerrain(void)  { return fullResTerrain; }
    std::unique_ptr<Terrain> & getLowResTerrain(void) { return lowResTerrain; }
    std::unique_ptr<TypeMap> & getOverlayMap(void) { return overlay; }
    void setSelectedRegion(Region reg) { selectedRegion = reg; } // NOTE: after this, sub-terr must be extracted again
    Region getSelectedRegion(void) const { return selectedRegion; }
    Region getEntireRegion() { return fullResTerrain->getEntireRegion(); }
    std::string getBaseName(void) const { return basename; }
    bool subwindowValid(Region subwindow);

    void setDownsampleFactor(int factor) { downFactor = factor; }

    // extract the sub-region specified by region and update internal data structures
    // return the new extracted terrain for later processing.
    std::unique_ptr<Terrain> extractTerrainSubwindow(Region region);


    // factor: default reduction factor to extract sub-region for main terrain (10 = 1/10th)
    // return value = a unique_ptr to extracted Terrain  that must be managed by the caller
    std::unique_ptr<Terrain> loadOverViewData(int factor = 10);

};

class Scene
{
private:
    std::unique_ptr<Terrain> terrain;           //< underlying terrain
    Terrain *masterTerrain;                     //< a pointer to the large input terrain this one is extracted from
    TypeMap * maps[(int) TypeMapType::TMTEND];  //< underlying type map data
    string datadir;                             //< directory containing all the scene data
    string basename;                            //< base name for DEM and sequence of PDBs
    Timeline * tline;                           //< timeline
    NoiseField * nfield;                        //< random noise map
    DataMaps * dmaps;                           //< data maps for extracting textures

    ValueGridMap<std::vector<data_importer::ilanddata::cohort> > before_mod_map;

    EcoSystem * eco;
    Biome * biome;

    // ensure scene directory is valid
    std::string get_dirprefix();

public:

    std::shared_ptr<CohortMaps> cohortmaps;     //< agreggate ecosystem data
    std::unique_ptr<cohortsampler> sampler;     //< to derive individual trees from cohort maps

    Scene(string ddir, string base);

    ~Scene();

    /// getters for currently active view, terrain, typemaps, renderer, ecosystem
    Terrain *  getTerrain(){ return terrain.get(); }
    Terrain *  getMasterTerrain() { return masterTerrain; }
    TypeMap * getTypeMap(TypeMapType purpose){ return maps[static_cast<int>(purpose)]; }
    EcoSystem * getEcoSys(){ return eco; }
    Biome * getBiome(){ return biome; }
    Timeline * getTimeline(){ return tline; }
    NoiseField * getNoiseField(){ return nfield; }
    DataMaps * getDataMaps(){ return dmaps; }
    std::shared_ptr<CohortMaps> getCohortMaps()  { return cohortmaps; }

    // set new Terrain core data; assumes newTerr has internal state set up
    void setNewTerrainData(std::unique_ptr<Terrain> newTerr, Terrain *master)
    {
        terrain = std::move(newTerr);

        terrain->calcMeanHeight();

        // match dimensions for empty overlay
        int dx, dy;
        terrain->getGridDim(dx, dy);
        getTypeMap(TypeMapType::TRANSECT)->matchDim(dy, dx);
        getTypeMap(TypeMapType::TRANSECT)->fill(1);
        getTypeMap(TypeMapType::EMPTY)->matchDim(dy, dx);
        getTypeMap(TypeMapType::EMPTY)->clear();

        masterTerrain = master;
    }

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
     * @param timestepIDs   list of simulation timestamps in ascending order
     * @param shareCohorts  if this is true, then cohorts are shared
     * @param cohorts       the cohrt shared_ptr to be used if sharing is selected
     */
    void loadScene(std::string dirprefix, std::vector<int> timestepIDs, bool shareCohorts, std::shared_ptr<CohortMaps> cohorts);
    void loadScene(std::vector<int> timestepIDs, bool shareCohorts, std::shared_ptr<CohortMaps> cohorts);

    /**
     * @brief loadDataMaps Load all data maps for texturing from file
     * @param totRegion Region covered by the full terrain
     * @param total number of simulation timesteps
     */
    void loadDataMaps(int timesteps);

     /**
      * Export the scene (for Mitsuba) to the XML specified
      * @param speciesMap      Correspondence map between the plant type and a vector binding a height to a mitsuba id
      * @param xmlFile         XML file in which the scene will be exported
      * @param transect        Transect control in case the export concerns only the transect view, nullptr instead
      */
     void exportSceneXml(map<string, vector<MitsubaModel>>& speciesMap, ofstream& xmlFile, Transect * transect = nullptr);

     /**
      * Export the scene (for Mitsuba) to the JSON specified
      * @param speciesMap      Correspondence map between the plant type and a vector binding a height to a mitsuba id
      * @param xmlFile         XML file in which the scene will be exported
      * @param transect        Transect control in case the export concerns only the transect view, nullptr instead
      */
     void exportInstancesJSON(map<string, vector<MitsubaModel>>& speciesMap, const string urlInstances, const string nameInstances, Scene* scene, Transect* transect = nullptr);

     /**
			* @brief expoert the scene parameters to the JSON specified
      * @param jsonFile 
      * @param cameraName 
      * @param lightsName 
      * @param terrainName 
      * @param instancesName 
      * @param sceneName 
      * @param resX 
      * @param resY 
      * @param quality 
      * @param threads 
      */
     void exportSceneJSON(const string jsonDirPath, const string cameraName, const string lightsName, const string terrainName, const string instancesName, const string sceneName, const int resX, const int resY, const int quality, const int threads);

     /**
      * Export the terrain (for Mitsuba) to the JSON specified and OBJ File
      * @param xmlFile         XML file in which the scene will be exported
      * @param transect        Transect control in case the export concerns only the transect view, nullptr instead
      */
     void exportTerrainJSON(const string terrainURL, const string terrainName, Transect* transect = nullptr);

     /**
			* @brief compute the slope of the terrain and export it to a texture
      * @param URL 
      * @param slopeMin 
      * @param slopeMax 
      */
     void exportTextureSlope(const string URL, float slopeMin, float slopeMax);
};

#endif // SCENE_H
