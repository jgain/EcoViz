/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems
 * Copyright (C) 2020  K.P. Kapp  (konrad.p.kapp@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/


#include <data_importer/data_importer.h>
#include <common/basic_types.h>

#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cmath>
#include <numeric>
#include <map>
#include <sstream>
#include <string>
#include <cassert>
#include <sqlite3.h>
const std::array<std::string, 12> months_arr = {"January",
                                        "February",
                                        "March",
                                        "April",
                                        "May",
                                        "June",
                                        "July",
                                        "August",
                                        "September",
                                        "October",
                                        "November",
                                        "December"};


data_importer::ilanddata::params::params(std::string filename)
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

data_importer::ilanddata::cohort::cohort(int xs, int ys, int specidx, float dbh, float height, int nplants)
		: xs(xs), ys(ys), specidx(specidx), dbh(dbh), height(height), nplants(nplants)
{
    xe = xs + 2;
    ye = ys + 2;
}

data_importer::ilanddata::cohort::cohort(std::stringstream &ss, const std::map<std::string, int> &species_lookup)
{
    std::string id;
    ss >> xs;
	ss >> ys;
    ss >> id;
    specidx = species_lookup.at(id);
	ss >> dbh;
	ss >> height;
    ss >> nplants;

    xe = xs + 2;
    ye = ys + 2;
}

std::ostream &data_importer::ilanddata::cohort::operator >>(std::ostream &ostr) const
{
    ostr << "coordinates: (" << xs << ", " << xe << "), " << ys << ", " << ye << ")" << std::endl;
    ostr << "dbh: " << dbh << std::endl;
    ostr << "height: " << height << std::endl;
    ostr << "specidx: " << specidx << std::endl;
    ostr << "nplants: " << int(nplants + 1e-3f) << std::endl;
    ostr << "startidx: " << int(startidx) << std::endl;
    return ostr;
}

bool data_importer::ilanddata::cohort::operator ==(const data_importer::ilanddata::cohort &other) const
{
    return fabs(this->xs - other.xs) < 1e-3f
            && fabs(this->xe - other.xe) < 1e-3f
            && fabs(this->ys - other.ys) < 1e-3f
            && fabs(this->ye - other.ye) < 1e-3f
            && fabs(this->dbh - other.dbh) < 1e-3f
            && fabs(this->height - other.height) < 1e-3f
            && fabs(this->nplants - other.nplants) < 1e-3f
            && this->specidx == other.specidx;
}

bool data_importer::ilanddata::cohort::operator !=(const data_importer::ilanddata::cohort &other) const
{
    return !(*this == other);
}

xy<float> data_importer::ilanddata::cohort::get_middle() const
{
    return xy<float>((xe + xs) / 2.0f, (ye + ys) / 2.0f);
}

float data_importer::ilanddata::cohort::get_size() const
{
    return (xe - xs) * (ye - ys);
}

bool data_importer::ilanddata::fileversion_gteq(std::string v1, std::string v2)
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

std::vector<data_importer::ilanddata::filedata> data_importer::ilanddata::read_many(const std::vector<std::string> &filenames, std::string minversion, const std::map<std::string, int> &species_lookup)
{
    std::vector<data_importer::ilanddata::filedata> fdatas;
    for (auto &fname : filenames)
    {
        fdatas.push_back(read(fname, minversion, species_lookup, false));
    }
    return fdatas;
}

data_importer::ilanddata::filedata data_importer::ilanddata::read(std::string filename, std::string minversion, const std::map<std::string, int> &species_lookup, bool timestep_only)
{
    using namespace data_importer::ilanddata;

    std::map<int, bool> species_avail;
    std::map<int, bool> species_avail_cohorts;

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

    if(timestep_only)
    {
        return fdata;
    }

	std::getline(ifs, lstr);
	int ntrees_expected = std::stoi(lstr);		// can use this integer to check that the file and import are consistent by comparing to tree vector size

	std::cout << "Reading " << ntrees_expected << " trees..." << std::endl;
    std::string species_id;

	for (int i = 0; i < ntrees_expected; i++)
	{
		std::getline(ifs, lstr);
		std::stringstream ss(lstr);

		basic_tree tree;
		
		int treeid;
		ss >> treeid;    // TODO: leaving out ID for now, must include it later

        ss >> species_id; // alpha-numeric species key
        tree.species = species_lookup.at(species_id);
        //ss >> tree.species;
		ss >> tree.x;
		ss >> tree.y;
		ss >> tree.height;
		ss >> tree.radius;
        ss >> tree.dbh;
		ss >> lstr;		// seems like an unused zero at the end of each line? ignoring it for now

        fdata.trees.push_back(tree);

        species_avail[tree.species] = true;
	}

	std::getline(ifs, lstr);
	int ncohorts_expected = std::stoi(lstr);
    std::cout << "Reading " << ncohorts_expected << " cohorts..." << std::endl;

    float minx = std::numeric_limits<float>::max() , miny = std::numeric_limits<float>::max();
    float maxx = -std::numeric_limits<float>::max() , maxy = -std::numeric_limits<float>::max();
    float dx = -1.0f, dy = -1.0f;

	for (int i = 0; i < ncohorts_expected; i++)
	{
		std::getline(ifs, lstr);
		std::stringstream ss(lstr);
        if (!ifs.eof())
        {
            fdata.cohorts.emplace_back(ss, species_lookup); // load values from stream
            auto &crt = fdata.cohorts.back();
            if (crt.xs < minx)
            {
                minx = crt.xs;
            }
            if (crt.ys < miny)
            {
                miny = crt.ys;
            }
            if (crt.xe > maxx)
            {
                maxx = crt.xe;
            }
            if (crt.ye > maxy)
            {
                maxy = crt.ye;
            }
            float xdiff = crt.xe - crt.xs;
            float ydiff = crt.ye - crt.ys;
            if (dy < 0.0f && dx < 0.0f)
            {
                dy = ydiff;
                dx = xdiff;
            }
            else
            {
                if (fabs(dy - ydiff) > 1e-5f || fabs(dx - xdiff) > 1e-5f)
                {
                    throw std::invalid_argument("Input cohorts have inconsistent sizes in file " + filename);
                }
            }
            species_avail_cohorts[crt.specidx] = true;
        }
		else
		{
			std::cout << "Warning: Read only " << i << " cohorts out of an expected " << ncohorts_expected << std::endl;
			break;
		}
    }

    fdata.minx = minx;
    fdata.miny = miny;
    fdata.maxx = maxx;
    fdata.maxy = maxy;
    fdata.dx = dx;
    fdata.dy = dy;

    auto minmaxh = std::minmax_element(fdata.trees.begin(), fdata.trees.end(), [&](const basic_tree &a, const basic_tree &b) {return a.height<b.height;});
    auto minmaxdbh = std::minmax_element(fdata.trees.begin(), fdata.trees.end(), [&](const basic_tree &a, const basic_tree &b) {return a.dbh<b.dbh;});
    std::cout << "Range height: " << minmaxh.first->height << " - " << minmaxh.second->height << ", Range DBH: " << minmaxdbh.first->dbh << " - " << minmaxdbh.second->dbh << std::endl;

    std::cout << "Maximum species index for larger trees in timestep " << fdata.timestep << ": " << std::max_element(species_avail.begin(), species_avail.end(), [](const std::pair<int, bool> &p1, const std::pair<int, bool> &p2) { return p1.first < p2.first; })->first << std::endl;
    std::cout << "Maximum species index for cohort trees in timestep " << fdata.timestep << ": " << std::max_element(species_avail_cohorts.begin(), species_avail_cohorts.end(), [](const std::pair<int, bool> &p1, const std::pair<int, bool> &p2) { return p1.first < p2.first; })->first << std::endl;
    std::cout << "Number of larger tree species found in timestep " << fdata.timestep << ": " << species_avail.size() << std::endl;
    std::cout << "Number of cohort tree species found in timestep " << fdata.timestep << ": " << species_avail_cohorts.size() << std::endl;
    std::cout << "Done. Returning file data for timestep " << fdata.timestep << std::endl;

	return fdata;
}

struct cohortA {
 int treeid;
 char code[4]; // 4 byte ASCII tree code
 int x;
 int y;
 float height;
 float radius;
 float dbh;
 int dummy;
};

struct cohortB {
    int xs;
    int ys;
    char code[4];
    float dbh;
    float height;
    float nplants;
};

data_importer::ilanddata::filedata data_importer::ilanddata::readbinary(std::string filename, std::string minversion, const std::map<std::string, int> &species_lookup, bool timestep_only)
{
    using namespace data_importer::ilanddata;

    std::map<int, bool> species_avail;
    std::map<int, bool> species_avail_cohorts;

    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs.is_open())
        throw std::invalid_argument("Could not open binary file at " + filename);

    filedata fdata;

    std::string lstr;

    // TODO: make sure this string's format is correct for the fileversion function call below
    int slen;

    ifs.read(reinterpret_cast<char*>(&slen), sizeof(int));
    char *buf = new char [slen+1];
    ifs.read(buf, slen);
    buf[slen] = 0;
    lstr = buf;
    delete [] buf;

    if (!fileversion_gteq(lstr, minversion))
    {
        throw std::invalid_argument("File version " + lstr + " is not up to date with minimum version " + minversion + ". Aborting import.");
    }
    fdata.version = lstr;

    ifs.read(reinterpret_cast<char*>(&fdata.timestep), sizeof(int));

    if(timestep_only)
    {
        return fdata;
    }

    int ntrees_expected;
    ifs.read(reinterpret_cast<char*>(&ntrees_expected), sizeof(int));

    // can use this integer to check that the file and import are consistent by comparing to tree vector size

    std::cout << "Reading " << ntrees_expected << " trees..." << std::endl;
    std::string species_id;

    std::vector<cohortA> Adata;
    Adata.resize(ntrees_expected);

    ifs.read(reinterpret_cast<char*>(Adata.data()), sizeof(cohortA)*ntrees_expected);

    for (int i = 0; i < ntrees_expected; i++)
    {
        basic_tree tree;

        // TODO: leaving out ID for now, must include it later

        std::string species_id; // alpha-numeric species key
        species_id += Adata[i].code[0];
        species_id += Adata[i].code[1];
        species_id += Adata[i].code[2];
        species_id += Adata[i].code[3];
        tree.species = species_lookup.at(species_id);

        tree.x = Adata[i].x;
        tree.y = Adata[i].y;
        tree.height = Adata[i].height;
        tree.radius = Adata[i].radius;
        tree.dbh = Adata[i].dbh;
        // seems like an unused zero at the end of each line? ignoring it for now

        fdata.trees.push_back(tree);

        species_avail[tree.species] = true;
    }

    Adata.clear();

    std::vector<cohortB> dataB;
    int ncohorts_expected;

    ifs.read(reinterpret_cast<char*>(&ncohorts_expected), sizeof(int));

    dataB.resize(ncohorts_expected);

    std::cout << "Reading " << ncohorts_expected << " cohorts..." << std::endl;

    float minx = std::numeric_limits<float>::max() , miny = std::numeric_limits<float>::max();
    float maxx = -std::numeric_limits<float>::max() , maxy = -std::numeric_limits<float>::max();
    float dx = -1.0f, dy = -1.0f;

    ifs.read(reinterpret_cast<char*>(dataB.data()), sizeof(cohortB)*ncohorts_expected);

    for (int i = 0; i < ncohorts_expected; i++)
    {
        std::string species_id; // alpha-numeric species key
        species_id += dataB[i].code[0];
        species_id += dataB[i].code[1];
        species_id += dataB[i].code[2];
        species_id += dataB[i].code[3];

        fdata.cohorts.emplace_back(dataB[i].xs, dataB[i].ys, species_lookup.at(species_id), dataB[i].dbh, dataB[i].height, dataB[i].nplants);

        auto &crt = fdata.cohorts.back();
        if (crt.xs < minx)
        {
            minx = crt.xs;
        }
        if (crt.ys < miny)
        {
            miny = crt.ys;
        }
        if (crt.xe > maxx)
        {
            maxx = crt.xe;
        }
        if (crt.ye > maxy)
        {
            maxy = crt.ye;
        }
        float xdiff = crt.xe - crt.xs;
        float ydiff = crt.ye - crt.ys;
        if (dy < 0.0f && dx < 0.0f)
        {
            dy = ydiff;
            dx = xdiff;
        }
        else
        {
            if (fabs(dy - ydiff) > 1e-5f || fabs(dx - xdiff) > 1e-5f)
            {
                throw std::invalid_argument("Input cohorts have inconsistent sizes in file " + filename);
            }
        }

        species_avail_cohorts[crt.specidx] = true;
    }

    fdata.minx = minx;
    fdata.miny = miny;
    fdata.maxx = maxx;
    fdata.maxy = maxy;
    fdata.dx = dx;
    fdata.dy = dy;

    auto minmaxh = std::minmax_element(fdata.trees.begin(), fdata.trees.end(), [&](const basic_tree &a, const basic_tree &b) {return a.height<b.height;});
    auto minmaxdbh = std::minmax_element(fdata.trees.begin(), fdata.trees.end(), [&](const basic_tree &a, const basic_tree &b) {return a.dbh<b.dbh;});
    std::cout << "Range height: " << minmaxh.first->height << " - " << minmaxh.second->height << ", Range DBH: " << minmaxdbh.first->dbh << " - " << minmaxdbh.second->dbh << std::endl;

    std::cout << "Maximum species index for larger trees in timestep " << fdata.timestep << ": " << std::max_element(species_avail.begin(), species_avail.end(), [](const std::pair<int, bool> &p1, const std::pair<int, bool> &p2) { return p1.first < p2.first; })->first << std::endl;
    std::cout << "Maximum species index for cohort trees in timestep " << fdata.timestep << ": " << std::max_element(species_avail_cohorts.begin(), species_avail_cohorts.end(), [](const std::pair<int, bool> &p1, const std::pair<int, bool> &p2) { return p1.first < p2.first; })->first << std::endl;
    std::cout << "Number of larger tree species found in timestep " << fdata.timestep << ": " << species_avail.size() << std::endl;
    std::cout << "Number of cohort tree species found in timestep " << fdata.timestep << ": " << species_avail_cohorts.size() << std::endl;
    std::cout << "Done. Returning file data for timestep " << fdata.timestep << std::endl;

    return fdata;
}


