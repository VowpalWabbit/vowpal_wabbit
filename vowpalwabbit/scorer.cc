#include <float.h>

#include "reductions.h"

using namespace LEARNER;

namespace Scorer {
  struct scorer{
    vw* all;
  };

  template <bool is_learn, float (*link)(float in)>
  void predict_or_learn(scorer& s, learner& base, example& ec)
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
  float logistic(float in)
  {
    return 1.f / (1.f + exp(- in));
  }

  // http://en.wikipedia.org/wiki/Generalized_logistic_curve
  // where the lower & upper asymptotes are -1 & 1 respectively
  // 'glf1' stands for 'Generalized Logistic Function with [-1,1] range'
  //    y = f(x) -> [-1, 1]
  float glf1(float in)
  {
    return 2.f / (1.f + exp(- in)) - 1.f;
  }

  float noop(float in)
  {
    return in;
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    scorer* s = (scorer*)calloc_or_die(1, sizeof(scorer));
    s->all = &all;

    po::options_description link_opts("Link options");

    link_opts.add_options()
      ("link", po::value<string>()->default_value("identity"), "Specify the link function: identity, logistic or glf1");

    vm = add_options(all, link_opts);

    learner* l = new learner(s, all.l);

    string link = vm["link"].as<string>();
    if (!vm.count("link") || link.compare("identity") == 0)
      {
	l->set_learn<scorer, predict_or_learn<true, noop> >();
	l->set_predict<scorer, predict_or_learn<false, noop> >();
      }
    else if (link.compare("logistic") == 0)
      {
	all.file_options.append(" --link=logistic ");
	l->set_learn<scorer, predict_or_learn<true, logistic> >();
	l->set_predict<scorer, predict_or_learn<false, logistic> >();
      }
    else if (link.compare("glf1") == 0)
      {
	all.file_options.append(" --link=glf1 ");
	l->set_learn<scorer, predict_or_learn<true, glf1> >();
	l->set_predict<scorer, predict_or_learn<false, glf1> >();
      }
    else
      {
	cerr << "Unknown link function: " << link << endl;
	throw exception();
      }

    return l;
  }
}
