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
using namespace ACTION_SCORE;

struct cbify;

//Scorer class for use by the exploration library
class vw_scorer : public IScorer<example>
{
public:
  vector<float> Score_Actions(example& ctx);
};

struct vw_recorder : public IRecorder<example>
{ void Record(example& context, u32 a, float p, string /*unique_key*/)
  { }

  virtual ~vw_recorder()
  { }
};

struct cbify
{ CB::label cb_label;
  GenericExplorer<example>* generic_explorer;
  //v_array<float> probs;
  vw_scorer* scorer;
  MwtExplorer<example>* mwt_explorer;
  vw_recorder* recorder;
  v_array<action_score> a_s;
  // used as the seed
  size_t example_counter;
};

vector<float> vw_scorer::Score_Actions(example& ctx)
{ vector<float> probs_vec;
  for(uint32_t i = 0; i < ctx.pred.a_s.size(); i++)
    probs_vec.push_back(ctx.pred.a_s[i].score);
  return probs_vec;
}

float loss(uint32_t label, uint32_t final_prediction)
{ if (label != final_prediction)
    return 1.;
  else
    return 0.;
}

template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

void finish(cbify& data)
{ CB::cb_label.delete_label(&data.cb_label);
  //data.probs.delete_v();
  delete_it(data.scorer);
  delete_it(data.generic_explorer);
  delete_it(data.mwt_explorer);
  delete_it(data.recorder);
  data.a_s.delete_v();
}

template <bool is_learn>
void predict_or_learn(cbify& data, base_learner& base, example& ec)
{ //Store the multiclass input label
  MULTICLASS::label_t ld = ec.l.multi;
  data.cb_label.costs.erase();
  ec.l.cb = data.cb_label;
  ec.pred.a_s = data.a_s;

  //Call the cb_explore algorithm. It returns a vector of probabilities for each action
  base.predict(ec);
  //data.probs = ec.pred.scalars;

  uint32_t action = data.mwt_explorer->Choose_Action(*data.generic_explorer, StringUtils::to_string(data.example_counter++), ec);

  CB::cb_class cl;
  cl.action = action;
  cl.probability = ec.pred.a_s[action-1].score;

  if(!cl.action)
    THROW("No action with non-zero probability found!");
  cl.cost = loss(ld.label, cl.action);

  //Create a new cb label
  data.cb_label.costs.push_back(cl);
  ec.l.cb = data.cb_label;
  base.learn(ec);
  data.a_s.erase();
  data.a_s = ec.pred.a_s;
  ec.l.multi = ld;
  ec.pred.multiclass = action;
}

base_learner* cbify_setup(vw& all)
{ //parse and set arguments
  if (missing_option<size_t, true>(all, "cbify", "Convert multiclass on <k> classes into a contextual bandit problem"))
    return nullptr;

  po::variables_map& vm = all.vm;
  uint32_t num_actions = (uint32_t)vm["cbify"].as<size_t>();

  cbify& data = calloc_or_throw<cbify>();
  data.recorder = new vw_recorder();
  data.mwt_explorer = new MwtExplorer<example>("vw",*data.recorder);
  data.scorer = new vw_scorer();
  data.a_s = v_init<action_score>();
  //data.probs = v_init<float>();
  data.generic_explorer = new GenericExplorer<example>(*data.scorer, (u32)num_actions);

  if (count(all.args.begin(), all.args.end(),"--cb_explore") == 0)
  { all.args.push_back("--cb_explore");
    stringstream ss;
    ss << num_actions;
    all.args.push_back(ss.str());
  }
  base_learner* base = setup_base(all);

  all.delete_prediction = nullptr;
  learner<cbify>* l;
  l = &init_multiclass_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>, all.p, 1);
  l->set_finish(finish);

  return make_base(*l);
}