void data_importer::ilanddata::trim_filedata_spatial(data_importer::ilanddata::filedata &data, int width, int height)
{
    using namespace data_importer::ilanddata;
    std::vector<std::vector<cohort>::iterator > to_remove;
    for (auto iter = data.cohorts.begin(); iter != data.cohorts.end(); advance(iter, 1))
    {
        if (iter->xs <= -2 || iter->xs >= width || iter->ys <= -2 || iter->ys >= height)
        {
            to_remove.push_back(iter);
        }
    }

    // remove iterators in reverse order, so that we don't invalidate the ones coming after the one we removed last
    for (auto riter = to_remove.rbegin(); riter != to_remove.rend(); advance(riter, 1))
    {
        data.cohorts.erase(*riter);
    }

    std::vector<std::vector<basic_tree>::iterator > to_remove_trees;

    for (auto iter = data.trees.begin(); iter != data.trees.end(); advance(iter, 1))
    {
        to_remove_trees.push_back(iter);
    }

    for (auto riter = to_remove_trees.rbegin(); riter != to_remove_trees.rend(); advance(riter, 1))
    {
        data.trees.erase(*riter);
    }
}

std::map<std::string, int> make_monthmap()
{
    std::map<std::string, int> monthmap;
    for (int i = 0; i < (int) months_arr.size(); i++)
    {
        monthmap[months_arr[i]] = i;
    }
    return monthmap;
}

