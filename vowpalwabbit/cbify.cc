#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"

using namespace LEARNER;
using namespace MultiWorldTesting;
using namespace MultiWorldTesting::SingleAction;

struct cbify
{ CB::label cb_label;
};

float loss(uint32_t label, uint32_t final_prediction)
{
  if (label != final_prediction)
    return 1.;
  else
    return 0.;
}

template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

void finish(cbify& data)
{ CB::cb_label.delete_label(&data.cb_label);
}

template <bool is_learn>
void predict_or_learn(cbify& data, base_learner& base, example& ec)
{ //ALEKH: Ideally, we will be able to return the probability from base.predict, perhaps using the probs field in ec.pred.
  //Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;
  data.cb_label.costs.erase();
  ec.l.cb = data.cb_label;

  //Call the cb_explore algorithm. It returns a vector with one non-zero entry denoting the probability of the chosen action
  base.predict(ec);

  CB::cb_class cl;
  cl.action = ec.pred.action_prob.action;
  cl.probability = ec.pred.action_prob.probability;

  if(!cl.action)
    THROW("No action with non-zero probability found!");
  uint32_t action = cl.action;
  cl.cost = loss(ld.label, cl.action);

  //Create a new cb label
  data.cb_label.costs.push_back(cl);
  ec.l.cb = data.cb_label;
  base.learn(ec);
  ec.l.multi = ld;
  ec.pred.multiclass = action;
}

base_learner* cbify_setup(vw& all)
{ //parse and set arguments
  if (missing_option<size_t, true>(all, "cbify", "Convert multiclass on <k> classes into a contextual bandit problem"))
    return nullptr;

  po::variables_map& vm = all.vm;
  cbify& data = calloc_or_throw<cbify>();

  if (count(all.args.begin(), all.args.end(),"--cb_explore") == 0)
  { all.args.push_back("--cb_explore");
    stringstream ss;
    ss << vm["cbify"].as<size_t>();
    all.args.push_back(ss.str());
    all.args.push_back("--learn_only");
  }
  base_learner* base = setup_base(all);

  learner<cbify>* l;
  l = &init_multiclass_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>, all.p, 1);
  l->set_finish(finish);

  return make_base(*l);
}
