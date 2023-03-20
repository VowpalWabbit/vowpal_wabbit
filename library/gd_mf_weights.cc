#include "vw/config/cli_help_formatter.h"
#include "vw/config/option_builder.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options_cli.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  using std::cout;
  using std::string;

  bool help = false;
  string infile;
  string outdir;
  string vwparams;

  VW::config::options_cli opts(std::vector<std::string>(argv + 1, argv + argc));
  VW::config::option_group_definition desc("GD MF Weights");
  desc.add(VW::config::make_option("help", help).short_name("h").help("Produce help message"))
      .add(VW::config::make_option("infile", infile).short_name("I").help("Input (in vw format) of weights to extract"))
      .add(VW::config::make_option("outdir", outdir).short_name("O").help("Directory to write model files to"))
      .add(VW::config::make_option("vwparams", vwparams)
               .help("vw parameters for model instantiation (-i model.reg -t ..."));

  opts.add_and_parse(desc);
  // Return value is ignored as option reachability is not relevant here.
  auto warnings = opts.check_unregistered();
  _UNUSED(warnings);

  if (help || infile.empty() || vwparams.empty())
  {
    cout << "Dumps weights for matrix factorization model (gd_mf)." << std::endl;
    cout << "The constant will be written to <outdir>/constant." << std::endl;
    cout << "Linear and quadratic weights corresponding to the input features will be " << std::endl;
    cout << "written to <outdir>/<ns>.linear and <outdir>/<ns>.quadratic,respectively." << std::endl;
    cout << std::endl;

    VW::config::cli_help_formatter help_formatter;
    std::cout << help_formatter.format_help(opts.get_all_option_group_definitions()) << std::endl;

    cout << "Example usage:" << std::endl;
    cout << "    Extract weights for user 42 and item 7 under randomly initialized rank 10 model:" << std::endl;
    cout << "    echo '|u 42 |i 7' | ./gd_mf_weights -I /dev/stdin --vwparams '-q ui --rank 10'" << std::endl;
    return 1;
  }

  // initialize model
  auto model = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(vwparams)));
  model->output_config.audit = true;

  string target("--rank ");
  size_t loc = vwparams.find(target);
  const char* location = vwparams.c_str() + loc + target.size();
  size_t rank = atoi(location);

  // global model params
  std::vector<unsigned char> first_pair;
  for (auto const& i : model->feature_tweaks_config.interactions)
  {
    if (i.size() == 2)
    {
      first_pair = i;
      break;
    }
  }
  if (first_pair.size() != 2)
  {
    cout << "Model doesn't include a quadratic interaction." << std::endl;
    return 2;
  }

  unsigned char left_ns = first_pair[0];
  unsigned char right_ns = first_pair[1];
  auto& weights = model->weights.dense_weights;

  FILE* file;
  VW::file_open(&file, infile.c_str(), "r");
  char* line = nullptr;
  size_t len = 0;
  ssize_t read;

  // output files
  std::ofstream constant((outdir + string("/") + string("constant")).c_str());
  std::ofstream left_linear((outdir + string("/") + string(1, left_ns) + string(".linear")).c_str());
  std::ofstream left_quadratic((outdir + string("/") + string(1, left_ns) + string(".quadratic")).c_str());
  std::ofstream right_linear((outdir + string("/") + string(1, right_ns) + string(".linear")).c_str());
  std::ofstream right_quadratic((outdir + string("/") + string(1, right_ns) + string(".quadratic")).c_str());

  VW::example* ec = nullptr;
  while ((read = getline(&line, &len, file)) != -1)
  {
    line[strlen(line) - 1] = 0;  // chop

    ec = VW::read_example(*model, line);

    // write out features for left namespace
    VW::features& left = ec->feature_space[left_ns];
    for (size_t i = 0; i < left.size(); ++i)
    {
      left_linear << left.space_names[i].name << '\t' << weights[left.indices[i]];

      left_quadratic << left.space_names[i].name;
      for (size_t k = 1; k <= rank; k++) { left_quadratic << '\t' << weights[(left.indices[i] + k)]; }
    }
    left_linear << std::endl;
    left_quadratic << std::endl;

    // write out features for right namespace
    VW::features& right = ec->feature_space[right_ns];
    for (size_t i = 0; i < right.size(); ++i)
    {
      right_linear << right.space_names[i].name << '\t' << weights[right.indices[i]];

      right_quadratic << right.space_names[i].name;
      for (size_t k = 1; k <= rank; k++) { right_quadratic << '\t' << weights[(right.indices[i] + k + rank)]; }
    }
    right_linear << std::endl;
    right_quadratic << std::endl;

    VW::finish_example(*model, *ec);
  }

  // write constant
  constant << weights[ec->feature_space[VW::details::CONSTANT_NAMESPACE].indices[0]] << std::endl;

  // clean up
  model->finish();
  fclose(file);
}