const std::map<std::string, int> monthmap = make_monthmap();

void data_importer::modelset::add_model(treemodel model)
{
    int count = 0;
    for (auto &m : models)
    {
        if (model.hmin < m.hmin)
        {
            models.insert(std::next(models.begin(), count), model);
            break;
            //return;
        }
        count++;
    }
    //models.push_back(model);		// if we return, instead of break above, use this code

    // if we break, instead of return above, use this code
    if (count == (int) models.size())
    {
        models.push_back(model);
    }

    vueid_to_ratio[model.vueid] = model.whratio / 2.0f;

    //setup_ranges();		// if we want the object ready after sql import, this has to happen...
}

void data_importer::modelset::setup_ranges()
{
    ranges.clear();
    selections.clear();
    samplemap.clear();
    for (int i = 0; i < (int) models.size(); i++)
    {
        add_to_ranges(i);
    }
    setup_selections();

    // setup samplemap

    for (int i = 1; i < (int) ranges.size(); i++)
    {
        float cand;
        if ((cand = ranges[i] - ranges[i - 1]) < minrange)
        {
            minrange = cand;
        }
    }
    // let's make binsize 1.0f - if
    // we can enforce that range borders are integers, we will not have bins that stride
    // range borders
    //binsize = 1.0f;


    binsize = minrange;
    nbins = std::ceil(ranges.back() - ranges.front()) / binsize;

    //float binsize = minrange / 5.0f;
    //int nbins = std::ceil((ranges.back() - ranges.front()) / binsize);
    /*
    if (nbins > 200)
    {
        nbins = 200;
        binsize = (ranges.back() - ranges.front()) / nbins;
    }
    */
    samplemap.resize(nbins);

    int curridx = 0;
    samplemap.at(0) = curridx;
    for (int i = 1; i < nbins; i++)
    {
        if (i * binsize >= ranges.at(curridx + 1) - 1e-4f)		// curridx + 1, because ranges[curr_idx + 1] corresponds to selections[curr_idx]
        {
            curridx++;
            if (i * binsize >= ranges.at(curridx + 1) - 1e-4)
            {
                throw std::runtime_error("binsize too big in modelset::setup_ranges");
            }
        }
        samplemap.at(i) = curridx;
    }
}

