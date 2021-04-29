#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <bits/stdc++.h>
#include <deque>
#include <algorithm>
#include "cohortsampler.h"
#include "data_importer/data_importer.h"

using namespace data_importer::ilanddata;

static const float SQRT2 = sqrt(2.0f);

static float min(float v1, float v2) { if (v1 < v2) return v1; else return v2; }
static float max(float v1, float v2) { if (v1 > v2) return v1; else return v2; }

cohortsampler::cohortsampler(float tw, float th, float rw, float rh, float xoff, float yoff, int maxpercell, int samplemult)
    : maxpercell(maxpercell), samplemult(samplemult), width(rw), height(rh),
      tileidxes(tw, th, rw - xoff, rh - yoff, xoff, yoff)
{
    tileidxes.getDim(gw, gh);

    int ntiles = 100;

    std::default_random_engine gen;
    std::uniform_int_distribution<int> unif(0, ntiles - 1);

    for (int y = 0; y < gh; y++)
        for (int x = 0; x < gh; x++)
            tileidxes.set(x, y, unif(gen));

    generate_tiles(ntiles, 2);
}

std::deque<int> cohortsampler::gen_poisson_list(int sqsize, int nplants, std::deque<int> *dists, std::default_random_engine &gen)
{
	int ncells = 25;
	int ncent = sqsize * 100;
	int cellsize = ncent / ncells;
	int maxnum = ncent * ncent;

	std::vector< std::vector< std::vector<int> > > hash;

	hash.resize(ncells);
	for (auto &row : hash)
		row.resize(ncells);

	std::map<int, int> posdist;
	std::deque<int> positions;

	int ngen = nplants * samplemult;

	for (int i = 0; i < ngen; i++)
	{
		int newnum = int(unif(gen) * maxnum);
		int nx = newnum % ncent;
		int ny = newnum / ncent;
		int celly = ny / cellsize;
		int cellx = nx / cellsize;
		hash.at(celly).at(cellx).push_back(newnum);
	}

	int nremain = ngen;


	while (nremain > nplants)
	{
		int closest = std::numeric_limits<int>::max();
		std::vector<int>::iterator close_iter;
		std::vector<int> *close_vec = nullptr;

		for (int y = 0; y < ncells; y++)
			for (int x = 0; x < ncells; x++)
			{
				int idx = 0;
				int mag = 0;
				while (idx < hash.at(y).at(x).size())
				{
					int i = hash.at(y).at(x).at(idx);
					std::vector<int>::iterator iiter = std::next(hash.at(y).at(x).begin(), idx);
					int xi = i % ncent;
					int yi = i / ncent;
					int sidedist = ncent * ncent;
					if (unif(gen) < 1.0f)
					{
						sidedist = 2 *  min(min(min(xi, ncent - xi), yi), ncent - yi);
						sidedist *= sidedist;
					}
					if (sidedist < closest)
					{
						closest = sidedist;
						close_iter = iiter;
						close_vec = &hash.at(y).at(x);
					}

					bool found = false;
					for (int cyi = y - mag; cyi <= y + mag; cyi++)
					{
						int cy = cyi;
						int xincr;
						if (cy == y - mag || cy == y + mag)
							xincr = 1;
						else
							xincr = 2 * mag;
						if (cy < 0)
							continue;
							//cy = ncells - cy;
						if (cy >= ncells)
							continue;
							//cy = cy - ncells;
						for (int cxi = x - mag; cxi <= x + mag; cxi += xincr)
						{
							int cx = cxi;
							if (cx < 0)
								continue;
								//cx = ncells - cx;
							if (cx >= ncells)
								continue;
								//cx = cx - ncells;
							bool ignore_first = false;
							if (x == cx && y == cy)
								ignore_first = true;
							for (auto jiter = hash.at(cy).at(cx).begin(); jiter != hash.at(cy).at(cx).end(); advance(jiter, 1))
							{
								auto &j = *jiter;
								if (i == j && ignore_first)
								{
									ignore_first = false;
									continue;
								}
								else
								{
									found = true;
									int xj = j % ncent;
									int yj = j / ncent;
									int distance = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
									if (distance < closest)
									{
										closest = distance;
										close_iter = iiter;
										close_vec = &hash.at(y).at(x);
									}
								}
							}
						}
					}
					if (!found)
					{
						mag++;
						continue;
					}
					else
						idx++;
				}
			}
		if (close_vec)
		{
			close_vec->erase(close_iter);
			nremain--;
		}
		else
		{
			throw std::runtime_error("No closest distance found. Fix this bug");
		}
	}
	
	for (int y = 0; y < ncells; y++)
		for (int x = 0; x < ncells; x++)
		{
			for (auto &i : hash.at(y).at(x))
			{
				positions.push_back(i);
			}
		}
	
	return positions;
}

