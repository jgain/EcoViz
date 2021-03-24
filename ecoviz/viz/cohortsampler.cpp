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

cohortsampler::cohortsampler(float width, float height, int gw, int gh, int maxpercell, int samplemult)
	: width(width), height(height), maxpercell(maxpercell), samplemult(samplemult),
		gw(gw), gh(gh)
{
	tileidxes.setDim(gw, gh);
    tileidxes.setDimReal(width, height);

    int ntiles = 100;

    std::default_random_engine gen;
    std::uniform_int_distribution<int> unif(0, ntiles - 1);

    for (int y = 0; y < gh; y++)
        for (int x = 0; x < gw; x++)
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


std::vector<basic_tree> cohortsampler::sample(const ValueMap< std::vector<cohort> > &cohortmap)
{
    int placediv = 10;

    using namespace data_importer;

	std::vector<basic_tree> trees;

	std::uniform_real_distribution<float> unif;
	for (int cidx = 0; cidx < gw * gh; cidx++)
	{
        //std::cout << "Cell index: " << cidx << std::endl;
		int cx = cidx % gw;
		int cy = cidx / gw;
		int tileidx = tileidxes.get(cx, cy);
        const std::vector<cohort> &crts = cohortmap.get(cx, cy);
        float nsimplants = std::accumulate(crts.begin(), crts.end(), 0, [](int value, const cohort &c1) { return value + c1.nplants; });
        nsimplants = std::min(float(maxpercell), nsimplants / placediv);

        // XXX: we round up to the nearest integer when we sample plants (especially for the cases where we have less than one plant in the cohort)
        // 		we could also consider sampling with a certain probability, if num plants < 1
        int nplants = ceil(nsimplants);
		auto &idxes = randtiles.at(tileidx);
        int count = 0;
		for (auto &i : idxes)
		{
			if (count >= nplants)
				break;
            float cprob = count / float(nplants);
			float probsum = 0.0f;
            int species = -1;
            int specidx = 0;

            for (const cohort &crt : crts)
            {
                probsum += (crt.nplants / placediv) / float(nplants);
                if (cprob < probsum)
                {
                    species = crt.specidx;
                    break;
                }
                specidx++;
            }
			if (species == -1)
			{
                if (crts.size() > 0)
                {
                    species = crts.rbegin()->specidx;
                    specidx = crts.size() - 1;
                }
				else if (nplants > 0)
					throw std::runtime_error("Species cannot be null");
			}
			int x = cx * 200 + i % 200;		// XXX: the number of cm per cell is hardcoded here...
			int y = cy * 200 + i / 200;

            basic_tree tree(x / 100.0f, y / 100.0f, crts.at(specidx).height * 0.5f, crts.at(specidx).height);
			tree.species = species;
			trees.push_back(tree);
			//ofs << x << " " << y << " " << species << std::endl;
			count++;
		}
	}

	return trees;
}