float data_importer::modelset::sample_rh_ratio(float height, int *vuemodel)
{
    int vm = sample_selection_robust(height);
    if (vuemodel)
    {
        *vuemodel = vm;
    }
    return vueid_to_ratio.at(vm);
}

int data_importer::modelset::sample_selection_robust(float height)
{
    std::vector<int> *sel;
    //binsize = (ranges.back() - ranges.front()) / nbins;
    int idx = height / binsize;
    if (idx >= (int) samplemap.size() || height > ranges.back())
    {
        throw std::runtime_error("height out of range in modelset::sample_selection");
    }
    int selidx = -1;
    if (idx < (int) samplemap.size() - 1)
    {
        if (samplemap.at(idx) != samplemap.at(idx + 1))
        {
            if (height > ranges.at(samplemap.at(idx + 1)))
            {
                selidx = samplemap.at(idx + 1);
            }
        }
        if (selidx == -1)
            selidx = samplemap.at(idx);
    }
    else
        selidx = samplemap.back();

    sel = &selections.at(selidx);

    // std::cout << "selection size: " << sel->size() << std::endl;
    int randidx = rand() % sel->size();
    return sel->at(randidx);
}

// this function makes the assumption that each bin has size 1.0f
int data_importer::modelset::sample_selection_fast(float height)
{
    throw std::runtime_error("modelset::sample_selection_fast not implemented");

    std::vector<int> *sel;
    //float binsize = (ranges.back() - ranges.front()) / nbins;
    int idx = height - ranges.front();
    if (idx >= (int) samplemap.size() || height > ranges.back())
    {
        throw std::runtime_error("height out of range in modelset::sample_selection");
    }
    int selidx = samplemap.at(idx);

    sel = &selections.at(selidx);

    int randidx = rand() % sel->size();
    return sel->at(randidx);
}

