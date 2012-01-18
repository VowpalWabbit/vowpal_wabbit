#include "sequence.h"
#include "parser.h"

size_t sequence_history=1;
bool sequence_bigrams = false;
size_t sequence_features = 0;
bool sequence_bigram_features = false;
size_t sequence_rollout = 256;
size_t sequence_passes_per_policy = 1;
float sequence_beta = 0.5;
size_t sequence_k=2;

example* sequence_get()
{
  example* ec = get_example();
  return ec;
}

void sequence_return(example* ec)
{
  free_example(ec);
}

void parse_sequence_args(po::variables_map& vm, example* (**gf)(), void (**rf)(example*))
{
  *(global.lp)=OAA::oaa_label;
  *gf = sequence_get;
  *rf = sequence_return;
  sequence_k = vm["sequence"].as<size_t>();

  if (vm.count("sequence_bigrams"))
    sequence_bigrams = true;
  if (vm.count("sequence_bigram_features"))
    sequence_bigrams = true;

  if (vm.count("sequence_features"))
    sequence_features = vm["sequence_features"].as<size_t>();
  if (vm.count("sequence_rollout"))
    sequence_rollout = vm["sequence_rollout"].as<size_t>();
  if (vm.count("sequence_passes_per_policy"))
    sequence_passes_per_policy = vm["sequence_passes_per_policy"].as<size_t>();
  if (vm.count("sequence_beta"))
    sequence_beta = vm["sequence_beta"].as<float>();
}
