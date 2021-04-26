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

    //std::default_random_engine gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    auto in_bound = [](int gw, int gh, int x, int y){
        return x < gw && x >= 0 && y < gh && y >= 0;
    };

    auto determine_action = [this, &in_bound, &unif, &gen](int x, int y, ValueGridMap<std::vector< data_importer::ilanddata::cohort > > &m)
    {
        int distance = unif(gen) * 4 + 1;		// [1, 4] inclusive
        std::vector<std::pair<int, int> > dirs;
        for (int cy = y - distance; cy <= y + distance; cy += distance)
        {
            for (int cx = x - distance; cx <= x + distance; cx += distance)
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
            bool done = false;
            int xdiff = dxdy.first;
            int ydiff = dxdy.second;
            int cy = y + ydiff;
            int cx = x + xdiff;
            int distance = std::max(abs(xdiff), abs(ydiff));

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
                if (otherc.size() == 0)		// REMOVEME: We should also be able to send cohorts to non-empty tiles
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
                    DonateAction action_donor = {dir, specidx, distance};
                    //DonateAction action_rec = {opdir, giveback_idx};
                    DonateAction action_rec = {DonateDir::RECEIVE, -1, distance};
                    this->actionmap.get(x, y) = action_donor;
                    this->actionmap.get(cx, cy) = action_rec;
                    done = true;
                    break;
                }
            }
            if (done) break;
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
                    if (unif(gen) < 0.5f)
                        determine_action(x, y, m);
                }
            }
        }
        //break;		// REMOVEME
    }
}

void CohortMaps::compute_specset_map()
{
    specset_map = std::unique_ptr<ValueGridMap<std::set<int> > >(new ValueGridMap<std::set<int> >(gw, gh, rw, rh, 1.0f, 1.0f));

    for (auto &ts: timestep_maps)
    {
        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                for (const ilanddata::cohort &c : ts.get(x, y))
                {
                    specset_map->get(x, y).insert(c.specidx);
                }
            }
        }
    }
}

std::unique_ptr<ValueGridMap<std::vector<int> > > CohortMaps::compute_spectoidx_map()
{
    if (!specset_map)
        compute_specset_map();

    std::unique_ptr<ValueGridMap<std::vector<int> > > spectoidx_map = std::unique_ptr<ValueGridMap<std::vector<int> > >(new ValueGridMap<std::vector<int> >());
    spectoidx_map->setDim(*specset_map);
    spectoidx_map->setDimReal(*specset_map);
    spectoidx_map->setOffsets(*specset_map);

    for (int y = 0; y < gh; y++)
    {
        for (int x = 0; x < gw; x++)
        {
            auto &set = specset_map->get(x, y);
            std::vector<int> toidx;
            int idx = 0;
            for (auto &id : set)
            {
                if (id + 1 > toidx.size())
                {
                    toidx.resize(id + 1, -1);
                }
                toidx[id] = idx;
                idx++;
            }
            spectoidx_map->set(x, y, toidx);
        }
    }

    return std::move(spectoidx_map);

}

std::unique_ptr<ValueGridMap<std::set<int> > > CohortMaps::move_specset_map()
{
    //std::unique_ptr<ValueGridMap<std::set<int> > > ptr(specset_map);

    if (!specset_map)
        throw std::logic_error("specset_map not computed yet");

    return std::move(specset_map);

    //return ptr;
}

ValueGridMap<CohortMaps::DonateDir> CohortMaps::get_actionmap_actions(int gw, int gh, float rw, float rh)
{
    ValueGridMap<CohortMaps::DonateDir> map;
    map.setDim(gw, gh);
    map.setDimReal(rw, rh);

    float cw = rw / gw;
    float ch = rh / gh;

    for (int y = 0; y < gh; y++)
    {
        for (int x = 0; x < gw; x++)
        {
            float rx = x * cw + cw / 2.0f;
            float ry = y * ch + ch / 2.0f;
            if (actionmap.in_landscape(rx, ry))
                map.set_fromreal(rx, ry, actionmap.get_fromreal(rx, ry).dir);
        }
    }
    return map;
}

ValueGridMap<float> CohortMaps::get_actionmap_floats(int gw, int gh, float rw, float rh)
{
    auto tempmap = get_actionmap_actions(gw, gh, rw, rh);
    ValueGridMap<float> map;
    map.setDim(tempmap);
    map.setDimReal(tempmap);

    for (int y = 0; y < gh; y++)
    {
        for (int x = 0; x < gw; x++)
        {
            switch (tempmap.get(x, y))
            {
                case DonateDir::NORTH:
                case DonateDir::EAST:
                case DonateDir::SOUTH:
                case DonateDir::WEST:
                    map.set(y, x, 2.0);
                    break;
                case DonateDir::RECEIVE:
                    map.set(y, x, 1.0f);
                    break;
                default:
                    map.set(y, x, 0.0f);
            }
        }
    }
    return map;
}

ValueGridMap<CohortMaps::DonateAction> CohortMaps::get_actionmap()
{
    return actionmap;
}

void CohortMaps::apply_actionmap()
{
    if (timestep_maps.size() == 0)
        return;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    int movecount = 0;

    auto move_cohort = [&movecount, this](std::vector<ilanddata::cohort> &cvec_origin, std::vector<ilanddata::cohort> &cvec_dest, int specidx, float xmod, float ymod)
    {
        if (specidx < 0)
            return;
        auto iter = std::find_if(cvec_origin.begin(), cvec_origin.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
        if (iter != cvec_origin.end())
        {
            cvec_dest.push_back(*iter);
            cvec_dest.back().xs += xmod;
            cvec_dest.back().xe += xmod;
            cvec_dest.back().ys += ymod;
            cvec_dest.back().ye += ymod;

            ilanddata::cohort &c = cvec_dest.back();

            if (c.ys < 0.0f) c.ys = 1e-3f;
            if (c.ye >= rw) c.ye = rw - 1e-3f;
            if (c.xs < 0.0f) c.xs = 1e-3f;
            if (c.xe >= rh) c.xe = rh - 1e-3f;

            cvec_origin.erase(iter);
            if (cvec_dest.size() == 1)
            {
                movecount++;
            }
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
                int d = action.distance;
                if (action.dir == DonateDir::NONE)
                    continue;
                switch (action.dir)
                {
                    case DonateDir::NORTH:
                        if (y > d)
                            move_cohort(m.get(x, y), m.get(x, y - d), action.specidx, 0.0f, -2.0f * action.distance);
                        break;
                    case DonateDir::WEST:
                        if (x > d)
                            move_cohort(m.get(x, y), m.get(x - d, y), action.specidx, -2.0f * action.distance, 0.0f);
                        break;
                    case DonateDir::SOUTH:
                        if (y < gh - d)
                            move_cohort(m.get(x, y), m.get(x, y + d), action.specidx, 0.0f, 2.0f * action.distance);
                        break;
                    case DonateDir::EAST:
                        if (x < gw - d)
                            move_cohort(m.get(x, y), m.get(x + d, y), action.specidx, 2.0f * action.distance, 0.0f);
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
