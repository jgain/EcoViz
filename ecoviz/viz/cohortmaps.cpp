#include "data_importer/data_importer.h"
#include "cohortmaps.h"
#include <random>
#include <chrono>

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
    int timestep_range = max_timestep - min_timestep + 1;

    timestep_indices.resize(timestep_range, 0);

    for (int ts : timesteps)
    {
        timestep_indices.at(ts - min_timestep) = 1;
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

        int idx = timestep_indices.at(fdata.timestep - min_timestep);
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
            if (middle.x >= rw - 5.0f || middle.y >= rh - 5.0f)
                continue;
            //if (crt.nplants > 0.0f)
            //    printf("cohort with %f plants being added at mid location %f, %f\n", crt.nplants, middle.x, middle.y);
            map.get_fromreal(middle.x, middle.y).push_back(crt);
        }
    }

    //fix_cohortmaps();

}

void CohortMaps::do_adjustments(int times)
{
    for (int i = 0; i < times; i++)
    {
        determine_actionmap();
        apply_actionmap();
    }

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
                if (basepos + neg < 0)
                    break;
                int size = xdir ? refmap.get(cx + neg, cy).size() : refmap.get(cx, cy + neg).size();
                if (size > 0)
                    break;
            }
            int pos = 1;
            int poshigh = xdir ? std::min(extent, gw - cx) : std::min(extent, gh - cy);
            for (; pos < poshigh; pos++)
            {
                if (basepos + pos >= (xdir ? gw : gh))
                    break;
                int size = xdir ? refmap.get(cx + pos, cy).size() : refmap.get(cx, cy + pos).size();
                if (size > 0)
                    break;
            }
            if (unif(gen) < (pos - neg) * unitp)
            {
                int nresize_pos = ceil(abs(normd(gen)) * pos * scale + pos * base);
                nresize_pos = std::min(poshigh, nresize_pos);
                if (pos == 1)
                    nresize_pos = 1;
                int nresize_neg = ceil(abs(normd(gen)) * neg * scale + neg * base);
                nresize_neg = std::max(neghigh, nresize_neg);
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

                        if (basestart < 0.0f)
                            basestart = 0.0f;
                        if (baseend >= (xdir ? rw : rh))
                            baseend = (xdir ? rw : rh) - 1e-2f;
                    }
                }
            }
        }
    }
}