void cohortsampler::generate_tiles(int ntiles, int tilesize)
{
	std::default_random_engine gen;
	for (int cidx = 0; cidx < ntiles; cidx++)
	{
		std::deque<int> dists;
		auto idxes = gen_poisson_list(tilesize, maxpercell, &dists, gen);
		std::random_shuffle(idxes.begin(), idxes.end());
		randtiles.push_back(idxes);

		/*
		 * // some debug output below
		if (cidx >= 1 && cidx <= 3)
		{
			std::ofstream ofs("tile" + std::to_string(cidx) + ".pdb");
			for (auto &idx : idxes)
			{
				int x = idx % 200;
				int y = idx / 200;
				
				ofs << x << " " << y << std::endl;
			}
		}
		*/
	}
}

void cohortsampler::set_spectoidx_map(std::unique_ptr<ValueGridMap<std::vector<int> > > spectoidx_map_ptr)
{
    spectoidx_map = std::move(spectoidx_map_ptr);
}

std::vector<basic_tree> cohortsampler::sample(const ValueGridMap< std::vector<cohort> > &cohortmap, std::vector< std::vector<basic_tree> > *allcells_trees)
{
    int specmodulo = 64;

    using namespace data_importer;

    std::vector<basic_tree> trees;

    int cgw, cgh;
    cohortmap.getDim(cgw, cgh);


    printf("Cohortmap dimensions %d, %d\n", cgw, cgh);

    std::uniform_real_distribution<float> unif;
    for (int cidx = 0; cidx < cgw * cgh; cidx++)
    {
        const std::vector<cohort> &crts = cohortmap.get(cidx);

        if (crts.size() == 0)
        {
            if (allcells_trees)
                allcells_trees->push_back({});
            continue;
        }
        xy<float> middle = crts.front().get_middle();
        int tileidx = tileidxes.get_fromreal(middle.x, middle.y);

        float nsimplants = int(ceil(std::accumulate(crts.begin(), crts.end(), 0.0f, [](float value, const cohort &c1) { return value + c1.nplants; })) + 1e-3f);
        nsimplants = std::min(float(maxpercell), nsimplants);
        nsimplants = std::max(nsimplants, float(crts.size()));

        // XXX: we round up to the nearest integer when we sample plants (especially for the cases where we have less than one plant in the cohort)
        // 		we could also consider sampling with a certain probability, if num plants < 1
        int nplants = ceil(nsimplants);
        auto &idxes = randtiles.at(tileidx);
        int count = 0;
        if (nplants == 0 && crts.size() > 0)
        {
            for (const auto &c : crts)
            {
                c >> std::cout;
                std::cout << "---------------" << std::endl;
            }
            throw std::logic_error("Number of plants to be sampled zero but number of cohorts nonzero!");
        }
        std::vector<basic_tree> celltrees;
        for (const ilanddata::cohort &crt : crts)
        {

            int nplants = int(crt.nplants + 1e-3f);
            for (int i = 0; i < nplants; i++)
            {
                int pointidx = int(crt.startidx) + i;

                int idx = idxes.at(pointidx);

                int sx = crt.xs;
                int sy = crt.ys;
                int ex = crt.xe;
                int ey = crt.ye;

                int crtw = int(ex - sx);
                int crth = int(ey - sy);

                // get x, y indices from single index
                int cohortx = idx % 200;
                int cohorty = idx / 200;

                // normalize to [0, 1] in relative to cohort size in each dimension
                float cxf = cohortx / 200.0f;
                float cyf = cohorty / 200.0f;

                // scale by cohort size in meters
                cxf = cxf * float(crtw);
                cyf = cyf * float(crth);

                // add to world coordinates of cohort (top left corner)
                float x = float(sx) + cxf;		// XXX: the number of cm per cell is hardcoded here...
                float y = float(sy) + cyf;

                basic_tree tree(x, y, crt.height * 0.5f, crt.height);			// REPLACEME: radius = crts.at(specidx).height * 0.5f is temporary
                tree.species = crt.specidx % specmodulo;
                trees.push_back(tree);
                if (allcells_trees)
                    celltrees.push_back(tree);
                count++;
            }
        }
        /*
        for (const auto &c : crts)
        {
            auto iter = std::find_if(celltrees.begin(), celltrees.end(), [&c](const basic_tree &tree) { return tree.species == (c.specidx % specmodulo); });
            if (iter == celltrees.end())
            {
                for (const auto &c2 : crts)
                {
                    c2 >> std::cout;
                    std::cout << "------------------" << std::endl;
                }

                std::cout << "maxcounts: " << std::endl;
                for (auto &m : maxcounts)
                    std::cout << m << " ";
                std::cout << std::endl;
                std::cout << "cohortcounts: " << std::endl;
                for (auto &m : cohortcounts)
                    std::cout << m << " ";
                std::cout << std::endl;
                throw std::logic_error("at least one species in cohorts not sampled!");
            }
        }
        */
        if (allcells_trees)
            allcells_trees->push_back(celltrees);
    }

    return trees;
}

