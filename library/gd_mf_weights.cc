#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;


int main(int argc, char *argv[])
{
  string infile;
  string outdir(".");
  string vwparams;

  po::variables_map vm;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("infile,I", po::value<string>(&infile), "input (in vw format) of weights to extract")
    ("outdir,O", po::value<string>(&outdir), "directory to write model files to (default: .)")
    ("vwparams", po::value<string>(&vwparams), "vw parameters for model instantiation (-i model.reg -t ...")
    ;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch(exception & e)
    {
      cout << endl << argv[0] << ": " << e.what() << endl << endl << desc << endl;
      exit(2);
    }

  if (vm.count("help") || infile.empty() || vwparams.empty()) {
    cout << "Dumps weights for matrix factorization model (gd_mf)." << endl;
    cout << "The constant will be written to <outdir>/constant." << endl;
    cout << "Linear and quadratic weights corresponding to the input features will be " << endl;
    cout << "written to <outdir>/<ns>.linear and <outdir>/<ns>.quadratic,respectively." << endl;
    cout << endl;
    cout << desc << "\n";
    cout << "Example usage:" << endl;
    cout << "    Extract weights for user 42 and item 7 under randomly initialized rank 10 model:" << endl;
    cout << "    echo '|u 42 |i 7' | ./gd_mf_weights -I /dev/stdin --vwparams '-q ui --rank 10'" << endl;
    return 1;
  }

  // initialize model
  vw* model = VW::initialize(vwparams);
  model->audit = true;

  // global model params
  unsigned char left_ns = model->pairs[0][0];
  unsigned char right_ns = model->pairs[0][1];
  weight* weights = model->reg.weight_vector;
  size_t mask = model->reg.weight_mask;

  // const char *filename = argv[0];
  FILE* file = fopen(infile.c_str(), "r");
  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  // output files
  ofstream constant((outdir + string("/") + string("constant")).c_str()),
    left_linear((outdir + string("/") + string(1, left_ns) + string(".linear")).c_str()),
    left_quadratic((outdir + string("/") + string(1, left_ns) + string(".quadratic")).c_str()),
    right_linear((outdir + string("/") + string(1, right_ns) + string(".linear")).c_str()),
    right_quadratic((outdir + string("/") + string(1, right_ns) + string(".quadratic")).c_str());

  example *ec;
  while ((read = getline(&line, &len, file)) != -1)
    {
      line[strlen(line)-1] = 0; // chop

      ec = VW::read_example(*model, line);

      // write out features for left namespace
      if (ec->audit_features[left_ns].begin != ec->audit_features[left_ns].end)
	{
	  for (audit_data *f = ec->audit_features[left_ns].begin; f != ec->audit_features[left_ns].end; f++)
	    {
	      left_linear << f->feature << '\t' << weights[f->weight_index & mask];

	      left_quadratic << f->feature;
	      for (size_t k = 1; k <= model->rank; k++)
		left_quadratic << '\t' << weights[(f->weight_index + k) & mask];
	    }
	  left_linear << endl;
	  left_quadratic << endl;
	}

      // write out features for right namespace
      if (ec->audit_features[right_ns].begin != ec->audit_features[right_ns].end)
	{
	  for (audit_data *f = ec->audit_features[right_ns].begin; f != ec->audit_features[right_ns].end; f++)
	    {
	      right_linear << f->feature << '\t' << weights[f->weight_index & mask];

	      right_quadratic << f->feature;
	      for (size_t k = 1; k <= model->rank; k++)
		right_quadratic << '\t' << weights[(f->weight_index + k + model->rank) & mask];
	    }
	  right_linear << endl;
	  right_quadratic << endl;
	}

      VW::finish_example(*model, ec);
    }

  // write constant
  feature* f = ec->atomics[constant_namespace].begin;
  constant << weights[f->weight_index & mask] << endl;

  // clean up
  VW::finish(*model);
  fclose(file);
}
