#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <bits/stdc++.h>
#include <deque>
#include <algorithm>
#include "cohorts.h"
#include "data_importer/data_importer.h"

ilanddata::params::params(std::string filename)
{
	std::ifstream ifs(filename);

	std::string line;

	while (ifs.good())
	{
		std::getline(ifs, line);
		std::stringstream ss(line);
		std::string category, value;
		std::getline(ss, category, '=');
		std::getline(ss, value);

		if (category == "width")
			width = atoi(value.c_str());
		else if (category == "height")
			height = atoi(value.c_str());
		else if (category == "quantdiv")
			quantdiv = atof(value.c_str());
		else if (category == "radialmult")
			radialmult = atof(value.c_str());
		else if (category == "maxpercell")
			maxpercell = atoi(value.c_str());
		else if (category == "samplemult")
			samplemult = atoi(value.c_str());
	}
}

ilanddata::cohort::cohort(int xs, int ys, int specidx, float dbh, float height, int nplants)
		: xs(xs), ys(ys), specidx(specidx), dbh(dbh), height(height), nplants(nplants)
{}

ilanddata::cohort::cohort(std::stringstream &ss)
{
	ss >> xs;
	ss >> ys;
	ss >> specidx;
	ss >> dbh;
	ss >> height;
	ss >> nplants;
}

std::ostream & operator << (std::ostream &ostr, const ilanddata::cohort &rhs)
{
	ostr << rhs.xs << " " << rhs.ys << " " << rhs.specidx << " " << rhs.dbh << " " << rhs.height << " " << rhs.nplants;
}

bool ilanddata::fileversion_gteq(std::string v1, std::string v2)
{
	std::stringstream sstr1(v1);
	std::stringstream sstr2(v2);

	while (sstr1.good() || sstr2.good())
	{
		std::string s1 = "0", s2 = "0";

		if (sstr1.good())
			std::getline(sstr1, s1, '.');
		if (sstr2.good())
			std::getline(sstr2, s2, '.');

		if (std::stoi(s1) > std::stoi(s2))
			return true;
		else if (std::stoi(s1) < std::stoi(s2))
			return false;
	}

	return true;		// in this case, they should be equal
}

ilanddata::filedata ilanddata::read(std::string filename, std::string minversion)
{
	using namespace ilanddata;

	std::ifstream ifs(filename);

	if (!ifs.is_open())
		throw std::invalid_argument("Could not open file at " + filename);

	filedata fdata;

	std::string lstr;

	std::getline(ifs, lstr);		// TODO: make sure this string's format is correct for the fileversion function call below
	if (!fileversion_gteq(lstr, minversion))
	{
		throw std::invalid_argument("File version " + lstr + " is not up to date with minimum version " + minversion + ". Aborting import.");
	}
	fdata.version = lstr;

	std::getline(ifs, lstr);
	fdata.timestep = std::stoi(lstr);

	std::getline(ifs, lstr);
	int ntrees_expected = std::stoi(lstr);		// can use this integer to check that the file and import are consistent by comparing to tree vector size

	int lidx = 3;

	std::cout << "Reading " << ntrees_expected << " trees..." << std::endl;

	int ntokens;
	for (int i = 0; i < ntrees_expected; i++)
	{
		std::getline(ifs, lstr);
		std::stringstream ss(lstr);

		basic_tree tree;
		
		int treeid;
		ss >> treeid;    // TODO: leaving out ID for now, must include it later

		ss >> tree.species;
		ss >> tree.x;
		ss >> tree.y;
		ss >> tree.height;
		ss >> tree.radius;

		float dbh;
		ss >> dbh;   // TODO: leaving out dbh for now, must include it later
		ss >> lstr;		// seems like an unused zero at the end of each line? ignoring it for now

		fdata.trees.push_back(tree);
	}

	std::getline(ifs, lstr);
	int ncohorts_expected = std::stoi(lstr);
	std::cout << "Reading " << ncohorts_expected << " cohorts..." << std::endl;

	for (int i = 0; i < ncohorts_expected; i++)
	{
		std::getline(ifs, lstr);
		std::stringstream ss(lstr);
		if (!ifs.eof())
			fdata.cohorts.emplace_back(ss);
		else
		{
			std::cout << "Warning: Read only " << i << " cohorts out of an expected " << ncohorts_expected << std::endl;
			break;
		}
	}

	std::cout << "Done. Returning file data for timestep " << fdata.timestep << std::endl;

	return fdata;
}

