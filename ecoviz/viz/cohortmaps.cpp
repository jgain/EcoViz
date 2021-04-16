#include "data_importer/data_importer.h"
#include "cohortmaps.h"
#include <random>

#define TIMESTEP_ONLY true
#define ALL_FILEDATA false

using namespace data_importer;

CohortMaps::CohortMaps(const std::vector<std::string> &filenames, float rw, float rh, std::string minversion)
    : rw(rw), rh(rh), gw(-1), gh(-1), dx(-1.0f), dy(-1.0f)
{
    std::vector<int> timesteps;
    std::vector<int> timestep_indices;

    for (auto &fname : filenames)
    {
        int timestep = ilanddata::read(fname, minversion, TIMESTEP_ONLY).timestep;
        timesteps.push_back(timestep);
    }

    int min_timestep = *std::min_element(timesteps.begin(), timesteps.end());
    int max_timestep = *std::max_element(timesteps.begin(), timesteps.end());
    int timestep_range = max_timestep - min_timestep;

    timestep_indices.resize(timestep_range, 0);

    for (int ts : timesteps)
    {
        timestep_indices.at(ts) = 1;
    }

    int prefix_sum = 0;
    for (auto &idx : timestep_indices)
    {
        if (idx == 1)
        {
            idx = prefix_sum;
            prefix_sum++;
        }
    }


    timestep_maps.resize(filenames.size());
    for (auto &fname : filenames)
    {
        auto fdata = ilanddata::read(fname, minversion, ALL_FILEDATA);
        if (dx < 0.0f || dy < 0.0f)
        {
            dx = fdata.dx;
            dy = fdata.dy;
        }
        else
        {
            if (fabs(fdata.dx - dx) > 1e-5f || fabs(fdata.dy - dy) > 1e-5f)
                throw std::invalid_argument("All cohorts must have the same dimentions");
        }

        // XXX: it might be useful later on to allow each cohort map to have its own size, offset, etc. So just keeping this here for now, commented
        //float thisrw = fdata.maxx - fdata.minx;
        //float thisrh = fdata.maxy - fdata.miny;

        int idx = timestep_indices.at(fdata.timestep);
        timestep_maps.at(idx) = ValueGridMap<std::vector<ilanddata::cohort > >(fdata.dx, fdata.dy, rw, rh, 1.0f, 1.0f);
        auto &map = timestep_maps.at(idx);
        if (gw == -1 || gh == -1)
            map.getDim(gw, gh);
        else
        {
            int checkw, checkh;
            map.getDim(checkw, checkh);
            if (checkw != gw || checkh != gh)
                throw std::logic_error("Grid widths and heights must be the same in CohortMaps constructor");
        }

        for (ilanddata::cohort &crt : fdata.cohorts)
        {
            xy<float> middle = crt.get_middle();
            map.get_fromreal(middle.x, middle.y).push_back(crt);
        }
    }

    fix_cohortmaps();
}

void CohortMaps::fix_cohortmaps()
{
    using namespace data_importer::ilanddata;

    int maxpercell = 10;
    float base = 0.2f;
    float scale = (1.0f - base) / 3.0f;
    int extent = 20;
    float unitp = 0.2f;
    int placediv = 10;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;
    std::normal_distribution<float> normd;
    for (int cidx = 0; cidx < gw * gh; cidx++)
    {
        //std::cout << "Cell index: " << cidx << std::endl;
        int cx = cidx % gw;
        int cy = cidx / gw;
        float nsimplants = 0;
        int mapidx = 0;
        for (auto &cohortmap : timestep_maps)
        {
            std::vector<cohort> &crts = cohortmap.get(cx, cy);
            nsimplants = std::accumulate(crts.begin(), crts.end(), 0, [](int value, const cohort &c1) { return value + c1.nplants; });
            nsimplants = std::min(float(maxpercell), nsimplants / placediv);
            if (nsimplants > 0)
                break;
            mapidx++;
        }

        if (nsimplants > 0)
        {
            auto &refmap = timestep_maps.at(mapidx);
            auto &crts = refmap.get(cx, cy);
            xy<float> middle = crts.front().get_middle();
            bool xdir;
            if (unif(gen) < 0.5f)
            {
                xdir = true;
            }
            else
            {
                xdir = false;
            }
            int neg = -1;
            int neghigh = xdir ? std::max(-extent, -cx) : std::max(-extent, -cy);
            int basepos = xdir ? cx : cy;
            for (; neg > neghigh; neg--)
            {
                int size = xdir ? refmap.get(cx + neg, cy).size() : refmap.get(cx, cy + neg).size();
                if (basepos + neg >= gw || size > 0)
                    break;
            }
            int pos = 1;
            for (; pos < extent; pos++)
            {
                int size = xdir ? refmap.get(cx + pos, cy).size() : refmap.get(cx, cy + pos).size();
                if (basepos + pos >= gw || size > 0)
                    break;
            }
            if (unif(gen) < (pos - neg) * unitp)
            {
                int nresize_pos = ceil(abs(normd(gen)) * pos * scale + pos * base);
                nresize_pos = std::min(extent, nresize_pos);
                if (pos == 1)
                    nresize_pos = 1;
                int nresize_neg = ceil(abs(normd(gen)) * neg * scale + neg * base);
                nresize_neg = std::max(-extent, nresize_neg);
                if (neg == -1)
                    nresize_neg = -1;
                for (auto &currmap : timestep_maps)
                {
                    auto &currcrts = currmap.get_fromreal(middle.x, middle.y);
                    for (auto &c : currcrts)
                    {
                        auto &basestart = xdir ? c.xs : c.ys;
                        auto &baseend = xdir ? c.xe : c.ye;
                        basestart += (nresize_neg + 1) * 2;
                        baseend = baseend + (nresize_pos - nresize_neg) * 2;
                    }
                }
            }
        }
    }
}

int CohortMaps::get_nmaps()
{
    return timestep_maps.size();
}

void CohortMaps::get_grid_dims(int &gw, int &gh)
{
    gw = this->gw;
    gh = this->gh;
}

void CohortMaps::get_cohort_dims(float &w, float &h)
{
    w = this->dx;
    h = this->dy;
}

const ValueGridMap<std::vector<ilanddata::cohort> > &CohortMaps::get_map(int timestep_idx) const
{
    return timestep_maps.at(timestep_idx);
}
