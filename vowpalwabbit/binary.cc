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

    label_data* ld = (label_data*)ec.ld;//New loss
    if ( ld->prediction > 0)
      ld->prediction = 1;
    else
      ld->prediction = -1;

    if (ld->label == ld->prediction)
      ec.loss = 0.;
    else
      ec.loss = ld->weight;
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