std::pair<std::vector<std::vector<int> >, std::vector<std::vector<std::map<int, int> > > > ilanddata::read_nospecies(std::string filename)
{
	using namespace ilanddata;

	auto fdata = ilanddata::read(filename, "2.0");

	int maxx = -1, maxy = -1;
	for (auto &c : fdata.cohorts)
	{
		if (c.xs > maxx)
			maxx = c.xs;
		if (c.ys > maxy)
			maxy = c.ys;
	}

	if (maxx % 2 == 0)
		throw std::runtime_error("Expected: uneven numbers for start x");
	if (maxy % 2 == 0)
		throw std::runtime_error("Expected: uneven numbers for start y");

	int nrows = (maxy - 1) / 2 + 1;
	int ncols = (maxx - 1) / 2 + 1;

	int width = std::min(100, ncols), height = std::min(100, nrows);

	std::vector<std::vector<int> > counts(height, std::vector<int>(width, 0));
	std::vector<std::vector<std::map<int, int > > > speccounts(height, std::vector<std::map<int, int> >(width));
	std::vector<int> cellcounts(width * height, 0);
	int maxcount = 0;

	for (auto &c : fdata.cohorts)
	{
		int row = (c.ys - 1) / 2;
		int col = (c.xs - 1) / 2;
		if (row >= height || col >= width)
			continue;
		counts.at(row).at(col) += int(c.nplants);
		speccounts.at(row).at(col)[c.specidx] = int(c.nplants);
		cellcounts.at(row * width + col) += 1;
		if (c.nplants > maxcount)
			maxcount = c.nplants;
	}
	std::cout << "maximum count: " << maxcount << std::endl;

	//std::cout << "Max cell count: " << *std::max_element(cellcounts.begin(), cellcounts.end()) << std::endl;

	//std::cin.get();

	return {counts, speccounts};
}

using namespace ilanddata;

static const float SQRT2 = sqrt(2.0f);

static float min(float v1, float v2) { if (v1 < v2) return v1; else return v2; }
static float max(float v1, float v2) { if (v1 > v2) return v1; else return v2; }

ilanddata::sampler::sampler(std::string filename, ilanddata::params params)
	: allc(ilanddata::read(filename, "2.0").cohorts), width(params.width), height(params.height), quantdiv(params.quantdiv), radialmult(params.radialmult),
		maxpercell(params.maxpercell), samplemult(params.samplemult)
{
	for (int i = 0; i < allc.size(); i++)
		generators.emplace_back(i);
}

std::vector<basic_tree> ilanddata::sampler::sample_one_soft(ilanddata::cohort chrt, std::default_random_engine &gen)
{
	float mx = chrt.xs;
	float my = chrt.ys;


	std::vector<basic_tree> trees;
	int nsample = std::min(int(chrt.nplants / quantdiv), maxpercell);
	for (int i = 0; i < nsample; i++)
	{
		float rdeg = unif(gen) * 2 * M_PI;
		float radius = unif(gen) * SQRT2 * radialmult;
		float th = chrt.height - 0.5f + unif(gen);

		basic_tree ntree(max(min(mx + radius * cos(rdeg), width), 0.0f), max(min(my + radius * sin(rdeg), height), 0.0f), th * 0.2f, th);
		ntree.species = chrt.specidx % 16;
		trees.push_back(ntree);
	}


	return trees;
}

std::deque<int> ilanddata::sampler::gen_poisson_list(int sqsize, int nplants, std::deque<int> *dists, std::default_random_engine &gen)
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

std::vector<basic_tree> ilanddata::sampler::sample_one_hard(ilanddata::cohort chrt, std::default_random_engine &gen)
{
	float mx = chrt.xs - 1.0f;
	float my = chrt.ys - 1.0f;

	
	std::vector<basic_tree> trees;
	int nsample = std::min(int(chrt.nplants / quantdiv), maxpercell);
	for (int i = 0; i < nsample; i++)
	{
		float th = chrt.height - 0.5f + unif(gen);

		basic_tree ntree(max(min(mx + unif(gen) * 2.0f, width), 0.0f), max(min(my + unif(gen) * 2.0f, height), 0.0f), th * 0.2f, th);
		ntree.species = chrt.specidx % 16;
		trees.push_back(ntree);
	}

	return trees;
}