int data_importer::modelset::sample_selection_simple(float height)
{
    throw std::runtime_error("modelset::sample_selection_simple not implemented");

    int selidx = -1;
    for (int i = 1; i < (int) ranges.size(); i++)
    {
        if (height < ranges.at(i))
        {
            selidx = i;
            break;
        }
    }
    if (selidx == -1 || height < ranges.at(0))
    {
        throw std::runtime_error("height out of range in modelset::sample_selection_simple");
    }
    auto &sel = selections.at(selidx);
    int randidx = rand() % sel.size();
    return sel.at(randidx);
}

void data_importer::modelset::add_to_ranges(int midx)
{
    auto &m = models.at(midx);
    int minidx = -1;
    for (int i = 0; i < (int) ranges.size(); i++)
    {
        if (fabs(ranges[i] - m.hmin) < 1e-4f)
        {
            // this model's hmin already coincides with another range border. So we don't add this division
            minidx = i;
            break;
        }
        else if (m.hmin < ranges[i])
        {
            // the division at ranges[i] is the smallest div bigger than this m.hmin. Insert before it
            minidx = i;
            ranges.insert(std::next(ranges.begin(), i), m.hmin);
            break;
        }
    }
    if (minidx == -1)
    {
        //if (ranges.size() > 0)
        //    throw std::runtime_error("minimum value leaves a gap in ranges, in data_importer::modelset::setup_ranges");
        ranges.push_back(m.hmin);
        ranges.push_back(m.hmax);
        //selections.push_back({midx});
    }
    else
    {
       int maxidx = -1;
       for (int i = minidx + 1; i < (int) ranges.size(); i++)
       {
           bool found = false;
            if (fabs(ranges[i] - m.hmax) < 1e-4f)
            {
                found = true;
            }
            else if (m.hmax < ranges[i])
            {
                ranges.insert(std::next(ranges.begin(), i), m.hmax);
                //selections.insert(std::next(selections.begin(), i - 1), {});
                found = true;
            }
            //selections.at(i - 1).push_back(midx);
            if (found)
            {
                maxidx = i;
                break;
            }
       }
       if (maxidx == -1)
       {
           ranges.push_back(m.hmax);
           //selections.push_back({midx});
       }
    }
}

