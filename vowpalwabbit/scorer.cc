#include <float.h>
#include "reductions.h"

struct scorer{ vw* all; };

template <bool is_learn, float (*link)(float in)>
void predict_or_learn(scorer& s, LEARNER::base_learner& base, example& ec)
{
  s.all->set_minmax(s.all->sd, ec.l.simple.label);
  
  if (is_learn && ec.l.simple.label != FLT_MAX && ec.l.simple.weight > 0)
    base.learn(ec);
  else
    base.predict(ec);
  
  if(ec.l.simple.weight > 0 && ec.l.simple.label != FLT_MAX)
    ec.loss = s.all->loss->getLoss(s.all->sd, ec.pred.scalar, ec.l.simple.label) * ec.l.simple.weight;
  
  ec.pred.scalar = link(ec.pred.scalar);
}

// y = f(x) -> [0, 1]
float logistic(float in) { return 1.f / (1.f + exp(- in)); }

// http://en.wikipedia.org/wiki/Generalized_logistic_curve
// where the lower & upper asymptotes are -1 & 1 respectively
// 'glf1' stands for 'Generalized Logistic Function with [-1,1] range'
//    y = f(x) -> [-1, 1]
float glf1(float in) { return 2.f / (1.f + exp(- in)) - 1.f; }

float id(float in) { return in; }

LEARNER::base_learner* scorer_setup(vw& all)
{
  new_options(all)
    ("link", po::value<string>()->default_value("identity"), "Specify the link function: identity, logistic or glf1");
  add_options(all);
  po::variables_map& vm = all.vm;
  scorer& s = calloc_or_die<scorer>();
  s.all = &all;
  
  LEARNER::base_learner* base = setup_base(all);
  LEARNER::learner<scorer>* l; 
  
  string link = vm["link"].as<string>();
  if (!vm.count("link") || link.compare("identity") == 0)
    l = &init_learner(&s, base, predict_or_learn<true, id>, predict_or_learn<false, id>);
  else if (link.compare("logistic") == 0)
    {
      *all.file_options << " --link=logistic ";
      l = &init_learner(&s, base, predict_or_learn<true, logistic>, 
			predict_or_learn<false, logistic>);
    }
  else if (link.compare("glf1") == 0)
    {
      *all.file_options << " --link=glf1 ";
      l = &init_learner(&s, base, predict_or_learn<true, glf1>, 
			predict_or_learn<false, glf1>);
    }
  else
    {
      cerr << "Unknown link function: " << link << endl;
      throw exception();
    }
  all.scorer = make_base(*l);
  
  return all.scorer;
}
