// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include <cerrno>
#include <algorithm>

#include "red_python.h"

#include "reductions.h"
#include "label_dictionary.h"
#include "vw.h"
#include "cb_algs.h"
#include "vw_exception.h"
#include "gen_cs_example.h"
#include "vw_versions.h"
#include "explore.h"

using namespace LEARNER;
using namespace CB;
using namespace ACTION_SCORE;
using namespace GEN_CS;
using namespace CB_ALGS;
using namespace VW::config;
using namespace exploration;

namespace RED_PYTHON
{
struct red_python
{
 private:

 public:
  Copperhead* ch;
  template <bool is_learn>
  void do_actual_learning(LEARNER::single_learner& base, example& ec);

  red_python(
      Copperhead* ch)
      : ch(ch)
  {
  }

  ~red_python()
  {
  }
};

template <bool is_learn>
void red_python::do_actual_learning(LEARNER::single_learner& base, example& ec)
{
    //whatever
}

void learn(red_python& c, single_learner& base, example& ec)
{ 
    c.ch->base_learn = &base;

  //c.ch->exc = &ec;
  if (c.ch->run_f)
    c.ch->run_f(*c.ch, &ec);
  else
    std::cerr << "warning: HookTask::structured_predict called before hook is set" << std::endl;
}

void predict(red_python& c, single_learner& base, example& ec) { c.do_actual_learning<false>(base, ec); }

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(options_i& options, vw& all)
{
  bool red_python_option = false;

  option_group_definition new_options("Reduction (not base) implemented in Python");
  new_options
      .add(make_option("red_python", red_python_option)
               //.keep()
               .help("Add python reduction"));
  options.add_and_parse(new_options);

  if (!red_python_option)
    return nullptr;
  
  free_ptr<Copperhead> chead = scoped_calloc_or_throw<Copperhead>();
  auto ld = scoped_calloc_or_throw<red_python>(chead.get());

  chead->random_number = 4;
  all.copperhead = chead.get();
  chead.release();

//   auto ld = scoped_calloc_or_throw<red_python>(all.sd, cb_type, &all.model_file_ver, rank_all, clip_p, no_predict);

  VW::LEARNER::learner<red_python, example>& ret =
      VW::LEARNER::init_learner(ld, as_singleline(setup_base(options, all)), learn, predict);
  return make_base(ret);
}