void data_importer::modelset::setup_selections()
{
    selections.clear();
    for (int i = 0; i < (int) ranges.size() - 1; i++)
    {
        selections.push_back({});
        float min = ranges[i];
        float max = ranges[i + 1];
        for (treemodel &m : models)
        {
            if (m.hmin + 1e-2f < max && m.hmax - 1e-2f > min)   // allowing for quite a large margin of error here...
            {
                selections.back().push_back(m.vueid);
            }
        }
    }
}


using namespace basic_types;



void data_importer::eliminate_outliers(MapFloat &data)
{
    int width, height;
    data.getDim(width, height);
    float mean = std::accumulate(data.begin(), data.end(), 0.0f);
    mean = mean / (width * height);

    float stddev = std::accumulate(data.begin(), data.end(), 0.0f, [&mean](float &sum, float &val) { return sum + (val - mean) * (val - mean); });
    stddev = sqrt(stddev / (width * height));

    std::for_each(data.begin(), data.end(), [&mean, &stddev](float &val) { if (val > mean + 3 * stddev) val = mean; });
}

std::vector<basic_tree> data_importer::read_pdb(std::string filename)
{
    std::map<int, std::vector<MinimalPlant> > retvec;
    if (!read_pdb(filename, retvec))
    {
        throw std::runtime_error("File " + filename + " not found in data_importer::read_pdb");
    }
    else
    {
        return data_importer::minimal_to_basic(retvec);
    }
}

bool data_importer::read_pdb(std::string filename, std::map<int, std::vector<MinimalPlant> > &retvec)
{
    //std::vector< std::vector<basic_plant> > retvec;
    std::ifstream infile;
    int numcat;

    infile.open(filename, std::ios_base::in);
    if(infile.is_open())
    {
        // list of prioritized categories, not all of which are used in a particular sandbox
        infile >> numcat;
        for(int c = 0; c < numcat; c++)
        {
            float junk;
            int cat;
            int nplants;

            infile >> cat;
            for (int i = 0; i < 3; i++)
                infile >> junk;	// skip minheight, maxheight, and avgCanopyRadToHeightRatio

            infile >> nplants;
            retvec[cat].resize(nplants);
            for (int plnt_idx = 0; plnt_idx < nplants; plnt_idx++)
            {
                float x, y, z, radius, height;
                infile >> x >> y >> z;
                infile >> height;
                infile >> radius;
                MinimalPlant plnt = {x, y, height, radius, 1.0f, false, cat}; // TO DO need to fix dbh import
                retvec[cat][plnt_idx] = plnt;
            }
        }
        std::cerr << std::endl;


        infile.close();
        return true;
    }
    else
        return false;
}


