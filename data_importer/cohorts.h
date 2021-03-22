#include <vector>
#include <sstream>
#include <random>
#include "../../common/basic_types.h"


namespace ilanddata
{
	struct params
	{
		params(std::string filename);

		int width, height;
		float quantdiv, radialmult;
		int maxpercell;
		int samplemult;
	};


	struct cohort
	{
		cohort(int xs, int ys, int specidx, float dbh, float height, int nplants);

		cohort(std::stringstream &ss);

		int xs, ys;
		int specidx;
		float dbh, height;
		int nplants;
	};

	struct filedata
	{
		int timestep;
		std::string version;
		std::vector<cohort> cohorts;
		std::vector<basic_tree> trees;
	};

	bool fileversion_gteq(std::string v1, std::string v2);

	filedata read(std::string filename, std::string minversion);
	std::pair<std::vector<std::vector<int> >, std::vector<std::vector<std::map<int, int> > > > read_nospecies(std::string filename);

	class sampler
	{
	public:
		sampler(std::string filename, ilanddata::params params);


		std::vector<basic_tree> sample_all(bool soft);
		std::deque<int> gen_poisson_list(int sqsize, int nplants, std::deque<int> *dists, std::default_random_engine &gen);

	private:
		std::vector<basic_tree> sample_one_soft(cohort chrt, std::default_random_engine &gen);
		std::vector<basic_tree> sample_one_hard(cohort chrt, std::default_random_engine &gen);

		std::uniform_real_distribution<float> unif;
		std::vector<std::default_random_engine> generators;
		std::vector<cohort> allc;
		float width, height;
		float quantdiv, radialmult;
		int maxpercell;
		int samplemult;
	};
}
