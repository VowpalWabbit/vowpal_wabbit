#include "gd.h"
#include "float.h"
#include "reductions.h"

using namespace VW::config;

struct print
{
  vw* all;
};  // regressor, feature loop

void print_feature(vw& /* all */, float value, uint64_t index)
{
  std::cout << index;
  if (value != 1.)
    std::cout << ":" << value;
  std::cout << " ";
}

void learn(print& p, LEARNER::base_learner&, example& ec)
{
  label_data& ld = ec.l.simple;
  if (ld.label != FLT_MAX)
  {
    std::cout << ld.label << " ";
    if (ec.weight != 1 || ld.initial != 0)
    {
      std::cout << ec.weight << " ";
      if (ld.initial != 0)
        std::cout << ld.initial << " ";
    }
  }
  if (ec.tag.size() > 0)
  {
    std::cout << '\'';
    std::cout.write(ec.tag.begin(), ec.tag.size());
  }
  std::cout << "| ";
  GD::foreach_feature<vw, uint64_t, print_feature>(*(p.all), ec, *p.all);
  std::cout << std::endl;
}

LEARNER::base_learner* print_setup(options_i& options, vw& all)
{
  bool print_option = false;
  option_group_definition new_options("Print psuedolearner");
  new_options.add(make_option("print", print_option).keep().help("print examples"));
  options.add_and_parse(new_options);

  if (!print_option)
    return nullptr;

  auto p = scoped_calloc_or_throw<print>();
  p->all = &all;

  all.weights.stride_shift(0);

  LEARNER::learner<print, example>& ret = init_learner(p, learn, learn, 1);
  return make_base(ret);
}
