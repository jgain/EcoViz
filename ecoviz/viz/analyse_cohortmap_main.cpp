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

    CohortMaps cmaps(filenames, 1000.0f, 1000.0f, "2.0");
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
