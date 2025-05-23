#include "data_importer/data_importer.h"
#include "cohortmaps.h"
#include <random>
#include <chrono>
#include <numeric>

#define TIMESTEP_ONLY true
#define ALL_FILEDATA false

using namespace data_importer;

CohortMaps::CohortMaps(const std::vector<std::string> &filenames, float rw, float rh, std::string minversion, const std::map<std::string, int> &species_lookup)
    : rw(rw), rh(rh), gw(-1), gh(-1), dx(-1.0f), dy(-1.0f), nplant_div(1), maxpercohort(10)
{
    std::vector<int> timesteps;
    std::vector<int> timestep_indices;

    for (auto &fname : filenames)
    {
        bool binFileRead = false;
        if (fname.rfind(".pdbb") != std::string::npos)
            binFileRead = true;

        int timestep =  (binFileRead == false ? ilanddata::read(fname, minversion,  species_lookup, TIMESTEP_ONLY).timestep :
                                                ilanddata::readbinary(fname, minversion,  species_lookup, TIMESTEP_ONLY).timestep);

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
    timestep_mature.resize(filenames.size());
    for (auto &fname : filenames)
    {
        bool binFileRead = false;
        if (fname.rfind(".pdbb") != std::string::npos)
            binFileRead = true;

          std::cerr << "CohortMaps: A" << std::endl;

        auto fdata = (binFileRead == false ?  ilanddata::read(fname, minversion, species_lookup, ALL_FILEDATA) :
                                              ilanddata::readbinary(fname, minversion, species_lookup, ALL_FILEDATA));
          std::cerr << "CohortMaps: B" << std::endl;
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
        locx = fdata.locx; locy = fdata.locy;

        int maxx = 0, maxy = 0;
        for(auto &tree: fdata.trees)
        {
            if(tree.x > maxx)
                maxx = tree.x;
            if(tree.y > maxy)
                maxy = tree.y;
            timestep_mature.at(idx).push_back(tree);
        }
        //std::cerr << "Max tree placement = " << maxx << ", " << maxy << std::endl;
        timestep_maps.at(idx) = ValueGridMap<std::vector<ilanddata::cohort > >(fdata.dx, fdata.dy, rw, rh, 1.0f, 1.0f);
        auto &map = timestep_maps.at(idx);
        if (gw < 0 || gh < 0) // in case no cohorts are loaded
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
            try {
            map.get_fromreal(middle.x, middle.y).push_back(crt);
            } catch (const std::exception &e) { std::cerr << e.what(); }
        }
    }

    set_nplants_each();

    maxpercell = determine_cohort_startidxes();

    actionmap.setDim(timestep_maps.at(0));
    actionmap.setDimReal(timestep_maps.at(0));
    actionmap.fill({DonateDir::NONE, -1, 0});

    //fix_cohortmaps();
    std::cerr << "CohortMaps: END" << std::endl;


}

int CohortMaps::get_maxpercell()
{
    return maxpercell;
}

void CohortMaps::set_nplants_each()
{
    auto set_nplants_crtlist = [this](std::vector<ilanddata::cohort> &crts) {
        for (auto &c : crts)
            c.nplants = std::min(double(maxpercohort), double(ceil(c.nplants / nplant_div))) + 1e-3f;
    };

    for (auto &m : timestep_maps)
    {
        int gw, gh;
        m.getDim(gw, gh);
        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                set_nplants_crtlist(m.get(x, y));
            }
        }
    }
}

