#ifndef COHORTSAMPLER_H
#define COHORTSAMPLER_H

#include <vector>
#include <sstream>
#include <random>
#include "../../common/basic_types.h"

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
        cohortsampler(float width, float height, int gw, int gh, int maxpercell, int samplemult);

        std::vector<basic_tree> sample_all(bool soft);
        std::deque<int> gen_poisson_list(int sqsize, int nplants, std::deque<int> *dists, std::default_random_engine &gen);

        std::vector<basic_tree> sample(ValueMap<std::vector<data_importer::ilanddata::cohort> > &cohortmap);
        void fix_cohortmaps(std::vector<ValueMap<std::vector<data_importer::ilanddata::cohort> > > &cohortmaps);
private:
        std::vector<basic_tree> sample_one_soft(data_importer::ilanddata::cohort chrt, std::default_random_engine &gen);
        std::vector<basic_tree> sample_one_hard(data_importer::ilanddata::cohort chrt, std::default_random_engine &gen);

        ValueGridMap<int> tileidxes;

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
