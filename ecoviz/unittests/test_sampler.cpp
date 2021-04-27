
#include <cppunit/Test.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <random>
#include <chrono>
#include <memory>
#include <experimental/filesystem>

#include "cohortmaps.h"
#include "cohortsampler.h"
#include "data_importer/data_importer.h"

using namespace CppUnit;
using namespace data_importer;

class TestCohortSampler : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestCohortSampler);
    CPPUNIT_TEST(test_temporal_consistency);
    CPPUNIT_TEST(test_undo_actionmap);
    CPPUNIT_TEST_SUITE_END();

public:
    TestCohortSampler();
    ~TestCohortSampler();

    void set_up();
    void test_temporal_consistency();
    void test_undo_actionmap();
protected:

    std::default_random_engine gen;
    std::uniform_real_distribution<float> unif;

    std::unique_ptr<CohortMaps> cmaps;
    std::unique_ptr<cohortsampler> sampler;

    std::vector<ValueGridMap<std::vector<ilanddata::cohort> > > orig_tsmaps;

    const int maxpercell = 10;
    const int samplemult = 3;
};

TestCohortSampler::TestCohortSampler()
{
    set_up();
}
TestCohortSampler::~TestCohortSampler()
{
}

void TestCohortSampler::set_up()
{
    using namespace std::experimental::filesystem;

    float rw = 1000.0f;
    float rh = 1000.0f;

    directory_iterator diriter("/home/konrad/vizproj/cohorts/datadir");

    std::vector<std::string> filenames;
    for (const directory_entry &iter : diriter)
    {
        std::string fname = iter.path().c_str();
        if (fname.substr(fname.size() - 4, 4) == ".pdb")
        {
            filenames.push_back(fname);
        }
    }

    cmaps = std::unique_ptr<CohortMaps>(new CohortMaps(filenames, rw, rh, "2.0"));

    int nmaps = cmaps->get_nmaps();

    for (int i = 0; i < nmaps; i++)
    {
        orig_tsmaps.push_back(cmaps->get_map(i));
    }

    cmaps->do_adjustments(1);
    int gw, gh;
    cmaps->get_grid_dims(gw, gh);
    float tw, th;
    cmaps->get_cohort_dims(tw, th);

    sampler = std::unique_ptr<cohortsampler>(new cohortsampler(tw, th, rw - 1.0f, rh - 1.0f, 1.0f, 1.0f, maxpercell, samplemult));
    sampler->set_spectoidx_map(cmaps->compute_spectoidx_map());
}

void TestCohortSampler::test_undo_actionmap()
{
    cmaps->undo_actionmap();

    int nmaps = cmaps->get_nmaps();

    for (int i = 0; i < nmaps; i++)
    {

        const auto &m1 = orig_tsmaps.at(i);
        const auto &m2 = cmaps->get_map(i);

        int gw, gh;
        m1.getDim(gw, gh);

        int gw2, gh2;
        m2.getDim(gw2, gh2);

        CPPUNIT_ASSERT(gw == gw2 && gh == gh2);

        for (int y = 0; y < gh; y++)
        {
            for (int x = 0; x < gw; x++)
            {
                const auto &c1 = m1.get(x, y);
                const auto &c2 = m2.get(x, y);

                for (int i = 0; i < c1.size(); i++)
                {
                    bool found = false;
                    const ilanddata::cohort &crt1 = c1.at(i);
                    //std::cout << "Comparing " << std::endl;
                    //crt1 >> std::cout;
                    //std::cout << ": " << std::endl;
                    for (int j = 0; j < c2.size(); j++)
                    {
                        const ilanddata::cohort &crt2 = c2.at(j);
                        //crt2 >> std::cout;
                        if (crt1 == crt2)
                        {
                            //std::cout << "Found" << std::endl;
                            found = true;
                            break;
                        }
                    }
                    CPPUNIT_ASSERT(found);
                }

            }
        }
    }
}

void TestCohortSampler::test_temporal_consistency()
{
    for (int ts = 0; ts < 20; ts++)
    {
        std::cout << "Starting timestep " << ts << std::endl;

        std::vector<std::vector<basic_tree> > celltrees_prev;
        std::vector<std::vector<basic_tree> > celltrees_next;

        sampler->sample(cmaps->get_map(ts), &celltrees_prev);
        sampler->sample(cmaps->get_map(ts + 1), &celltrees_next);

        if (celltrees_prev.size() != celltrees_next.size())
        {
            throw std::logic_error("celltrees_prev.size() != celltrees_next.size()");
        }

        for (int i = 0; i < celltrees_prev.size(); i++)
        {
            auto &c1 = celltrees_prev.at(i);
            auto &c2 = celltrees_next.at(i);

            for (auto &t : c2)
            {
                bool found = false;
                for (auto &t_first : c1)
                {
                    if (t.x == t_first.x && t.y == t_first.y)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    if (t.height > 1.0f)
                    {
                        int higher_ts = ts + 1;
                        int lower_ts = ts;
                        auto &crts1 = cmaps->get_map(lower_ts).get(i);
                        auto &crts2 = cmaps->get_map(higher_ts).get(i);
                        bool ignore = false;
                        for (auto &c1 : crts1)
                        {
                            int spidx = c1.specidx;
                            for (auto &c2 : crts2)
                            {
                                if (c2.specidx == c1.specidx && c2.nplants > c1.nplants)
                                {
                                    ignore = true;
                                    break;
                                }
                            }
                            if (ignore) break;
                        }
                        if (!ignore)
                        {
                            std::cout << "Cohorts from which sampling faults occurred: " << std::endl;
                            std::cout << "====================" << std::endl;
                            std::cout << "Timestep 1 cohorts: " << std::endl;
                            for (auto &c : crts1)
                            {
                                c >> std::cout;
                                std::cout << "----------------" << std::endl;
                            }
                            std::cout << "====================" << std::endl;
                            std::cout << "Timestep 2 cohorts: " << std::endl;
                            for (auto &c : crts2)
                            {
                                c >> std::cout;
                                std::cout << "----------------" << std::endl;
                            }
                            std::cout << "====================" << std::endl;
                            std::cout << "Trees set 1 (from timestep 1): " << std::endl;
                            for (auto &t : c1)
                            {
                                std::cout << t.x << ", " << t.y << std::endl;
                            }
                            std::cout << "====================" << std::endl;
                            std::cout << "Trees set 2 (from timestep 2): " << std::endl;
                            for (auto &t : c2)
                            {
                                std::cout << t.x << ", " << t.y << std::endl;
                            }
                            CPPUNIT_ASSERT(false);
                            //throw std::logic_error("Large tree suddenly appeared in cell " + std::to_string(i));
                        }
                        else
                        {
                            std::cout << "Ignoring suspicious case" << std::endl;
                        }
                    }
                    else
                    {
                        //std::cout << "New tree appeared with height " << t.height << " in cell " << std::to_string(i) << std::endl;
                    }
                }
            }
        }


    }
}


int main(int argc, char * argv [])
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(TestCohortSampler::suite());
    runner.run();

    return 0;
}
