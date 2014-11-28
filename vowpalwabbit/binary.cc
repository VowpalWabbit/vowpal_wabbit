#include "reductions.h"
#include "multiclass.h"
#include "simple_label.h"

using namespace LEARNER;

namespace BINARY {

  template <bool is_learn>
  void predict_or_learn(float&, learner& base, example& ec) {
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

  learner* setup(vw& all, po::variables_map& vm)
  {//parse and set arguments
    all.sd->binary_label = true;
    //Create new learner
    learner* ret = new learner(NULL, all.l);
    ret->set_learn<float, predict_or_learn<true> >();
    ret->set_predict<float, predict_or_learn<false> >();
    return ret;
  }
}
