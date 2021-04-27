#ifndef COHORTMAPS
#define COHORTMAPS

#include "data_importer/data_importer.h"
#include "common/basic_types.h"

#include <memory>

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
    CohortMaps(const std::vector<std::string> &filenames, float rw, float rh, std::string minversion);

    void fix_cohortmaps();

    int get_nmaps();
    void get_grid_dims(int &gw, int &gh);
    const ValueGridMap<std::vector<data_importer::ilanddata::cohort> > &get_map(int timestep_idx) const;
    void get_cohort_dims(float &w, float &h);
    void do_adjustments(int times);
    ValueGridMap<CohortMaps::DonateDir> get_actionmap_actions(int gw, int gh, float rw, float rh);
    ValueGridMap<float> get_actionmap_floats(int gw, int gh, float rw, float rh);
    ValueGridMap<CohortMaps::DonateAction> get_actionmap();

    void compute_specset_map();
    std::unique_ptr<ValueGridMap<std::set<int> > > move_specset_map();
    std::unique_ptr<ValueGridMap<std::vector<int> > > compute_spectoidx_map();
    void move_cohort(std::vector<data_importer::ilanddata::cohort> &destvec, std::vector<data_importer::ilanddata::cohort> &srcvec, std::vector<data_importer::ilanddata::cohort>::iterator &srciter, float xmod, float ymod);
    void undo_actionmap();
private:
    void apply_actionmap();
    void determine_actionmap();

    std::vector<ValueGridMap<std::vector< data_importer::ilanddata::cohort > > > timestep_maps;
    std::vector<ValueGridMap<int> > plantcountmaps;
    ValueGridMap<DonateAction> actionmap;
    std::unique_ptr<ValueGridMap<std::set<int> > > specset_map;

    float rw, rh;
    int gw, gh;
    float dx, dy;
    int maxpercell = 10;
};

#endif
