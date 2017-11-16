#include <float.h>
#include "reductions.h"

using namespace std;
template <bool is_learn>
void predict_or_learn(char&, LEARNER::base_learner& base, example& ec)
{ if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if ( ec.pred.scalar > 0)
    ec.pred.scalar = 1;
  else
    ec.pred.scalar = -1;

  if (ec.l.simple.label != FLT_MAX)
  { if (fabs(ec.l.simple.label) != 1.f)
      cout << "You are using label " << ec.l.simple.label << " not -1 or 1 as loss function expects!" << endl;
    else if (ec.l.simple.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ec.weight;
  }
}

LEARNER::base_learner* binary_setup(vw& all)
{ if (missing_option(all, false, "binary", "report loss as binary classification on -1,1"))
    return nullptr;

  LEARNER::learner<char>& ret =
    LEARNER::init_learner<char>(nullptr, setup_base(all),
                                predict_or_learn<true>, predict_or_learn<false>);
  return make_base(ret);
}