static std::string get_name_from_line(std::string line)
{
    std::stringstream sstr(line);
    std::string name;
    std::getline(sstr, name, ' ');
    return name;
}

static species_params parse_species_line(std::string line, std::string name)
{
    std::vector<float> locs, width_scales;
    std::stringstream sstr = std::stringstream(line);
    std::string astr, bstr, percstr;
    std::getline(sstr, astr, ' ');
    std::getline(sstr, bstr, ' ');
    std::getline(sstr, percstr, ' ');
    float a, b, perc;
    a = std::stof(astr);
    b = std::stof(bstr);
    perc = std::stof(percstr);

    int i = 0;
    for (; sstr.good(); i++)
    {
        float location, width_scale;
        std::string token;
        std::getline(sstr, token, ' ');
        int mod = i % 2;
        switch(mod)
        {
        case (0):
            location  = atof(token.c_str());
            locs.push_back(location);
            break;
        case (1):
            width_scale = atof(token.c_str());
            width_scales.push_back(width_scale);
            break;
        default:
            break;
        }
    }
    while (locs.size() > width_scales.size())
    {
        locs.pop_back();
    }
    while (width_scales.size() > locs.size())
    {
        width_scales.pop_back();
    }

    return species_params(name, a, b, locs, width_scales, perc);
}

static int sql_callback_common_data_species(void *write_info, int argc, char ** argv, char **colnames)
{
    //std::cout << "Processing row in sql_callback_common_data_species" << std::endl;

    data_importer::common_data *common = (data_importer::common_data *)write_info;

    int tree_idx;
    for (int i = 0; i < argc; i++)
    {
        std::string valstr;
        std::string colstr;
        if (argv[i])
        {
            valstr = argv[i];
        }
        else
        {
            valstr = "NULL";
        }
        colstr = colnames[i];

        if (colstr == "Tree_ID")
        {
            tree_idx = std::stoi(valstr);
        }
    }
    data_importer::species sp;
    sp.idx = tree_idx;
    auto result = common->all_species.insert({sp.idx, sp});
    assert(result.second);		// Each species should only be inserted once. If it already exists in the map
                                // there is a bug
    return 0;
}

