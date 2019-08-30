#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{ std::string infile;
  std::string outdir(".");
  std::string vwparams;

  po::variables_map vm;
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "produce help message")
  ("infile,I", po::value<std::string>(&infile), "input (in vw format) of weights to extract")
  ("outdir,O", po::value<std::string>(&outdir), "directory to write model files to (default: .)")
  ("vwparams", po::value<std::string>(&vwparams), "vw parameters for model instantiation (-i model.reg -t ...")
  ;

  try
  { po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch(std::exception & e)
  {std::cout << std::endl << argv[0] << ": " << e.what() << std::endl << std::endl << desc << std::endl;
    exit(2);
  }

  if (vm.count("help") || infile.empty() || vwparams.empty())
  { std::cout << "Dumps weights for matrix factorization model (gd_mf)." << std::endl;
    std::cout << "The constant will be written to <outdir>/constant." << std::endl;
    std::cout << "Linear and quadratic weights corresponding to the input features will be " << std::endl;
    std::cout << "written to <outdir>/<ns>.linear and <outdir>/<ns>.quadratic,respectively." << std::endl;
    std::cout << std::endl;
    std::cout << desc << "\n";
    std::cout << "Example usage:" << std::endl;
    std::cout << "    Extract weights for user 42 and item 7 under randomly initialized rank 10 model:" << std::endl;
    std::cout << "    echo '|u 42 |i 7' | ./gd_mf_weights -I /dev/stdin --vwparams '-q ui --rank 10'" << std::endl;
    return 1;
  }

  // initialize model
  vw* model = VW::initialize(vwparams);
  model->audit = true;

  std::string target("--rank ");
  size_t loc = vwparams.find(target);
  const char* location = vwparams.c_str()+loc+target.size();
  size_t rank = atoi(location);

  // global model params
  unsigned char left_ns = model->pairs[0][0];
  unsigned char right_ns = model->pairs[0][1];
  dense_parameters& weights = model->weights.dense_weights;

  // const char *filename = argv[0];
  FILE* file = fopen(infile.c_str(), "r");
  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  // output files
  std::ofstream constant((outdir + std::string("/") + std::string("constant")).c_str()),
           left_linear((outdir + std::string("/") + std::string(1, left_ns) + std::string(".linear")).c_str()),
           left_quadratic((outdir + std::string("/") + std::string(1, left_ns) + std::string(".quadratic")).c_str()),
           right_linear((outdir + std::string("/") + std::string(1, right_ns) + std::string(".linear")).c_str()),
           right_quadratic((outdir + std::string("/") + std::string(1, right_ns) + std::string(".quadratic")).c_str());

  example *ec = NULL;
  while ((read = getline(&line, &len, file)) != -1)
  { line[strlen(line)-1] = 0; // chop

    ec = VW::read_example(*model, line);

    // write out features for left namespace
    features& left = ec->feature_space[left_ns];
    for (size_t i = 0; i < left.size(); ++i)
    { left_linear << left.space_names[i].get()->second << '\t' << weights[left.indicies[i]];

      left_quadratic << left.space_names[i].get()->second;
      for (size_t k = 1; k <= rank; k++)
        left_quadratic << '\t' << weights[(left.indicies[i] + k)];
    }
    left_linear << std::endl;
    left_quadratic << std::endl;

    // write out features for right namespace
    features& right = ec->feature_space[right_ns];
    for (size_t i = 0; i < right.size(); ++i)
    { right_linear << right.space_names[i].get()->second << '\t' << weights[right.indicies[i]];

      right_quadratic << right.space_names[i].get()->second;
      for (size_t k = 1; k <= rank; k++)
        right_quadratic << '\t' << weights[(right.indicies[i] + k + rank)];
    }
    right_linear << std::endl;
    right_quadratic << std::endl;

    VW::finish_example(*model, *ec);
  }

  // write constant
  constant << weights[ec->feature_space[constant_namespace].indicies[0]] << std::endl;

  // clean up
  VW::finish(*model);
  fclose(file);
}