std::vector<basic_tree> ilanddata::sampler::sample_all(bool soft)
{
	std::vector<basic_tree> trees;
	for (int i = 0; i < allc.size(); i++)
	{
		auto &c = allc[i];
		auto &gen = generators[i];
		if (c.xs - 1.0f > width || c.xs - 1.0f < 0 || c.ys - 1.0f > height || c.ys - 1.0f < 0)
			continue;
		if (soft)
		{
			auto temp = sample_one_soft(c, gen);
			trees.insert(trees.end(), temp.begin(), temp.end());
		}
		else
		{
			auto temp = sample_one_hard(c, gen);
			trees.insert(trees.end(), temp.begin(), temp.end());
		}
	}
	return trees;
}

int main(int argc, char * argv [])
{
	if (argc < 2)
	{
		std::cout << "usage: cohorts [infiles]+" << std::endl;
		return 1;
	}

	std::string cfgfile = "cohorts.cfg";
	std::vector<std::string> infiles;
	std::vector<std::string> outfiles;
	for (int i = 1; i < argc; i++)
	{
		infiles.push_back(argv[i]);
		std::string out = argv[i];
		out = out.substr(0, out.size() - 3);
		out += "_sampled.pdb";
		outfiles.push_back(out);
	}

	sampler sm(infiles.at(0), cfgfile);
	/*
	std::cout << "Starting sample..." << std::endl;
	auto start = std::chrono::steady_clock::now();
	auto trees = sm.sample_all(true);
	auto end = std::chrono::steady_clock::now();
	std::cout << trees.size() << " plants sampled in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
	*/

	std::default_random_engine gen;
	std::vector<std::deque<int> > allidxes;

	for (int cidx = 0; cidx < 100; cidx++)
	{
		std::deque<int> dists;
		auto idxes = sm.gen_poisson_list(2, 10, &dists, gen);
		std::random_shuffle(idxes.begin(), idxes.end());
		allidxes.push_back(idxes);
		//std::cout << "Generating index: " << cidx << std::endl;
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
	}

	return 0;

	std::cout << "Reading nospecies..." << std::endl;

	for (int j = 0; j < infiles.size(); j++)
	{
		std::default_random_engine gen;

		auto infile = infiles.at(j);
		auto outfile = outfiles.at(j);
		auto countgridpair = ilanddata::read_nospecies(infile);
		auto &countgrid = countgridpair.first;
		auto &speccounts = countgridpair.second;
		std::cout << countgrid.size() << std::endl;
		std::cout << countgrid.at(0).size() << std::endl;

		std::ofstream ofs(outfile);


		std::uniform_real_distribution<float> unif;
		for (int cidx = 0; cidx < 10000; cidx++)
		{
			std::cout << "Cell index: " << cidx << std::endl;
			int cx = cidx % 100;
			int cy = cidx / 100;
			int tileidx = int(unif(gen) * allidxes.size());
			int nsimplants = std::min(10, countgrid.at(cy).at(cx) / 10);
			std::map<int, int> specs = speccounts.at(cy).at(cx);
			//int nsimplants = 10;
			//int nplants = int(unif(gen) * 10);
			int nplants = nsimplants;
			auto &idxes = allidxes.at(tileidx);
			int count = 0;
			for (auto &i : idxes)
			{
				if (count >= nplants)
					break;
				float cprob = count / float(nplants);
				float probsum = 0.0f;
				int species = -1;
				for (auto &spair : specs)
				{
					probsum += spair.second / float(nplants);
					if (cprob < probsum)
					{
						species = spair.first;
						break;
					}
				}
				if (species == -1)
				{
					if (specs.size() > 0)
						species = specs.rbegin()->first;
					else if (nplants > 0)
						throw std::runtime_error("Species cannot be null");
				}
				int x = cx * 200 + i % 200;
				int y = cy * 200 + i / 200;
				ofs << x << " " << y << " " << species << std::endl;
				//ofs << x << " " << y << std::endl;
				count++;
			}
		}
	}

	/*
	auto chs = ilanddata::read("ecoviz_0.pdb");
	for (auto &ch : chs)
		std::cout << ch << std::endl;
	*/


	//data_importer::write_pdb(outfile, trees.data(), trees.data() + trees.size());

	return 0;
}
