#ifndef COHORTSAMPLER_H
#define COHORTSAMPLER_H

#include <vector>
#include <sstream>
#include <random>
#include "../../common/basic_types.h"
#include "cohortmaps.h"

namespace data_importer
{
        namespace ilanddata
        {
                struct cohort;
        }
}

class cohortsampler
{
public:
        cohortsampler(float tw, float th, float rw, float rh, float xoff, float yoff, int maxpercell, int samplemult);

        std::vector<basic_tree> sample_all(bool soft);
        std::deque<int> gen_poisson_list(int sqsize, int nplants, std::deque<int> *dists, std::default_random_engine &gen);

        std::vector<basic_tree> sample(const ValueGridMap<std::vector<data_importer::ilanddata::cohort> > &cohortmap, std::vector<std::vector<basic_tree> > *allcells_trees);
        void fix_cohortmaps(std::vector<ValueMap<std::vector<data_importer::ilanddata::cohort> > > &cohortmaps);
        void set_spectoidx_map(std::unique_ptr<ValueGridMap<std::vector<int> > > spectoidx_map_ptr);
private:
        std::vector<basic_tree> sample_one_soft(data_importer::ilanddata::cohort chrt, std::default_random_engine &gen);
        std::vector<basic_tree> sample_one_hard(data_importer::ilanddata::cohort chrt, std::default_random_engine &gen);

        ValueGridMap<int> tileidxes;
        std::unique_ptr<ValueGridMap<std::vector<int> > > spectoidx_map;

        void generate_tiles(int ntiles, int tilesize);

        std::default_random_engine gen;
        std::uniform_real_distribution<float> unif;
        std::vector< std::deque<int> > randtiles;
        float width, height;
        float quantdiv, radialmult;
        int maxpercell;
        int samplemult;
        int gw, gh;
};

#endif // COHORTSAMPLER_H
