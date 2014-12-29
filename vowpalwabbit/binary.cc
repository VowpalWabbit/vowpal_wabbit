#include "reductions.h"
#include "multiclass.h"
#include "simple_label.h"

using namespace LEARNER;

namespace BINARY {

  template <bool is_learn>
  void predict_or_learn(char&, base_learner& base, example& ec) {
    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    if ( ec.pred.scalar > 0)
      ec.pred.scalar = 1;
    else
      ec.pred.scalar = -1;

    if (ec.l.simple.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ec.l.simple.weight;
  }

  base_learner* setup(vw& all, po::variables_map& vm)
  {//parse and set arguments
    all.sd->binary_label = true;
    //Create new learner
    learner<char>& ret = init_learner<char>(NULL, all.l, predict_or_learn<true>, predict_or_learn<false>);
    return make_base(ret);
  }
}