/*
std::vector<basic_tree> cohortsampler::sample(const ValueGridMap< std::vector<cohort> > &cohortmap, std::vector< std::vector<basic_tree> > *allcells_trees)
{
    int specmodulo = 64;

    using namespace data_importer;

    std::vector<basic_tree> trees;

    int cgw, cgh;
    cohortmap.getDim(cgw, cgh);


    printf("Cohortmap dimensions %d, %d\n", cgw, cgh);

	std::uniform_real_distribution<float> unif;
    for (int cidx = 0; cidx < cgw * cgh; cidx++)
	{
        const std::vector<cohort> &crts = cohortmap.get(cidx);

        if (crts.size() == 0)
        {
            if (allcells_trees)
                allcells_trees->push_back({});
            continue;
        }
        xy<float> middle = crts.front().get_middle();
        int tileidx = tileidxes.get_fromreal(middle.x, middle.y);
        xy<int> gxy = tileidxes.togrid(middle.x, middle.y);

        float nsimplants = std::accumulate(crts.begin(), crts.end(), 0.0f, [](float value, const cohort &c1) { return value + c1.nplants; });
        nsimplants = std::min(float(maxpercell), nsimplants);
        nsimplants = std::max(nsimplants, float(crts.size()));

        int specincr;
        if (spectoidx_map)
        {
            const auto &vec = spectoidx_map->get(gxy.x, gxy.y);
            specincr = std::count_if(vec.begin(), vec.end(), [](int v) { return v >= 0; });
            xy<float> xyreal = spectoidx_map->toreal(cidx);
            xyreal.x += 1.0f;
            xyreal.y += 1.0f;
            if (&cohortmap.get_fromreal(xyreal.x, xyreal.y) != &crts)
            {
                throw std::logic_error("Mismatch between spectoidx_map and cohortmap");
            }
        }
        else
            specincr = 1;


        int species = -1;

        // XXX: we round up to the nearest integer when we sample plants (especially for the cases where we have less than one plant in the cohort)
        // 		we could also consider sampling with a certain probability, if num plants < 1
        int nplants = ceil(nsimplants);
		auto &idxes = randtiles.at(tileidx);
        int count = 0;
        if (nplants == 0 && crts.size() > 0)
        {
            for (const auto &c : crts)
            {
                c >> std::cout;
                std::cout << "---------------" << std::endl;
            }
            throw std::logic_error("Number of plants to be sampled zero but number of cohorts nonzero!");
        }
        std::vector<basic_tree> celltrees;
        int cohortidx = 0;
        std::vector<int> cohortcounts(crts.size(), 0);
        std::vector<int> maxcounts;
        nplants = 0;		// recompute nplants, due to rounding issues (we round each float individually to get max count for each cohort, instead of summing then
                            // rounding up as done above. The former method results in a higher sum, which we compute here)
        for (const auto &crt : crts)
        {
            maxcounts.push_back(int(crt.nplants + 1e-3f));
            nplants += maxcounts.back();
        }
        while (true)
        {
            if (count > nplants)
				break;

            int origidx = cohortidx;
            bool finished = false;
            while (cohortcounts.at(cohortidx) >= maxcounts.at(cohortidx))
            {
                cohortidx++;
                cohortidx = cohortidx % crts.size();
                if (cohortidx == origidx)
                {
                    finished = true;
                    break;
                }
            }
            if (finished) break;

            cohortcounts.at(cohortidx)++;

            const auto &crt = crts.at(cohortidx);

            int i = spectoidx_map->get(cidx).at(crt.specidx) + specincr * cohortcounts.at(cohortidx);

            if (i >= idxes.size())
            {
                cohortcounts.at(cohortidx) = maxcounts.at(cohortidx);
                continue;
            }

            int idx = idxes.at(i);

            int specidx = cohortidx;

            species = crt.specidx;

            int sx = crt.xs;
            int sy = crt.ys;
            int ex = crt.xe;
            int ey = crt.ye;

            int crtw = int(ex - sx);
            int crth = int(ey - sy);

            // get x, y indices from single index
            int cohortx = idx % 200;
            int cohorty = idx / 200;

            // normalize to [0, 1] in relative to cohort size in each dimension
            float cxf = cohortx / 200.0f;
            float cyf = cohorty / 200.0f;

            // scale by cohort size in meters
            cxf = cxf * float(crtw);
            cyf = cyf * float(crth);

            // add to world coordinates of cohort (top left corner)
            float x = float(sx) + cxf;		// XXX: the number of cm per cell is hardcoded here...
            float y = float(sy) + cyf;

            basic_tree tree(x, y, crts.at(specidx).height * 0.5f, crts.at(specidx).height);			// REPLACEME: radius = crts.at(specidx).height * 0.5f is temporary
            tree.species = species % specmodulo;
            trees.push_back(tree);
            if (allcells_trees)
                celltrees.push_back(tree);
            count++;
        }
        if (allcells_trees)
            allcells_trees->push_back(celltrees);
	}

	return trees;
}
*/
