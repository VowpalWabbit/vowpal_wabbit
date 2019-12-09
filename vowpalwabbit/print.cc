// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "gd.h"
#include <cfloat>
#include "reductions.h"

using namespace VW::config;

using std::cout;

struct print
{
  vw* all;
};  // regressor, feature loop

void print_feature(vw& /* all */, float value, uint64_t index)
{
  cout << index;
  if (value != 1.)
    cout << ":" << value;
  cout << " ";
}

void learn(print& p, LEARNER::base_learner&, example& ec)
{
  label_data& ld = ec.l.simple;
  if (ld.label != FLT_MAX)
  {
    cout << ld.label << " ";
    if (ec.weight != 1 || ld.initial != 0)
    {
      cout << ec.weight << " ";
      if (ld.initial != 0)
        cout << ld.initial << " ";
    }
  }
  if (!ec.tag.empty())
  {
    cout << '\'';
    cout.write(ec.tag.begin(), ec.tag.size());
  }
  cout << "| ";
  GD::foreach_feature<vw, uint64_t, print_feature>(*(p.all), ec, *p.all);
  cout << std::endl;
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
