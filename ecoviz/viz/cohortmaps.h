#ifndef COHORTMAPS
#define COHORTMAPS

#include "data_importer/data_importer.h"
#include "common/basic_types.h"

#include <memory>
#include <functional>

class CohortMaps
{
public:
    enum class DonateDir
    {
        NORTH,
        WEST,
        SOUTH,
        EAST,
        RECEIVE,
        NONE
    };

    struct DonateAction
    {
        DonateDir dir;
        int specidx;
        int distance;
    };

public:
    CohortMaps(const std::vector<std::string> &filenames, float rw, float rh, std::string minversion, const std::map<std::string, int> &species_lookup);

    void fix_cohortmaps();

    int get_nmaps();
    void get_grid_dims(int &gw, int &gh);
    const ValueGridMap<std::vector<data_importer::ilanddata::cohort> > &get_map(int timestep_idx) const;
    void get_cohort_dims(float &w, float &h);
    void do_adjustments(int max_distance);
    ValueGridMap<CohortMaps::DonateDir> get_actionmap_actions(int gw, int gh, float rw, float rh);
    ValueGridMap<float> get_actionmap_floats(int gw, int gh, float rw, float rh);
    ValueGridMap<CohortMaps::DonateAction> get_actionmap();
    const std::vector<basic_tree> &get_maturetrees(int timestep_idx) const; // get mature trees for timestep t
    void getCohortLoc(long &lx, long &ly){ lx = locx; ly = locy; }

    void compute_specset_map();
    std::unique_ptr<ValueGridMap<std::set<int> > > move_specset_map();
    std::unique_ptr<ValueGridMap<std::vector<int> > > compute_spectoidx_map();
    void move_cohort(std::vector<data_importer::ilanddata::cohort> &destvec, std::vector<data_importer::ilanddata::cohort> &srcvec, std::vector<data_importer::ilanddata::cohort>::iterator &srciter, float xmod, float ymod);
    void undo_actionmap();

    void set_progress_function(std::function<void(int)> func);
    void set_progress_label_function(std::function<void(std::string)> func);
    int determine_cohort_startidxes();
    int get_maxpercell();
    void set_nplants_each();
private:
    void apply_actionmap();
    void determine_actionmap(int max_distance);

    std::vector<ValueGridMap<std::vector< data_importer::ilanddata::cohort > > > timestep_maps;
    std::vector<ValueGridMap<int> > plantcountmaps;
    ValueGridMap<DonateAction> actionmap;
    std::unique_ptr<ValueGridMap<std::set<int> > > specset_map;
    std::vector<std::vector<basic_tree>> timestep_mature; // mature trees at each timestep

    float rw, rh;
    int gw, gh;
    float dx, dy;
    long locx, locy; // global corner location

    bool action_applied = false;

    std::function<void (int) > progress_function;
    std::function<void (std::string) > progress_label_function;

    int maxpercell;
    int nplant_div;
    int maxpercohort;
};

#endif