int CohortMaps::determine_cohort_startidxes()
{
    if (timestep_maps.size() == 0)
        return 0;

    int maxpercell = 100000;		// just make this a big number, since we don't impose a limit on the maximum index for now. We determine it here

    unsigned char maxidx = 0;

    auto assign_newcohorts = [&maxidx](std::vector<ilanddata::cohort> &crts) {
        unsigned char currindex = 0;
        for (ilanddata::cohort &c : crts)
        {
            c.startidx = currindex;
            currindex += (unsigned char)(int(c.nplants));
            if (currindex > maxidx) maxidx = currindex;
        }
    };

    for (int y = 0; y < gh; y++)
    {
        for (int x = 0; x < gw; x++)
        {
            auto &crts = timestep_maps.at(0).get(x, y);
            std::sort(crts.begin(), crts.end(), [](ilanddata::cohort &c1, ilanddata::cohort &c2) { if (c1.specidx < c2.specidx) return true; else if (c1.specidx == c2.specidx) return c1.height > c2.height; else return false; });
            assign_newcohorts(crts);

            for (int i = 1; i < timestep_maps.size(); i++)
            {

                auto &crts2 = timestep_maps.at(i).get(x, y);
                auto &crts1 = timestep_maps.at(i - 1).get(x, y);

                if (crts2.size() == 0)
                    continue;

                std::sort(crts2.begin(), crts2.end(), [](ilanddata::cohort &c1, ilanddata::cohort &c2) { if (c1.specidx < c2.specidx) return true; else if (c1.specidx == c2.specidx) return c1.height > c2.height; else return false; });

                if (crts1.size() == 0)
                {
                    assign_newcohorts(crts2);
                    auto iter = std::find_if(crts2.begin(), crts2.end(), [](const ilanddata::cohort &crt) { return crt.startidx >= 250; });
                    if (iter != crts2.end())
                    {
                        throw std::logic_error("Mistake");
                    }
                    continue;
                }

                std::vector<ilanddata::cohort *> unassigned;
                std::vector<std::pair<int, int> > edges;

                // TODO: what if a cohort is new? what if we cannot find a spot to fit it into?

                int prev_idx = -1;
                int skip = 0;
                for (auto &c2 : crts2)
                {
                    c2.startidx = 255;
                    if (c2.specidx == prev_idx)
                    {
                        skip++;
                    }
                    else
                    {
                        prev_idx = c2.specidx;
                        skip = 0;
                    }
                    int idx = 0;
                    // XXX: loop below can be optimized, since we sorted the cohorts beforehand according to species index
                    for (auto &c1 : crts1)
                    {
                        if (c1.specidx == c2.specidx)
                        {
                            if (idx < skip)
                            {
                                idx++;
                                continue;
                            }
                            if (c2.height >= c1.height)
                            {
                                c2.startidx = c1.startidx;
                                edges.push_back({c2.startidx, int(c2.startidx) + int(round(c2.nplants) + 1e-3f) - 1});
                                int backedge = edges.back().second;
                                break;
                            }
                            else
                                continue;
                        }
                    }
                    if (c2.startidx == 255)
                        unassigned.push_back(&c2);
                }
                if (edges.size() == 0)		// if edges size is zero, then it means we could not find any matching species from previous cohort, so we create new index borders
                {
                    assign_newcohorts(crts2);
                    continue;
                }

                // sort edges of point indices list where different cohorts will be placed
                std::sort(edges.begin(), edges.end(), [](const std::pair<int, int> &p1, const std::pair<int, int> &p2) { return p1.first < p2.first; });
                std::vector<std::pair<int, int> > openslots;
                std::pair<int, int> dummyfirst = {0, 0};
                std::pair<int, int> *lastpair = &dummyfirst;		// last pair of slots, so that we can get the first index of the current one we are lookng at in loop below

                // check each consecutive pair of slots to see if there is an open slot between them, and add it if so
                for (auto &e : edges)
                {
                    // if there is a gap between the first index of the current slot and the last index of a previous one, add an open slot
                    if (e.first > lastpair->second + 1)
                        openslots.push_back({lastpair->second + 1, e.first - 1});
                    lastpair = &e;
                }
                if (edges.back().second < maxpercell - 1)
                {
                    openslots.push_back({edges.back().second + 1, maxpercell - 1});
                }

                /*
                std::cout << "Edges: ";
                for (auto &e : edges)
                    std::cout << "(" << e.first << ", " << e.second << ") ";
                std::cout << std::endl;
                */

                // now, for all cohorts that we still have to find a slot for, we go throught all open slots and try to find one large enough to accommodate it
                for (auto iter = unassigned.begin(); iter != unassigned.end(); advance(iter, 1))
                {
                    bool foundslot = false;
                    ilanddata::cohort *unas = *iter;
                    int nplants = int(round(unas->nplants) + 1e-3f);
                    for (auto slotiter = openslots.begin(); slotiter != openslots.end(); advance(slotiter, 1))
                    {
                        auto &slot = *slotiter;
                        int slotsize = slot.second - slot.first + 1;
                        if (nplants <= slotsize)
                        {
                            unas->startidx = (unsigned char)slot.first;
                            int nextslot_startidx = slot.first + int(round(unas->nplants) + 1e-3f);		// first index of next slot, after the one we inserted or are 'using' now
                            auto nextiter = openslots.erase(slotiter);
                            int nextslot_endidx = nextslot_startidx + (slotsize - nplants - 1);
                            if (nextslot_endidx >= nextslot_startidx)		// if the newly used slot does not use up all the space in the previously unused slot, then add new ununsed slot in remaining space
                                openslots.insert(nextiter, {nextslot_startidx, nextslot_endidx});
                            foundslot = true;
                            if ((unsigned char)(nextslot_startidx - 1) > maxidx) maxidx = (unsigned char)(nextslot_startidx - 1);
                            //std::cout << "Slot found: " << unas->startidx << ", " << nextslot_startidx - 1 << std::endl;
                            break;
                        }
                    }
                    if (!foundslot)
                    {
                        std::cout << "Slot not found for cohort!" << std::endl;
                        //throw std::logic_error("Slot not found for cohort");
                    }
                }
            }
        }
    }
    std::cout << "Maximum index: " << int(maxidx) << std::endl;

    return int(maxidx);
}

