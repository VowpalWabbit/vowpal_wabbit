#include <float.h>

#include "reductions.h"

using namespace LEARNER;

namespace Scorer {
  struct scorer{
    vw* all;
  };
  
  template <bool is_learn>
  void predict_or_learn(scorer& s, learner& base, example& ec)
  {
    label_data* ld = (label_data*)ec.ld;
    s.all->set_minmax(s.all->sd, ld->label);

    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);
    
    if(ld->weight > 0 && ld->label != FLT_MAX)
      ec.loss = s.all->loss->getLoss(s.all->sd, ld->prediction, ld->label) * ld->weight;
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    scorer* s = (scorer*)calloc_or_die(1, sizeof(scorer));
    s->all = &all;

    learner* l = new learner(s, all.l);
    l->set_learn<scorer, predict_or_learn<true> >();
    l->set_predict<scorer, predict_or_learn<false> >();

    return l;
  }
}
