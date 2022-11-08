#include "data_importer/data_importer.h"
#include <experimental/filesystem>
#include "cohortmaps.h"

using namespace data_importer::ilanddata;

int main(int argc, char * argv [])
{

    std::experimental::filesystem::directory_iterator diriter("/home/konrad/vizproj/cohorts/datadir");

    std::vector<std::string> filenames;

    for (auto &entry : diriter)
    {
        std::string fname = entry.path().string();
        if (fname.substr(fname.size() - 4, 4) == ".pdb")
        {
            filenames.push_back(fname);
            std::cout << filenames.back() << std::endl;
        }
    }
    std::map<std::string, int> species_lookup; // is EMPTY! will throw exceptions

    CohortMaps cmaps(filenames, 1000.0f, 1000.0f, "2.0", species_lookup);

    cmaps.determine_cohort_startidxes();
    return 0;

    int nmaps = cmaps.get_nmaps();

    ValueGridMap<int> ccount;
    if (nmaps > 0)
    {
        ccount.setDim(cmaps.get_map(0));
        ccount.setDimReal(cmaps.get_map(0));
        ccount.setOffsets(cmaps.get_map(0));
        ccount.fill(int(0));
    }
    int max_count = 0;

    bool duplicates_found = false;
    for (int i = 0; i < nmaps; i++)
    {
        auto map = cmaps.get_map(i);

        int gw, gh;
        map.getDim(gw, gh);

        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                auto vec = map.get(x, y);
                if (vec.size() > ccount.get(x, y))
                {
                    ccount.get(x, y) = vec.size();
                    if (ccount.get(x, y) > max_count)
                        max_count = ccount.get(x, y);
                }
                std::sort(vec.begin(), vec.end(), [](cohort &crt1, cohort &crt2) { return crt1.specidx < crt2.specidx; });
                for (int j = 0; j < int(vec.size()) - 1; j++)
                {
                    if (vec.at(j).specidx == vec.at(j + 1).specidx)
                    {
                        std::cout << "Duplicate found at " << x << ", " << y << " in timestep " << i << std::endl;
                        duplicates_found = true;

                        vec.at(j) >> std::cout;
                        std::cout << "------------------" << std::endl;
                        vec.at(j + 1) >> std::cout;
                        std::cout << "==================" << std::endl;
                    }
                }
            }
        }
        break;
    }
    if (!duplicates_found)
    {
        std::cout << "No duplicates found" << std::endl;
    }

    std::cout << "Maximum number of cohorts in any tile location: " << max_count << std::endl;

    cmaps.compute_specset_map();



    auto setmap = cmaps.move_specset_map();
    if (!setmap)
        throw std::logic_error("setmap is null");
    int gw, gh;
    setmap->getDim(gw, gh);


    ValueGridMap<int> countmap;

    countmap.setDim(*setmap);
    countmap.setDimReal(*setmap);

    countmap.getDim(gw, gh);

    for (int y = 0; y < gh; y++)
    {
        for (int x = 0; x < gw; x++)
        {
            countmap.set(x, y, setmap->get(x, y).size());
        }
    }
    std::cout << "writing countmap" << std::endl;

    std::map<int, int> ncohort_count;
    for (auto iter = countmap.begin(); iter != countmap.end(); advance(iter, 1))
    {
        int count = *iter;
        if (ncohort_count.count(count))
        {
            ncohort_count[count]++;
        }
        else
        {
            ncohort_count[count] = 1;
        }
    }

    for (auto &p : ncohort_count)
    {
        std::cout << "NCohort " << p.first << ": " << p.second << std::endl;
    }

    data_importer::write_txt("/home/konrad/countmap.txt", &countmap);

	return 0;
}