void CohortMaps::determine_actionmap()
{
    if (timestep_maps.size() == 0)
        return;

    std::default_random_engine gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> unif;

    auto in_bound = [](int gw, int gh, int x, int y){
        return x < gw && x >= 0 && y < gh && y >= 0;
    };

    auto determine_action = [this, &in_bound, &unif, &gen](int x, int y, ValueGridMap<std::vector< data_importer::ilanddata::cohort > > &m)
    {
        std::vector<std::pair<int, int> > dirs;
        for (int cy = y - 1; cy <= y + 1; cy++)
        {
            for (int cx = x - 1; cx <= x + 1; cx++)
            {
                int xdiff = cx - x;
                int ydiff = cy - y;
                if ((cy == y && cx == x) || (cy != y && cx != x) || !in_bound(gw, gh, cx, cy) || actionmap.get(cx, cy).dir != DonateDir::NONE)
                    continue;
                dirs.push_back({xdiff, ydiff});
            }
        }
        std::random_shuffle(dirs.begin(), dirs.end());

        for (auto &dxdy : dirs)
        {
            int xdiff = dxdy.first;
            int ydiff = dxdy.second;
            int cy = y + ydiff;
            int cx = x + xdiff;

            auto &thisc = m.get(x, y);
            auto &otherc = m.get(cx, cy);
            for (auto citer = thisc.begin(); citer != thisc.end(); advance(citer, 1))
            {
                if (unif(gen) > 0.5f)
                    continue;
                int specidx = citer->specidx;
                //if (citer->nplants > 2.0f)
                //    continue;
                auto iter = std::find_if(otherc.begin(), otherc.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
                //if (iter == otherc.end())
                if (otherc.size() == 0)
                {
                    /*
                    if (otherc.size() > 0)
                    {
                        auto remiter = std::next(otherc.end(), -1);		// this is the cohort we will exchange with that of the donor's
                        auto temp = *citer;		// make a backup and use it instead of iterator directly, since the push_backs below might invalidate citer
                        thisc.erase(citer);		// we erase first, since iterator might be invalidated when we push_back below
                        thisc.push_back(*remiter);
                        otherc.erase(remiter);
                        otherc.push_back(temp);
                    }
                    */
                    int giveback_idx = thisc.size() > 0 ? thisc.back().specidx : -1;
                    DonateDir dir;
                    DonateDir opdir;
                    if (xdiff > 0)
                    {
                        dir = DonateDir::EAST;
                        opdir = DonateDir::WEST;
                    }
                    else if (xdiff < 0)
                    {
                        dir = DonateDir::WEST;
                        opdir = DonateDir::EAST;
                    }
                    else if (ydiff > 0)
                    {
                        dir = DonateDir::SOUTH;
                        opdir = DonateDir::NORTH;
                    }
                    else if (ydiff < 0)
                    {
                        dir = DonateDir::NORTH;
                        opdir = DonateDir::SOUTH;
                    }
                    DonateAction action_donor = {dir, specidx};
                    //DonateAction action_rec = {opdir, giveback_idx};
                    DonateAction action_rec = {DonateDir::RECEIVE, giveback_idx};
                    this->actionmap.get(x, y) = action_donor;
                    this->actionmap.get(cx, cy) = action_rec;
                    break;
                }
            }
        }
    };

    actionmap.setDim(timestep_maps.at(0));
    actionmap.setDimReal(timestep_maps.at(0));
    actionmap.fill({DonateDir::NONE, -1});

    for (auto &m : timestep_maps)
    {
        int gw, gh;
        m.getDim(gw, gh);
        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                auto action = actionmap.get(x, y);
                if (action.dir == DonateDir::NONE)
                {
                    if (unif(gen) < 1.0f)
                        determine_action(x, y, m);
                }
            }
        }
    }
}

void CohortMaps::apply_actionmap()
{
    if (timestep_maps.size() == 0)
        return;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    int movecount = 0;

    auto move_cohort = [&movecount](std::vector<ilanddata::cohort> &cvec_origin, std::vector<ilanddata::cohort> &cvec_dest, int specidx)
    {
        auto iter = std::find_if(cvec_origin.begin(), cvec_origin.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
        if (iter != cvec_origin.end())
        {
            cvec_dest.push_back(*iter);
            cvec_origin.erase(iter);
            if (cvec_dest.size() == 1)
                movecount++;
        }
    };


    for (auto &m : timestep_maps)
    {
        int gw, gh;
        m.getDim(gw, gh);
        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                auto &cellcohorts = m.get(x, y);
                auto action = actionmap.get(x, y);
                if (action.dir == DonateDir::NONE)
                    continue;
                switch (action.dir)
                {
                    case DonateDir::NORTH:
                        if (y > 0)
                            move_cohort(m.get(x, y), m.get(x, y - 1), action.specidx);
                        break;
                    case DonateDir::WEST:
                        if (x > 0)
                            move_cohort(m.get(x, y), m.get(x - 1, y), action.specidx);
                        break;
                    case DonateDir::SOUTH:
                        if (y < gh - 1)
                            move_cohort(m.get(x, y), m.get(x, y + 1), action.specidx);
                        break;
                    case DonateDir::EAST:
                        if (x < gw - 1)
                            move_cohort(m.get(x, y), m.get(x + 1, y), action.specidx);
                        break;
                }
            }
        }
    }
    std::cout << movecount << " cohorts moved to empty tiles" << std::endl;
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