void CohortMaps::do_adjustments(int max_distance)
{
    undo_actionmap();
    if (max_distance > 0)
    {
        determine_actionmap(max_distance);
        apply_actionmap();
    }
    else
    {
        actionmap.fill({DonateDir::NONE, -1, 0});
    }
    maxpercell = determine_cohort_startidxes();
    specset_map.reset();
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
                    try{
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
                    } catch (const std::exception &e) { std::cerr << e.what(); }
                }
            }
        }
    }
}

void CohortMaps::determine_actionmap(int max_distance)
{
    if (timestep_maps.size() == 0)
        return;

    //std::default_random_engine gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    auto in_bound = [](int gw, int gh, int x, int y){
        return x < gw && x >= 0 && y < gh && y >= 0;
    };

    auto determine_action = [this, &in_bound, &unif, &gen, &max_distance](int x, int y, ValueGridMap<std::vector< data_importer::ilanddata::cohort > > &m)
    {
        int distance = unif(gen) * max_distance + 1;		// [1, max_distance] inclusive
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
        std::shuffle(dirs.begin(), dirs.end(), gen);

        for (auto &dxdy : dirs)
        {
            bool done = false;
            int xdiff = dxdy.first;
            int ydiff = dxdy.second;
            int cy = y + ydiff;
            int cx = x + xdiff;
            int distance = std::max(abs(xdiff), abs(ydiff));

            auto thisc = m.get(x, y);
            std::sort(thisc.begin(), thisc.end(), [](ilanddata::cohort &crt1, ilanddata::cohort &crt2) { return crt1.specidx < crt2.specidx; });
            auto &otherc = m.get(cx, cy);
            for (auto citer = thisc.begin(); citer != thisc.end(); advance(citer, 1))
            {
                if (unif(gen) > 0.5f)
                    continue;
                int specidx = citer->specidx;
                //if (citer->nplants > 2.0f)
                //    continue;
                auto iter = std::find_if(otherc.begin(), otherc.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
                if (iter == otherc.end())
                //if (otherc.size() == 0)		// REMOVEME: We should also be able to send cohorts to non-empty tiles
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

    if (progress_label_function)
        progress_label_function("Setting up actionmap...");
    if (progress_function)
        progress_function(0);

    actionmap.setDim(timestep_maps.at(0));
    actionmap.setDimReal(timestep_maps.at(0));
    actionmap.fill({DonateDir::NONE, -1, 0});

    int iteri = 0;
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
        iteri++;
        if (progress_function)
            progress_function(int(float(iteri) / timestep_maps.size() * 100));
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
   // std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
   // std::cerr << "GET ACTION MAP FLOATS: " << gw << " " << gh << " " << rw << " " << rh << std::endl;

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

void CohortMaps::move_cohort(std::vector<ilanddata::cohort> &destvec, std::vector<ilanddata::cohort> &srcvec, std::vector<ilanddata::cohort>::iterator &srciter, float xmod, float ymod)
{
    destvec.push_back(*srciter);
    srcvec.erase(srciter);

    auto &crt = destvec.back();
    crt.xs += xmod;
    crt.xe += xmod;
    crt.ys += ymod;
    crt.ye += ymod;

    if (crt.ys < 0.0f) crt.ys = 1e-3f;
    if (crt.ye >= rw) crt.ye = rw - 1e-3f;
    if (crt.xs < 0.0f) crt.xs = 1e-3f;
    if (crt.xe >= rh) crt.xe = rh - 1e-3f;


}

void CohortMaps::apply_actionmap()
{
    if (timestep_maps.size() == 0)
        return;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    int movecount_empty = 0;
    int movecount_total = 0;

    auto move_cohort_species = [&movecount_empty, &movecount_total, this](std::vector<ilanddata::cohort> &cvec_origin, std::vector<ilanddata::cohort> &cvec_dest, int specidx, float xmod, float ymod)
    {
        if (specidx < 0)
            return;
        auto iter = cvec_origin.end();
        iter = std::find_if(cvec_origin.begin(), cvec_origin.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
        while (iter != cvec_origin.end())
        {
            move_cohort(cvec_dest, cvec_origin, iter, xmod, ymod);
            cvec_dest.back().modified = true;

            if (cvec_dest.size() == 1)
            {
                movecount_empty++;
            }
            movecount_total++;
            iter = std::find_if(cvec_origin.begin(), cvec_origin.end(), [specidx](ilanddata::cohort &c) { return c.specidx == specidx; });
        }
    };

    if (progress_label_function)
        progress_label_function("Applying actionmap...");
    if (progress_function)
        progress_function(0);

    int iteri = 0;
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
                        if (y > d + 1)
                            move_cohort_species(m.get(x, y), m.get(x, y - d), action.specidx, 0.0f, -2.0f * action.distance);
                        break;
                    case DonateDir::WEST:
                        if (x > d + 1)
                            move_cohort_species(m.get(x, y), m.get(x - d, y), action.specidx, -2.0f * action.distance, 0.0f);
                        break;
                    case DonateDir::SOUTH:
                        if (y < gh - d - 1)
                            move_cohort_species(m.get(x, y), m.get(x, y + d), action.specidx, 0.0f, 2.0f * action.distance);
                        break;
                    case DonateDir::EAST:
                        if (x < gw - d - 1)
                            move_cohort_species(m.get(x, y), m.get(x + d, y), action.specidx, 2.0f * action.distance, 0.0f);
                        break;
                }
            }
        }
        iteri++;
        if (progress_function)
            progress_function(int(float(iteri) / timestep_maps.size() * 100));
    }
    std::cout << movecount_empty << " cohorts moved to empty tiles" << std::endl;
    std::cout << movecount_total << " cohorts moved in total" << std::endl;

    action_applied = true;
}

void CohortMaps::undo_actionmap()
{
    if (timestep_maps.size() == 0)
        return;

    if (!action_applied)
        return;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    int movecount = 0;

    using namespace data_importer;

    if (progress_label_function)
        progress_label_function("Undoing actionmap...");
    if (progress_function)
        progress_function(0);

    int iternum = 0;
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
                        if (y > d + 1)
                        {
                            auto &crts = m.get(x, y - d);
                            auto iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            while (iter != crts.end())
                            {
                                iter->modified = false;
                                float xmod = 0.0f, ymod = -2.0f * d;
                                move_cohort(m.get(x, y), m.get(x, y - d), iter, -xmod, -ymod);
                                movecount++;
                                iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            }
                        }
                        break;
                    case DonateDir::WEST:
                        if (x > d + 1)
                        {
                            auto &crts = m.get(x - d, y);
                            auto iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            while (iter != crts.end())
                            {
                                iter->modified = false;
                                float xmod = -2.0f * d, ymod = 0.0f;
                                move_cohort(m.get(x, y), m.get(x - d, y), iter, -xmod, -ymod);
                                movecount++;
                                iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            }
                        }
                        break;
                    case DonateDir::SOUTH:
                        if (y < gh - d - 1)
                        {
                            auto &crts = m.get(x, y + d);
                            auto iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            while (iter != crts.end())
                            {
                                iter->modified = false;
                                float xmod = 0.0f, ymod = 2.0f * d;
                                move_cohort(m.get(x, y), m.get(x, y + d), iter, -xmod, -ymod);
                                movecount++;
                                iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            }

                        }
                        break;
                    case DonateDir::EAST:
                        if (x < gw - d - 1)
                        {
                            auto &crts = m.get(x + d, y);
                            auto iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            while (iter != crts.end())
                            {
                                iter->modified = false;
                                float xmod = 2.0f * d, ymod = 0.0f;
                                move_cohort(m.get(x, y), m.get(x + d, y), iter, -xmod, -ymod);
                                movecount++;
                                iter = std::find_if(crts.begin(), crts.end(), [](const ilanddata::cohort &crt) { return crt.modified; });
                            }
                        }
                        break;
                }
            }
        }
        iternum++;
        if (progress_function)
            progress_function(int(float(iternum) / timestep_maps.size() * 100));
    }
    action_applied = false;
    std::cout << movecount << " cohorts moved to original tiles" << std::endl;

}

void CohortMaps::set_progress_function(std::function<void (int)> func)
{
    progress_function = func;
}

void CohortMaps::set_progress_label_function(std::function<void (std::string)> func)
{
    progress_label_function = func;
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

const std::vector<basic_tree> &CohortMaps::get_maturetrees(int timestep_idx) const
{
    return timestep_mature.at(timestep_idx);
}

const ValueGridMap<std::vector<ilanddata::cohort> > &CohortMaps::get_map(int timestep_idx) const
{
    return timestep_maps.at(timestep_idx);
}
