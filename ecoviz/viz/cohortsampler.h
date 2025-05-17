/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  K. Kapp
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

#ifndef COHORTSAMPLER_H
#define COHORTSAMPLER_H

#include <vector>
#include <sstream>
#include <random>
#include "../../common/basic_types.h"
#include "cohortmaps.h"
#include <deque>

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
