// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "red_python.h"

#include "reductions.h"
#include "vw.h"

using namespace LEARNER;
using namespace VW::config;

namespace RED_PYTHON
{
struct red_python
{
 private:

 public:
  PythonCppBridge* pyCppBridge;

  red_python(
      PythonCppBridge* pyCppBridge)
      : pyCppBridge(pyCppBridge)
  {
  }

  ~red_python()
  {
  }
};

void learn(red_python& redpy, single_learner& base, example& ec)
{ 
  redpy.pyCppBridge->base_learn = &base;

  if (redpy.pyCppBridge->run_f)
    redpy.pyCppBridge->run_f(*redpy.pyCppBridge, &ec);
  else
    std::cerr << "warning: learn called before hook with python is set" << std::endl;
}

void predict(red_python& c, single_learner& base, example& ec) { return; }

}  // namespace RED_PYTHON
using namespace RED_PYTHON;
VW::LEARNER::base_learner* red_python_setup(options_i& options, vw& all)
{
  bool red_python_option = false;

  option_group_definition new_options("Reduction (not base) implemented in Python");
  new_options
      .add(make_option("red_python", red_python_option)
              //  .keep()
               .help("EXPERIMENTAL: Add python reduction"));
  options.add_and_parse(new_options);

  if (!red_python_option)
    return nullptr;
  
  free_ptr<PythonCppBridge> pyCppBridge = scoped_calloc_or_throw<PythonCppBridge>();
  auto ld = scoped_calloc_or_throw<red_python>(pyCppBridge.get());

  pyCppBridge->random_number = 4;
  all.python_cpp_bridge = pyCppBridge.get();
  pyCppBridge.release();

  VW::LEARNER::learner<red_python, example>& ret =
      VW::LEARNER::init_learner(ld, as_singleline(setup_base(options, all)), learn, predict);

  //missing finish

  return make_base(ret);
}