static int sql_callback_common_data_all_species(void *write_info, int argc, char ** argv, char **colnames)
{
    data_importer::common_data *common = (data_importer::common_data *)write_info;

    int tree_idx;
    std::string alpha_code, common_name, sci_name;
    float draw_color [4];
    float draw_height;
    float draw_radius, draw_box1, draw_box2;

    data_importer::treeshape draw_shape;

    for (int i = 0; i < argc; i++)
    {
        std::string valstr;
        std::string colstr;
        if (argv[i])
        {
            valstr = argv[i];
        }
        else
        {
            valstr = "NULL";
        }
        colstr = colnames[i];

        // std::cerr << colstr << " " << valstr << std::endl;
        if (colstr == "Tree_ID")
        {
            tree_idx = std::stoi(valstr);
        }
        else if (colstr == "alpha_code")
        {
            alpha_code = valstr;
        }
        else if (colstr == "common_name")
        {
            common_name = valstr;
        }
        else if (colstr == "scientific_name")
        {
            sci_name = valstr;
        }
        else if (colstr == "base_col_red")
        {
            draw_color[0] = std::stof(valstr);
        }
        else if (colstr == "base_col_green")
        {
            draw_color[1] = std::stof(valstr);
        }
        else if (colstr == "base_col_blue")
        {
            draw_color[2] = std::stof(valstr);
        }
        else if (colstr == "draw_height")
        {
            draw_height = std::stof(valstr);
        }
        else if (colstr == "draw_radius")
        {
            draw_radius = std::stof(valstr);
        }
        else if (colstr == "draw_box1")
        {
            draw_box1 = std::stof(valstr);
        }
        else if (colstr == "draw_box2")
        {
            draw_box2 = std::stof(valstr);
        }
        else if (colstr == "draw_shape")
        {
            if (valstr == "CONE")
            {
                draw_shape = data_importer::treeshape::CONE;
            }
            else if (valstr == "SPHR")
            {
                draw_shape = data_importer::treeshape::SPHR;
            }
            else if (valstr == "BOX")
            {
                draw_shape = data_importer::treeshape::BOX;
            }
            else if (valstr == "INVCONE")
            {
                draw_shape = data_importer::treeshape::INVCONE;
            }
            else if (valstr == "HEMISPHR")
            {
                draw_shape = data_importer::treeshape::HEMISPHR;
            }
            else if (valstr == "CYL")
            {
                draw_shape = data_importer::treeshape::CYL;
            }
        }
    }
    data_importer::species sp;
    sp.idx = tree_idx;
    sp.basecol[0] = draw_color[0], sp.basecol[1] = draw_color[1], sp.basecol[2] = draw_color[2];
    sp.basecol[3] = 1.0f;
    sp.draw_hght = draw_height;
    sp.draw_radius = draw_radius;
    sp.draw_box1 = draw_box1;
    sp.draw_box2 = draw_box2;
    sp.cname = common_name;
    sp.sname = sci_name;
    sp.alpha_code = alpha_code;
    sp.shapetype = draw_shape;

    auto result = common->all_species.insert({sp.idx, sp});
    return 0;
}

static int sql_callback_common_data_check_tables(void *junk, int argc, char ** argv, char **colnames)
{
    for (int i = 0; i < argc; i++)
    {
        std::string value;
        if (argv[i])
            value = argv[i];
        else
            value = "NULL";
        std::cout << colnames[i] << ": " << value << std::endl;
    }
    std::cout << std::endl;
}

static void sql_err_handler(sqlite3 *db, int errcode, char * errmsg)
{
    if (errcode != SQLITE_OK)
    {
        std::cout << "SQL error: " << errmsg << std::endl;
        sqlite3_free(errmsg);
        sqlite3_close(db);
        throw std::runtime_error("SQL SELECT statment error");
    }
}

data_importer::common_data::common_data(std::string db_filename)
{
    sqlite3 *db;
    int errcode;
    std::cout << "Opening database file at " << db_filename << std::endl;
    errcode = sqlite3_open(db_filename.c_str(), &db);
    if (errcode)
    {
        std::string errstr = std::string("Cannot open database file at ") + db_filename;
        sqlite3_close(db);
        throw std::runtime_error(errstr.c_str());
    }
    char * errmsg;

    errcode = sqlite3_exec(db, "SELECT Tree_ID, \
                                    alpha_code, \
                                    common_name, \
                                    scientific_name, \
                                    base_col_red, \
                                    base_col_green, \
                                    base_col_blue, \
                                    draw_height, \
                                    draw_radius, \
                                    draw_box1, \
                                    draw_box2, \
                                    draw_shape \
                                    FROM species",
                          sql_callback_common_data_all_species,
                          this,
                          &errmsg);
    sql_err_handler(db, errcode, errmsg);

    sqlite3_close(db);
}


std::vector<basic_tree> data_importer::minimal_to_basic(const std::map<int, std::vector<MinimalPlant> > &plants)
{
    std::vector<basic_tree> trees;

    for (auto &speccls : plants)
    {
        int spec = speccls.first;
        const std::vector<MinimalPlant> &specplants = speccls.second;
        for (const auto &ctree : specplants)
        {
            float x, y, radius;
            x = ctree.x;
            y = ctree.y;
            radius = ctree.r;
            basic_tree newtree(x, y, radius, ctree.h, 1.0f); // dbh set to 1.0f as placeholder
            newtree.species = spec;
            trees.push_back(newtree);
        }
    }
    return trees;
}

